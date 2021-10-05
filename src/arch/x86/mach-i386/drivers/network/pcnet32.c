#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>
#include <stdint.h>

#include <xbook/driver.h>
#include <assert.h>
#include <xbook/byteorder.h>
#include <xbook/spinlock.h>
#include <math.h>
#include <xbook/waitqueue.h>
#include <xbook/memalloc.h>
#include <arch/io.h>
#include <xbook/hardirq.h>
#include <arch/pci.h>
#include <arch/atomic.h>
#include <arch/cpu.h>
#include <arch/page.h>
#include <sys/ioctl.h>

#define DRV_VERSION "v0.1"
#define DRV_NAME "net-pcnet32" DRV_VERSION

#define DEV_NAME "pcnet32"

// #define DEBUG_DRV

// #define DEBUG_LOG

#ifdef DEBUG_LOG
#define log_print(fmt, ...) \
    dbgprint(fmt, ##__VA_ARGS__)
#else
#define log_print(fmt, ...) 
#endif

#define DEBUG_INITBLOCK 0

#define ETH_ALEN 6  /* MAC addr */
#define ETH_ZLEN 60 /* Minimum length of data without CRC check */
#define ETH_DATA_LEN 1500 /* Maximum length of data in a frame */
#define ETH_FRAME_LEN 1518 /* Maximum Ethernet data length */

#define RX_MSG_CNT 8  /* 4 msg queue */
#define RX_MSG_SIZE (ETH_FRAME_LEN + 4)  /* 4 save real msg size */

#define TX_CACHE_BUF_SIZE          (2048)

#define PCNET32_VENDOR_ID   0x1022
#define PCNET32_DEVICE_ID   0x2000

/* Offsets from base I/O address. */
#define PCNET32_WIO_RDP     0x10
#define PCNET32_WIO_RAP     0x12
#define PCNET32_WIO_RESET   0x14
#define PCNET32_WIO_BDP     0x16

#define CSR0        0
#define CSR0_INIT   0x1
#define CSR0_START  0x2
#define CSR0_STOP   0x4
#define CSR0_TXPOLL 0x8
#define CSR0_INTEN  0x40
#define CSR0_IDON   0x0100
#define CSR0_NORMAL (CSR0_START | CSR0_INTEN)
#define CSR0_TINT   0x0200  /* Transmit Interrupt */
#define CSR0_RINT   0x0400  /* Receive Interrupt */
#define CSR0_MERR   0x0800  /* Memory Error */
#define CSR0_MISS   0x1000  /* Missed Frame */
#define CSR0_CERR   0x2000  /* Collision Error */
#define CSR0_BABL   0x4000  /* Babble is a transmitter time-out error. */

/* Error is set by the ORing of BABL, CERR, MISS, and MERR.
 * ERR remains set as long as any of the error flags are true.
 */
#define CSR0_ERR   0x8000

#define CSR1        1
#define CSR2        2
#define CSR3        3   /*  Interrupt Masks and Deferral Control */
#define CSR3_IDONM  (1 << 8)   /* Initialization Done Mask. */
#define CSR4        4   /* Test and Features Control */
#define CSR4_ASTRP_RCV  (1 << 10)   /* Auto Strip Receive */
#define CSR4_APAD_XMT   (1 << 11)   /* Auto Pad Transmit */

#define CSR5        5
#define CSR5_SUSPEND    0x0001
#define CSR6        6   /* RX/TX Descriptor Table Length */

#define CSR15       15  /* Mode */
#define CSR18       18  /* Current Receive Buffer Address Lower */
#define CSR19       19  /* Current Receive Buffer Address Upper */
#define CSR24       24  /* Base Address of Receive Descriptor Ring Lower */
#define CSR25       25  /* Base Address of Receive Descriptor Ring Upper */
#define CSR30       30  /* Base Address of Transmit Descriptor Ring Lower */
#define CSR31       31  /* Base Address of Transmit Descriptor Ring Upper */

#define CSR58       58  /* Software Style */
#define CSR58_PCNET_PCI_II   0x02

#define PCNET32_INIT_LOW    1
#define PCNET32_INIT_HIGH   2
#define PCNET32_MC_FILTER 8   /* broadcast filter */

#define CSR72       72  /* Receive Descriptor Ring Counter */
#define CSR74       74  /* Transmit Descriptor Ring Counter */

#define BCR2        2
#define BCR2_ASEL   (1 << 1)

#define PCNET32_TX_BUFFERS 8
#define PCNET32_RX_BUFFERS 32
#define PCNET32_LOG_TX_BUFFERS 3    /* 2^3 = 8 buffers */
#define PCNET32_LOG_RX_BUFFERS 5    /* 2^5 = 32 buffers */

#define PCNET32_RING_DE_SIZE     16

#define PCNET32_TX_RETRY     10 /* tx retry counter when no available descriptor entry */

#define PCNET32_DESC_STATUS_OWN 0x8000  /* card own the desc */

/**
 * End of Packet indicates that this is the last buffer used by
 * the PCnet-PCI II controller for this frame.
 */
#define PCNET32_DESC_STATUS_ENP 0x0100

/**
 * Start of Packet indicates that this
 * is the first buffer used by the
 * PCnet-PCI II controller for this
 * frame.
 */
#define PCNET32_DESC_STATUS_STP 0x0200

#define GET_PCNET32(eth)    (device_extension_t *)(eth)

/**
 * rx ring desc struct
 */
struct pcnet32_rx_desc
{
    uint32_t base;   /* buffer base addr */
    uint16_t buf_length; /* two`s complement of length */
    uint16_t status; /* desc status */
    uint16_t msg_length; /*  Message Byte Count is the length in bytes of the received message. */
    uint16_t rpc_rcc;
    uint32_t reserved;
} __attribute__ ((packed));

/**
 * tx ring desc struct
 */
struct pcnet32_tx_desc
{
    uint32_t base;   /* buffer base addr */
    uint16_t buf_length; /* two`s complement of length */
    uint16_t status; /* desc status */
    uint32_t misc;
    uint32_t reserved;
} __attribute__ ((packed));

/**
 * The PCNET32 32-Bit initialization block, described in databook.
 * The Mode Register (CSR15) allows alteration of the chip's operating
 * parameters. The Mode field of the Initialization Block is copied directly
 * into CSR15. Normal operation is the result of configuring the Mode field
 * with all bits zero.
 */
struct pcnet32_init_block
{
    uint16_t mode;
    uint16_t tlen_rlen;
    uint8_t  phys_addr[6];
    uint16_t reserved;
    uint32_t filter[2];
    /* Receive and transmit ring base, along with extra bits. */
    uint32_t rx_ring;
    uint32_t tx_ring;
} __attribute__ ((packed));

typedef struct _device_extension {
    /* interface address info. */
    uint8_t dev_addr[ETH_ALEN];         /* MAC address  */

    pci_device_t *pci_dev;   /* pci device info */
    flags_t flags;
    uint32_t iobase; /* io port base */
    uint32_t irqno;  /* irq number */

    struct pcnet32_init_block *init_block;

    uint16_t rx_len_bits;
    uint16_t tx_len_bits;

    dma_addr_t rx_ring_dma_addr;
    dma_addr_t tx_ring_dma_addr;

    dma_addr_t init_block_dma_addr;

    uint32_t rx_buffer_ptr;
    uint32_t tx_buffer_ptr;  /* pointers to transmit/receive buffers */

    size_t rx_buffer_count;   /* total number of receive buffers */
    size_t tx_buffer_count;   /* total number of transmit buffers */

    size_t buffer_size;  /* length of each packet buffer */

    size_t de_size;    /* length of descriptor entry */

    struct pcnet32_rx_desc *rdes;   /* pointer to ring buffer of receive des */
    struct pcnet32_tx_desc *tdes;   /* pointer to ring buffer of transmit des */

    uint32_t rx_buffers; /* physical address of actual receive buffers (< 4 GiB) */
    uint32_t tx_buffers; /* physical address of actual transmit buffers (< 4 GiB) */

    device_queue_t rx_queue;    /* 接收队列 */
} device_extension_t;

/**
 * does the driver own the particular buffer?
 */
static inline bool pcnet32_is_driver_own(device_extension_t *dev, bool is_tx, uint32_t idx)
{
    return (bool)(is_tx ? ((dev->tdes[idx].status & PCNET32_DESC_STATUS_OWN) == 0) :
                   ((dev->rdes[idx].status & PCNET32_DESC_STATUS_OWN) == 0));
}

/*
 * get the next desc buffer index
 */
static inline uint32_t pcnet32_get_next_desc(device_extension_t *dev, uint32_t cur_idx, uint32_t buf_count)
{
    return (cur_idx + 1) % buf_count;
}

static void pcnet32_init_rx_desc_entry(device_extension_t *dev, uint32_t idx)
{
    struct pcnet32_rx_desc *des = dev->rdes + idx;
    memset(des, 0, dev->de_size);

    des->base = kern_vir_addr2phy_addr(dev->rx_buffers + idx * dev->buffer_size);

    /* next 2 bytes are 0xf000 OR'd with the first 12 bits of the 2s complement of the length */
    uint16_t bcnt = (uint16_t)(-dev->buffer_size);
    bcnt &= 0x0fff;
    bcnt |= 0xf000; /* high 4 bits fixed 1 */
    des->buf_length = bcnt;

    /* finally, set ownership bit - transmit buffers are owned by us, receive buffers by the card */
    des->status = PCNET32_DESC_STATUS_OWN;
}

static void pcnet32_init_tx_desc_entry(device_extension_t *dev, uint32_t idx)
{
    struct pcnet32_tx_desc *des = dev->tdes + idx;
    memset(des, 0, dev->de_size);

    des->base = kern_vir_addr2phy_addr(dev->tx_buffers + idx * dev->buffer_size);

    /* next 2 bytes are 0xf000 OR'd with the first 12 bits of the 2s complement of the length */
    uint16_t bcnt = (uint16_t)(-dev->buffer_size);
    bcnt &= 0x0fff;
    bcnt |= 0xf000; /* high 4 bits fixed 1 */
    des->buf_length = bcnt;
}

static uint16_t pcnet32_wio_read_mac(uint32_t addr, int index)
{
    return in16(addr + index);
}

/*
 * write index to RAP, read data from RDP
 */
static uint16_t pcnet32_wio_read_csr(uint32_t addr, int index)
{
    out16(addr + PCNET32_WIO_RAP, index);
    return in16(addr + PCNET32_WIO_RDP);
}

/**
 * write index to RAP, write data to RDP
 */
static void pcnet32_wio_write_csr(uint32_t addr, int index, uint16_t val)
{
    out16(addr + PCNET32_WIO_RAP, index);
    out16(addr + PCNET32_WIO_RDP, val);
}

static void pcnet32_wio_write_bcr(uint32_t addr, int index, uint16_t val)
{
    out16(addr + PCNET32_WIO_RAP, index);
    out16(addr + PCNET32_WIO_BDP, val);
}

/*
 * Reset causes the device to cease operation and clear its internal logic.
 */
static void pcnet32_wio_reset(uint32_t addr)
{
    in16(addr + PCNET32_WIO_RESET);
}

static int pcnet32_get_pci(device_extension_t *dev)
{
    /* get pci device */
    pci_device_t *pci_dev = pci_get_device(PCNET32_VENDOR_ID, PCNET32_DEVICE_ID);
    if (pci_dev == NULL)
    {
        errprint("pcnet32: device not find on pci device.\n");
        return -1;
    }
    dev->pci_dev = pci_dev;
    log_print("pcnet32: find device, vendor id: 0x%x, device id: 0x%x\n",
          pci_dev->vendor_id, pci_dev->device_id);

    /* enable bus mastering */
    pci_enable_bus_mastering(pci_dev);

    /* get io port address */
    dev->iobase = pci_device_get_io_addr(pci_dev);
    if (dev->iobase == 0)
    {
        errprint("pcnet32: invalid pci device io address.\n");
        return -1;
    }
    log_print("pcnet32: io base address: 0x%x\n", dev->iobase);
    /* get irq */
    dev->irqno = pci_device_get_irq_line(pci_dev);
    if (dev->irqno == 0xff)
    {
        errprint("pcnet32: invalid irqno.\n");
        return -1;
    }
    dbgprint("pcnet32: irqno %d\n", dev->irqno);
    return 0;
}

static int pcnet32_transmit(device_extension_t *dev, uint8_t *buf, size_t len)
{
    if(len > ETH_FRAME_LEN)
    {
        len = ETH_FRAME_LEN;
    }

    uint32_t tx_retry = PCNET32_TX_RETRY;

    while (tx_retry > 0)
    {
        /* the next available descriptor entry index is in tx_buffer_ptr */
        if(!pcnet32_is_driver_own(dev, TRUE, dev->tx_buffer_ptr))
        {
            /* try encourage the card to send all buffers. */
            pcnet32_wio_write_csr(dev->iobase, CSR0, pcnet32_wio_read_csr(dev->iobase, CSR0) | CSR0_TXPOLL);
        }
        else
        {
            break;
        }
        --tx_retry;
    }

    if (!tx_retry)  /* retry end, no entry available */
    {
        errprint("transmit no available descriptor entry\n");
        return -1;
    }

    memcpy((void *)(dev->tx_buffers + dev->tx_buffer_ptr * dev->buffer_size), buf, len);

    struct pcnet32_tx_desc *tdes = dev->tdes + dev->tx_buffer_ptr;
    /**
     * set the STP bit in the descriptor entry (signals this is the first
     * frame in a split packet - we only support single frames)
     */
    tdes->status |= PCNET32_DESC_STATUS_STP;

    /* similarly, set the ENP bit to state this is also the end of a packet */
    tdes->status |= PCNET32_DESC_STATUS_ENP;

    uint16_t bcnt = (uint16_t)(-len);
    bcnt &= 0xfff;
    bcnt |= 0xf000; /* high 4 bits fixed 1 */
    tdes->buf_length = bcnt;

    /* finally, flip the ownership bit back to the card */
    tdes->status |= PCNET32_DESC_STATUS_OWN;

    dev->tx_buffer_ptr = pcnet32_get_next_desc(dev, dev->tx_buffer_ptr, dev->tx_buffer_count);
    return 0;
}

static iostatus_t pcnet32_read(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len;
    device_extension_t *ext = device->device_extension;
    iostatus_t status = IO_SUCCESS;

    uint8_t *buf = (uint8_t *)ioreq->user_buffer; 
    int flags = 0;

#ifdef DEBUG_DRV    
    keprint(PRINT_DEBUG "pcnet32_read: receive data=%x len=%d flags=%x\n",
        buf, ioreq->parame.read.length, ioreq->parame.read.offset);
#endif
    len = ioreq->parame.read.length;

    if (ext->flags & DEV_NOWAIT) {
        flags |= IO_NOWAIT;
    }
    /* 从网络接收队列中获取一个包 */
    len = io_device_queue_pickup(&ext->rx_queue, buf, len, flags);
    if (len < 0)
        status = IO_FAILED;

#ifdef DEBUG_DRV    
    if (len > 0) {
        log_print("rx:\n");
        buf = (uint8_t *)ioreq->user_buffer; 
        log_dump_buffer(buf, 32, 1);
    }
#endif
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = len;
    
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return status;
}

static iostatus_t pcnet32_write(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len = ioreq->parame.write.length;
    
    uint8_t *buf = (uint8_t *)ioreq->user_buffer; 
#ifdef DEBUG_DRV    
    keprint(PRINT_DEBUG "pcnet32_write: transmit data=%x len=%d flags=%x\n",
        buf, ioreq->parame.write.length, ioreq->parame.write.offset);
#endif
  
    if (pcnet32_transmit(device->device_extension, buf, len) < 0)
        len = -1;

    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

static iostatus_t pcnet32_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;
    unsigned long arg = ioreq->parame.devctl.arg;
    device_extension_t *extension = (device_extension_t *) device->device_extension;
    iostatus_t status = IO_SUCCESS;
    unsigned char *mac;

    switch (ctlcode)
    {
    case NETIO_GETMAC:
        mac = (unsigned char *) arg;
        *mac++ = extension->dev_addr[0];
        *mac++ = extension->dev_addr[1];
        *mac++ = extension->dev_addr[2];
        *mac++ = extension->dev_addr[3];
        *mac++ = extension->dev_addr[4];
        *mac++ = extension->dev_addr[5];
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "pcnet32_devctl: copy mac addr to addr %x\n", ioreq->parame.devctl.arg);
#endif
        break;
    case NETIO_SETFLGS:
        extension->flags = *((unsigned long *) arg);
        break;
    case NETIO_GETFLGS:
        *((unsigned long *) arg) = extension->flags;
        break;
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

static void pcnet32_rx_packet(device_extension_t *dev)
{
    while (pcnet32_is_driver_own(dev, FALSE, dev->rx_buffer_ptr))
    {
        struct pcnet32_rx_desc *rdes = dev->rdes + dev->rx_buffer_ptr;
        uint32_t plen = rdes->msg_length; /* msg len no need to negate it unlike BCNT above */

        void *pbuf = (void *)(dev->rx_buffers + dev->rx_buffer_ptr * dev->buffer_size);
        // log_print("recv packet on ring %d: buf=%p, len=%d\n", dev->rx_buffer_ptr, pbuf, plen);
        /* merge size and data into receive pkg */
        io_device_queue_append(&dev->rx_queue, pbuf, plen);

        /* hand the buffer back to the card */
        rdes->status = PCNET32_DESC_STATUS_OWN;

        dev->rx_buffer_ptr = pcnet32_get_next_desc(dev, dev->rx_buffer_ptr, dev->rx_buffer_count);
    }
}

static int pcnet32_handler(irqno_t irq, void *data)
{
    device_extension_t *dev = GET_PCNET32(data);
    int intrhandled = IRQ_HANDLED;

    uint32_t iobase = dev->iobase;
    uint32_t csr0 = pcnet32_wio_read_csr(iobase, 0);

    if (csr0 & CSR0_RINT) /* recv packet */
    {
        log_print("RX intr occur!\n");
        pcnet32_rx_packet(dev);
    }
    else if ((csr0 & CSR0_TINT))    /* packet transmitted */
    {
        log_print("TX intr occur!\n");
    }
    else if ((csr0 & CSR0_IDON))
    {
        log_print("init done\n");
    }
    else if ((csr0 & CSR0_MERR))
    {
        warnprint("memory error!\n");
    }
    else if ((csr0 & CSR0_MISS))
    {
        warnprint("missed frame!\n");
    }
    else if ((csr0 & CSR0_CERR))
    {
        warnprint("collision error!\n");
    }
    else if ((csr0 & CSR0_BABL))
    {
        warnprint("transmitter time-out error!\n");
    }
    else
    {
        intrhandled = IRQ_NEXTONE;
        warnprint("unknown intr\n");
    }
    /* ack pcnet32 interrupt as handled */
    pcnet32_wio_write_csr(iobase, 0, csr0);
    return intrhandled;
}

static int pcnet32_alloc_ring_buffer(device_extension_t *dev)
{
    dev->rdes = mem_alloc_align(dev->rx_buffer_count * dev->de_size, 16);
    if (dev->rdes == NULL)
    {
        errprint("alloc memory for rx ring failed!");
        return -1;
    }
    dev->tdes = mem_alloc_align(dev->tx_buffer_count * dev->de_size, 16);
    if (dev->tdes == NULL)
    {
        errprint("alloc memory for tx ring failed!");
        mem_free_align(dev->rdes);
        return -1;
    }

    dev->rx_buffers = (uint32_t)mem_alloc_align(dev->rx_buffer_count * dev->buffer_size, 16);
    if (dev->rx_buffers == 0)
    {
        errprint("alloc memory for rx ring buffer failed!");
        mem_free_align(dev->rdes);
        mem_free_align(dev->tdes);
        return -1;
    }

    dev->tx_buffers = (uint32_t)mem_alloc_align(dev->tx_buffer_count * dev->buffer_size, 16);
    if (dev->tx_buffers == 0)
    {
        errprint("alloc memory for tx ring buffer failed!");
        mem_free_align(dev->rdes);
        mem_free_align(dev->tdes);
        mem_free_align((void *)dev->rx_buffers);
        return -1;
    }
    log_print("rdes:%p tdes:%p rbuf:%p tbuf:%p\n", dev->rdes, dev->tdes, dev->rx_buffers, dev->tx_buffers);

    int i = 0;
    for (i = 0; i < dev->rx_buffer_count; i++)
    {
        pcnet32_init_rx_desc_entry(dev, i);
    }
    for (i = 0; i < dev->tx_buffer_count; i++)
    {
        pcnet32_init_tx_desc_entry(dev, i);
    }
    return 0;
}

static void pcnet32_free_ring_buffer(device_extension_t *dev)
{
    mem_free_align(dev->rdes);
    mem_free_align(dev->tdes);
    mem_free_align((void *)dev->rx_buffers);
    mem_free_align((void *)dev->tx_buffers);
}

#if DEBUG_INITBLOCK == 1
static void pcnet32_print_init_block(device_extension_t *dev)
{
    uint32_t iobase = dev->iobase;

    struct pcnet32_init_block *init_block = dev->init_block;
    log_print("============\nprint init block\n");
    log_print("mode: %x, tlen_rlen:%x\n", init_block->mode, init_block->tlen_rlen);
    log_print("mac: %x:%x:%x:%x:%x:%x\n",
            init_block->phys_addr[0],
            init_block->phys_addr[1],
            init_block->phys_addr[2],
            init_block->phys_addr[3],
            init_block->phys_addr[4],
            init_block->phys_addr[5]);
    log_print("filter0: %x, filter1: %x\n", init_block->filter[0], init_block->filter[1]);
    log_print("rx ring dma: %x, tx ring dma: %x\n", init_block->rx_ring, init_block->tx_ring);
    log_print("init block dma: %x\n", dev->init_block_dma_addr);

    int i = 0;
    for (; i <= 46; i++)
    {
        log_print("csr%d=%x\n", i, pcnet32_wio_read_csr(iobase, i));
    }
}
#endif

static int pcnet32_init(device_extension_t *dev)
{
    uint32_t iobase = dev->iobase;

    dev->flags = 0;

    /* init buffer info */
    dev->rx_buffer_ptr = 0;
    dev->tx_buffer_ptr = 0;

    dev->rx_buffer_count = PCNET32_RX_BUFFERS;
    dev->tx_buffer_count = PCNET32_TX_BUFFERS;

    dev->buffer_size = ETH_FRAME_LEN;
    dev->de_size = PCNET32_RING_DE_SIZE;

    if (pcnet32_alloc_ring_buffer(dev) != 0)
    {
        return -1;
    }

    dev->rx_ring_dma_addr = (uint32_t)kern_vir_addr2phy_addr(dev->rdes);
    dev->tx_ring_dma_addr = (uint32_t)kern_vir_addr2phy_addr(dev->tdes);

    /* 初始化接收队列，用内核队列结构保存，等待被读取 */
    io_device_queue_init(&dev->rx_queue);

    /* alloc init block, must 16 bit align */
    dev->init_block = mem_alloc_align(sizeof(struct pcnet32_init_block), 16);
    if (dev->init_block == NULL)
    {
        errprint("alloc memory for init block failed!");

        pcnet32_free_ring_buffer(dev);
        return -1;
    }
    dev->init_block_dma_addr = (uint32_t)kern_vir_addr2phy_addr(dev->init_block);

    log_print("init block addr:%p size:%d\n", dev->init_block, sizeof(struct pcnet32_init_block));

    /* fill init block */
    dev->init_block->mode = 0;
    dev->tx_len_bits = (PCNET32_LOG_TX_BUFFERS << 4);
    dev->rx_len_bits = (PCNET32_LOG_RX_BUFFERS << 4);
    dev->init_block->tlen_rlen = (dev->tx_len_bits << 8) | dev->rx_len_bits;
    int i = 0;
    for (i = 0; i < ETH_ALEN; i++)
    {
        dev->init_block->phys_addr[i] = dev->dev_addr[i];
    }
    dev->init_block->filter[0] = 0x00000000;
    dev->init_block->filter[1] = 0x00000000;
    dev->init_block->rx_ring = dev->rx_ring_dma_addr;
    dev->init_block->tx_ring = dev->tx_ring_dma_addr;

    /* register init block, CSR1 save low 16 bit, CSR1 save high 16 bit */
    pcnet32_wio_write_csr(iobase, CSR1, (dev->init_block_dma_addr & 0xffff));
    pcnet32_wio_write_csr(iobase, CSR2, (dev->init_block_dma_addr >> 16) & 0xffff);

    /* register intr */
    if (irq_register(dev->irqno, pcnet32_handler, IRQF_SHARED, "IRQ-Network", DEV_NAME, (void *) dev) < 0)
    {
        errprint("install IRQ failed!\n");
        mem_free_align(dev->init_block);

        pcnet32_free_ring_buffer(dev);
        return -1;
    }

    /* Start init */
    pcnet32_wio_write_csr(iobase, CSR0, CSR0_INIT | CSR0_INTEN);
    log_print("card init done.\n");

    /* add auto pad amd strip recv */
    uint16_t csr4 = pcnet32_wio_read_csr(iobase, CSR4);
    pcnet32_wio_write_csr(iobase, CSR4, csr4 | CSR4_ASTRP_RCV | CSR4_APAD_XMT);

    /* start work */
    pcnet32_wio_write_csr(iobase, CSR0, CSR0_START | CSR0_INTEN);

#if DEBUG_INITBLOCK == 1
    pcnet32_print_init_block(dev);
#endif

    return 0;
}

static int pcnet32_init_hw(device_extension_t *dev)
{
    uint32_t iobase = dev->iobase;

    /* reset card to 16 bit io mode */
    pcnet32_wio_reset(iobase);

    /* use dealy to wait reset done, at least 1 microsecond */
    udelay(1000);
    
    /* switch to 32 bit soft-style mode, use 32 bit struct */
    pcnet32_wio_write_bcr(iobase, 20, 0x102);

    /* stop card work */
    pcnet32_wio_write_csr(iobase, 0, 0x4);

    /* read mac addr */
    uint16_t mac0 = pcnet32_wio_read_mac(iobase, 0);
    uint16_t mac1 = pcnet32_wio_read_mac(iobase, 2);
    uint16_t mac2 = pcnet32_wio_read_mac(iobase, 4);

    dev->dev_addr[0] = mac0 & 0xff;
    dev->dev_addr[1] = (mac0 >> 8) & 0xff;
    dev->dev_addr[2] = mac1 & 0xff;
    dev->dev_addr[3] = (mac1 >> 8) & 0xff;
    dev->dev_addr[4] = mac2 & 0xff;
    dev->dev_addr[5] = (mac2 >> 8) & 0xff;

    dbgprint("MAC addr: %x:%x:%x:%x:%x:%x\n",
        dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
        dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

    return pcnet32_init(dev);
}

static iostatus_t pcnet32_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *devext;
    
    dbgprint("[pcnet32] init.\n");

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_PHYSIC_NETCARD, &devobj);

    if (status != IO_SUCCESS) {
        keprint(PRINT_DEBUG PRINT_ERR "pcnet32_enter: create device failed!\n");
        return status;
    }
    /* neither io mode */
    devobj->flags = 0;
    
    devext = (device_extension_t *)devobj->device_extension;
    
    if (pcnet32_get_pci(devext)) {
        status = IO_FAILED;
        io_delete_device(devobj);
        return status;
    }

    if (pcnet32_init_hw(devext)) {
        status = IO_FAILED;
        io_delete_device(devobj);
        return status;
    }

    dbgprint("[pcnet32] init success.\n");

    return status;
}

static iostatus_t pcnet32_exit(driver_object_t *driver)
{
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        io_delete_device(devobj);   /* 删除每一个设备 */
    }

    string_del(&driver->name); /* 删除驱动名 */
    return IO_SUCCESS;
}

iostatus_t pcnet32_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = pcnet32_enter;
    driver->driver_exit = pcnet32_exit;

    driver->dispatch_function[IOREQ_READ] = pcnet32_read;
    driver->dispatch_function[IOREQ_WRITE] = pcnet32_write;
    driver->dispatch_function[IOREQ_DEVCTL] = pcnet32_devctl;

    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "pcnet32_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    return status;
}

static __init void pcnet32_driver_entry(void)
{
    if (driver_object_create(pcnet32_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(pcnet32_driver_entry);
