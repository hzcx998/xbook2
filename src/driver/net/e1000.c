#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <assert.h>
#include <xbook/byteorder.h>
#include <xbook/spinlock.h>
#include <math.h>
#include <xbook/waitqueue.h>
#include <xbook/kmalloc.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <xbook/pci.h>
#include <arch/atomic.h>
#include <arch/cpu.h>
#include <sys/ioctl.h>
#include <stddef.h>

#include <net/e1000_hw.h>
#include <net/e1000_osdep.h>
#include <net/e1000.h>
#include <net/kcompat.h>

#define DRV_VERSION "v0.1"
#define DRV_NAME "net-e1000" DRV_VERSION

#define DEV_NAME "e1000"
#define E1000_VENDOR_ID 0x8086

#define DEBUG_LOCAL 0

/*以太网的情况*/
#define ETH_ALEN 6 /*以太网地址，即MAC地址，6字节*/
#define ETH_ZLEN 60 /*不含CRC校验的数据最小长度*/
#define ETH_DATA_LEN 1500 /*帧内数据的最大长度*/
#define ETH_FRAME_LEN 1514 /*不含CRC校验和的最大以太网数据长度*/

/*  设备状态 */
struct net_device_status {
    /* 错误记录 */
    unsigned long tx_errors;         /* 传输错误记录 */
    unsigned long tx_aborted_errors;  /* 传输故障记录 */
    unsigned long tx_carrier_errors;  /* 传输携带错误记录 */
    unsigned long tx_window_errors;   /* 传输窗口错误记录 */
    unsigned long tx_fifo_errors;     /* 传输fifo错误记录 */
    unsigned long tx_dropped;        /* 传输丢弃记录 */

    unsigned long rx_errors;         /* 接收错误 */    
    unsigned long rx_length_errors;   /* 接收长度错误 */
    unsigned long rx_missed_errors;   /* 接收丢失错误 */
    unsigned long rx_fifo_errors;     /* 接收Fifo错误 */
    unsigned long rx_crc_errors;      /* 接收CRC错误 */
    unsigned long rx_frame_errors;    /* 接收帧错误 */
    unsigned long rx_dropped;        /* 接收丢弃记录 */
    /* 正确记录 */
    unsigned long tx_packets;        /* 传输包数量 */
    unsigned long tx_bytes;          /* 传输字节数量 */
    
    unsigned long collisions;       /* 碰撞次数 */
};

/* 设备拓展内容 */
typedef struct _e1000_82540em_extension {
    device_object_t *device_object;   //设备对象

    uint32_t io_addr;   //IO基地址
    int drv_flags;   //驱动标志
    uint32_t irq;
    uint8_t mac_addr[6];
    flags_t flags;
    pci_device_t *pci_device;

    uint32_t dev_features;   //设备结构特征

    uint8_t *rx_buffer;   //接收缓冲区
    uint8_t *rx_ring;   //接收环
    uint8_t current_rx;
    flags_t rx_flags;
    dma_addr_t rx_ring_dma;   //dma物理地址

    spinlock_t lock;   //普通锁
    spinlock_t rx_lock;   //接收锁

    device_queue_t rx_queue;   //接收队列
}e1000_82540em_extension_t;

/**
 * 1. 申请pci结构(pci_device_t)并初始化厂商号和设备号
 * 2. 启动总线控制
 * 3. 申请设备io空间，在pci_device中登记io空间基地址
 * 4. 申请中断，并在pci_device中登记中断号
**/
static int e1000_82540em_get_pci_info(e1000_82540em_extension_t* ext)
{
    /* get pci device*/
    pci_device_t* pci_device = pci_get_device(E1000_VENDOR_ID, E1000_DEV_ID_82540EM);
    if(pci_device == NULL) {
        printk(KERN_DEBUG "E1000_82540EM init failed: pci_get_device.\n");
        return -1;
    }
    ext->pci_device = pci_device;
#if DEBUG_LOCAL == 1    
    printk(KERN_DEBUG "find E1000_82540EM device, vendor id: 0x%x, device id: 0x%x\n",\
            device->vendor_id, device->device_id);
#endif
    /* enable bus mastering */
    pci_enable_bus_mastering(pci_device);

    /* get io address */
    ext->io_addr = pci_device_get_io_addr(pci_device);
    if(ext->io_addr == 0) {
        printk(KERN_DEBUG "E1000_82540EM init failed: INVALID pci device io address.\n");
        return -1;
    }
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "E1000_81540EM io address: 0x%x\n", ext->io_addr);
#endif
    /* get irq */
    ext->irq = pci_device_get_irq_line(pci_device);
    if(ext->irq == 0xff) {
        printk(KERN_DEBUG "E1000_82540EM init failed: INVALID irq.\n");
        return -1;
    }
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "E1000_82540EM irq: %d\n", ext->irq);
#endif
    return 0;
}

static int e1000_82540em_init_board(e1000_82540em_extension_t* ext)
{
    /* check for missing/broken hardware*/
    /* 检查丢失/损坏的硬件 */

    /* 判断是什么芯片，这里暂时默认为82540EM芯片 */

    /* 网卡加电，启动*/
}

static int e1000_82540em_init(e1000_82540em_extension_t* ext)
{
    static int board_idx = -1;

    ASSERT(ext);

    board_idx++;

    pci_device_t* pdev = ext->pci_device;

    /* 对版本进行检测，仅支持82540em网卡 */
    if(pdev->vendor_id != E1000_VENDOR_ID && pdev->device_id != E1000_DEV_ID_82540EM) {
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "this deiver only support 82540em netcard.\n");
#endif
    return -1;
    }

    /* 初始化版本设定 */
}

static iostatus_t e1000_enter(driver_object_t* driver)
{
    iostatus_t status;

    device_object_t *devobj;
    e1000_82540em_extension_t* devext;
    
    /* 初始化一些其他内容-初始化devobj(未扩展的内容) */
    status = io_create_device(driver, sizeof(e1000_82540em_extension_t), DEV_NAME, DEVICE_TYPE_PHYSIC_NETCARD, &devobj);

    if(status != IO_SUCCESS) {
        printk(KERN_DEBUG KERN_ERR "e1000_enter: create device failed!\n");
        return status;
    }

    /* neither io mode*/
    devobj->flags = 0;

    devext = (e1000_82540em_extension_t*)devobj->device_extension;
    devext->device_object = devobj;

    /*初始化接收队列，用内核队列结构保存，等待被读取*/
    io_device_queue_init(&devext->rx_queue);

    /* 申请并初始化pci_device_t*/
    if(e1000_82540em_get_pci_info(devext)) {
        status = IO_FAILED;
        io_delete_device(devobj);
        return status;
    }
}
