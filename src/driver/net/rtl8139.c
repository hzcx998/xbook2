
/*

	8139too.c: A RealTek RTL-8139 Fast Ethernet driver for Linux.

	Maintained by Jeff Garzik <jgarzik@pobox.com>
	Copyright 2000-2002 Jeff Garzik

	Much code comes from Donald Becker's rtl8139.c driver,
	versions 1.13 and older.  This driver was originally based
	on rtl8139.c version 1.07.  Header of rtl8139.c version 1.13:

	-----<snip>-----

        	Written 1997-2001 by Donald Becker.
		This software may be used and distributed according to the
		terms of the GNU General Public License (GPL), incorporated
		herein by reference.  Drivers based on or derived from this
		code fall under the GPL and must retain the authorship,
		copyright and license notice.  This file is not a complete
		program and may only be used when the entire operating
		system is licensed under the GPL.

		This driver is for boards based on the RTL8129 and RTL8139
		PCI ethernet chips.

		The author may be reached as becker@scyld.com, or C/O Scyld
		Computing Corporation 410 Severn Ave., Suite 210 Annapolis
		MD 21403

		Support and updates available at
		http://www.scyld.com/network/rtl8139.html

		Twister-tuning table provrtl8139d by Kinston
		<shangh@realtek.com.tw>.

	-----<snip>-----

	This software may be used and distributed according to the terms
	of the GNU General Public License, incorporated herein by reference.

	Contributors:

		Donald Becker - he wrote the original driver, kudos to him!
		(but please don't e-mail him for support, this isn't his driver)

		Tigran Aivazian - bug fixes, skbuff free cleanup

		Martin Mares - suggestions for PCI cleanup

		David S. Miller - PCI DMA and softnet updates

		Ernst Gill - fixes ported from BSD driver

		Daniel Kobras - rtl8139ntified specific locations of
			posted MMIO write bugginess

		Gerard Sharp - bug fix, testing and feedback

		David Ford - Rx ring wrap fix

		Dan DeMaggio - swapped RTL8139 cards with me, and allowed me
		to find and fix a crucial bug on older chipsets.

		Donald Becker/Chris Butterworth/Marcus Westergren -
		Noticed various Rx packet size-related buglets.

		Santiago Garcia Mantinan - testing and feedback

		Jens David - 2.2.x kernel backports

		Martin Dennett - incredibly helpful insight on undocumented
		features of the 8139 chips

		Jean-Jacques Michel - bug fix

		Tobias Ringström - Rx interrupt status checking suggestion

		Andrew Morton - Clear blocked signals, avoid
		buffer overrun setting current->comm.

		Kalle Olavi Niemitalo - Wake-on-LAN ioctls

		Robert Kuebel - Save kernel thread from dying on any signal.

    -----<snip>-----
    This was transplanted to BookOS by Jason Hu.
    2020.4.22   
    www.book-os.org

	Submitting bug reports:

		"rtl8139-diag -mmmaaavvveefN" output
		enable RTL8139_DEBUG below, and look at 'dmesg' or kernel log

*/

/*
主要流程:
1.从PCI获取网卡信息
2.初始化网卡配置空间以及中断
3.读取网卡信息，并初始化
4.重启网卡
5.网卡发送
6.网卡接收
*/
#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

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

#define DRV_VERSION "v0.1"
#define DRV_NAME "net-rtl8139" DRV_VERSION

#define DEV_NAME "rtl8139"

// #define DEBUG_DRV

/*备注以太网的情况*/
#define ETH_ALEN 6 /*以太网地址，即MAC地址，6字节*/
#define ETH_ZLEN 60 /*不含CRC校验的数据最小长度*/
#define ETH_DATA_LEN 1500 /*帧内数据的最大长度*/
#define ETH_FRAME_LEN 1514 /*不含CRC校验和的最大以太网数据长度*/

/* PCI rtl8139 配置空间寄存器 */
#define RTL8139_VENDOR_ID   0x10ec
#define RTL8139_DEVICE_ID   0x8139

/* ----定义接收和传输缓冲区的大小---- */

#define RX_BUF_IDX	2	/* 32K ring */

#define RX_BUF_LEN	(8192 << RX_BUF_IDX)
#define RX_BUF_PAD	16      /* 填充16字节 */
#define RX_BUF_WRAP_PAD 2048 /* spare padding to handle lack of packet wrap */

/* 接收缓冲区的总长度 */
#define RX_BUF_TOT_LEN	(RX_BUF_LEN + RX_BUF_PAD + RX_BUF_WRAP_PAD)

/* Number of Tx descriptor registers. */
#define NUM_TX_DESC	4

/* max supported ethernet frame size -- must be at least (dev->mtu+14+4).*/
#define MAX_ETH_FRAME_SIZE	1536

/* Size of the Tx bounce buffers -- must be at least (dev->mtu+14+4). */
#define TX_BUF_SIZE	MAX_ETH_FRAME_SIZE
#define TX_BUF_TOT_LEN	(TX_BUF_SIZE * NUM_TX_DESC)

/* PCI Tuning Parameters
   Threshold is bytes transferred to chip before transmission starts. */
#define TX_FIFO_THRESH  256	/* In bytes, rounded down to 32 byte units. */

/* The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024, 7==end of packet. */
#define RX_FIFO_THRESH	7	/* Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST	7	/* Maximum PCI burst, '7' is unlimited */
#define TX_DMA_BURST	6	/* Maximum PCI burst, '6' is 1024 */
#define TX_RETRY	8	/* 0-15.  retries = 16 + (TX_RETRY * 16) */

enum {
	HAS_MII_XCVR = 0x010000,
	HAS_CHIP_XCVR = 0x020000,
	HAS_LNK_CHNG = 0x040000,
};

#define RTL_NUM_STATS 4		/* number of ETHTOOL_GSTATS u64's */
#define RTL_REGS_VER 1		/* version of reg. data in ETHTOOL_GREGS */
#define RTL_MIN_IO_SIZE 0x80
#define RTL8139B_IO_SIZE 256
#define RTL8129_CAPS	HAS_MII_XCVR
#define RTL8139_CAPS	(HAS_CHIP_XCVR|HAS_LNK_CHNG)

typedef enum {
	RTL8139 = 0,
	RTL8129,
} board_t;

#if 0
/* indexed by board_t, above */
static const struct {
	const char *name;
	u32 hw_flags;
} borad_info[] = {
	{ "RealTek RTL8139", RTL8139_CAPS },
	{ "RealTek RTL8129", RTL8129_CAPS },
};
#endif

/* Symbolic offsets to registers. */
enum rtl8139_registers {
	MAC0		= 0,	 /* Ethernet hardware address. */
	MAR0		= 8,	 /* Multicast filter. */
	TX_STATUS0	= 0x10,	 /* Transmit status (Four 32bit registers). */
	TX_ADDR0    = 0x20,	 /* Tx descriptors (also four 32bit). */
	RX_BUF		= 0x30,
	CHIP_CMD	= 0x37,
	RX_BUF_PTR	= 0x38,
	RX_BUF_ADDR	= 0x3A,
	INTR_MASK	= 0x3C,
	INTR_STATUS	= 0x3E,
	TX_CONFIG	= 0x40,
	RX_CONFIG	= 0x44,
	TIMER		= 0x48,	 /* A general-purpose counter. */
	RX_MISSED	= 0x4C,  /* 24 bits valid, write clears. */
	CFG9346		= 0x50,
	CONFIG0		= 0x51,
	CONFIG1		= 0x52,
	TIMER_INT	= 0x54,
	MEDIA_STATUS= 0x58,
	CONFIG3		= 0x59,
	CONFIG4		= 0x5A,	 /* absent on RTL-8139A */
	HLT_CTL		= 0x5B,
	MULTI_INTR	= 0x5C,
	TX_SUMMARY	= 0x60,
	BASIC_MODE_CTRL	    = 0x62,
	BASIC_MODE_STATUS	= 0x64,
	NWAY_ADVERT	= 0x66,
	NWAY_LPAR	= 0x68,
	NWAY_EXPANSION	= 0x6A,
	/* Undocumented registers, but required for proper operation. */
	FIFOTMS		= 0x70,	 /* FIFO Control and test. */
	CSCR		= 0x74,	 /* Chip Status and Configuration Register. */
	PARA78		= 0x78,
	FLASH_REG	= 0xD4,	/* Communication with Flash ROM, four bytes. */
	PARA7c		= 0x7c,	 /* Magic transceiver parameter register. */
	CONFIG5		= 0xD8,	 /* absent on RTL-8139A */
};

enum clear_bit_masks {
	MULTI_INTR_CLEAR	= 0xF000,
	CHIP_CMD_CLEAR	= 0xE2,
	CONFIG1_CLEAR	= (1<<7)|(1<<6)|(1<<3)|(1<<2)|(1<<1),
};

enum chip_cmd_bits {
	CMD_RESET	    = 0x10,
	CMD_RX_ENABLE	= 0x08,
	CMD_TX_ENABLE	= 0x04,
	RX_BUFFER_EMPTY	= 0x01,
};

/* Interrupt register bits, using my own meaningful names.
中断状态位
 */
enum intr_status_bits {
	PCI_ERR		= 0x8000,
	PCS_TIMEOUT	= 0x4000,
	RX_FIFO_OVER= 0x40,
	RX_UNDERRUN	= 0x20,
	RX_OVERFLOW	= 0x10,
	TX_ERR		= 0x08,
	TX_OK		= 0x04,
	RX_ERR		= 0x02,
	RX_OK		= 0x01,
	RX_ACK_BITS	= RX_FIFO_OVER | RX_OVERFLOW | RX_OK,
};

/* 传输状态位 */
enum tx_status_bits {
	TX_HOST_OWNS	= 0x2000,
	TX_UNDERRUN	    = 0x4000,
	TX_STAT_OK	    = 0x8000,
	TX_OUT_OF_WINDOW= 0x20000000,
	TX_ABORTED	    = 0x40000000,
	TX_CARRIER_LOST	= 0x80000000,
};

/* 接收状态位 */
enum rx_status_bits {
	RX_MULTICAST	= 0x8000,
	RX_PHYSICAL	    = 0x4000,
	RX_BROADCAST    = 0x2000,
	RX_BAD_SYMBOL	= 0x0020,
	RX_RUNT		    = 0x0010,
	RX_TOO_LONG	    = 0x0008,
	RX_CRC_ERR	    = 0x0004,
	RX_BAD_Align	= 0x0002,
	RX_STATUS_OK    = 0x0001,
};

/* Bits in rx_config.
接收模式位
 */
enum rx_mode_bits {
	ACCEPT_ERR	= 0x20,
	ACCEPT_RUNT	= 0x10,
	ACCEPT_BROADCAST	= 0x08,
	ACCEPT_MULTICAST	= 0x04,
	ACCEPT_MY_PHYS	= 0x02,
	ACCEPT_ALL_PHYS	= 0x01,
};

/* Bits in TxConfig. */
enum tx_config_bits {
    /* Interframe Gap Time. Only TxIFG96 doesn't violate IEEE 802.3 */
    TX_IFG_SHIFT	= 24,
    TX_IFG84		= (0 << TX_IFG_SHIFT), /* 8.4us / 840ns (10 / 100Mbps) */
    TX_IFG88		= (1 << TX_IFG_SHIFT), /* 8.8us / 880ns (10 / 100Mbps) */
    TX_IFG92		= (2 << TX_IFG_SHIFT), /* 9.2us / 920ns (10 / 100Mbps) */
    TX_IFG96		= (3 << TX_IFG_SHIFT), /* 9.6us / 960ns (10 / 100Mbps) */

	TX_LOOP_BACK	= (1 << 18) | (1 << 17), /* enable loopback test mode */
	TX_CRC		    = (1 << 16),	/* DISABLE Tx pkt CRC append */
	TX_CLEAR_ABT	= (1 << 0),	/* Clear abort (WO) */
	TX_DMA_SHIFT	= 8, /* DMA burst value (0-7) is shifted X many bits */
	TX_RETRY_SHIFT	= 4, /* TXRR value (0-15) is shifted X many bits */

	TX_VERSION_MASK	= 0x7C800000, /* mask out version bits 30-26, 23 */
};

/* Bits in Config1 */
enum config1_bits {
	CFG1_PM_ENABLE	= 0x01,
	CFG1_VPD_ENABLE	= 0x02,
	CFG1_PIO	= 0x04,
	CFG1_MMIO	= 0x08,
	LWAKE		= 0x10,		/* not on 8139, 8139A */
	CFG1_DRIVER_LOAD = 0x20,
	CFG1_LED0	= 0x40,
	CFG1_LED1	= 0x80,
	SLEEP		= (1 << 1),	/* only on 8139, 8139A */
	PWRDN		= (1 << 0),	/* only on 8139, 8139A */
};

/* Bits in Config3 */
enum config3_bits {
	CFG3_FAST_ENABLE   	    = (1 << 0), /* 1	= Fast Back to Back */
	CFG3_FUNCTION_ENABLE	= (1 << 1), /* 1	= enable CardBus Function registers */
	CFG3_CLKRUN_ENABLE	    = (1 << 2), /* 1	= enable CLKRUN */
	CFG3_CARD_BUS_ENABLE 	= (1 << 3), /* 1	= enable CardBus registers */
	CFG3_LINK_UP   	        = (1 << 4), /* 1	= wake up on link up */
	CFG3_MAGIC    	        = (1 << 5), /* 1	= wake up on Magic Packet (tm) */
	CFG3_PARM_ENABLE  	    = (1 << 6), /* 0	= software can set twister parameters */
	CFG3_GNT   	            = (1 << 7), /* 1	= delay 1 clock from PCI GNT signal */
};

/* Bits in Config4 */
enum config4_bits {
	LWPTN	= (1 << 2),	/* not on 8139, 8139A */
};

/* Bits in Config5 */
enum config5_bits {
	Cfg5_PME_STS   	= (1 << 0), /* 1	= PCI reset resets PME_Status */
	Cfg5_LANWake   	= (1 << 1), /* 1	= enable LANWake signal */
	Cfg5_LDPS      	= (1 << 2), /* 0	= save power when link is down */
	Cfg5_FIFOAddrPtr= (1 << 3), /* Realtek internal SRAM testing */
	Cfg5_UWF        = (1 << 4), /* 1 = accept unicast wakeup frame */
	Cfg5_MWF        = (1 << 5), /* 1 = accept multicast wakeup frame */
	Cfg5_BWF        = (1 << 6), /* 1 = accept broadcast wakeup frame */
};

enum rx_config_bits {
	/* rx fifo threshold */
	RX_CFG_FIFO_SHIFT	= 13,
	RX_CFG_FIFO_NONE	= (7 << RX_CFG_FIFO_SHIFT),

	/* Max DMA burst */
	RX_CFG_DMA_SHIFT	= 8,
	RX_CFG_DMA_UNLIMITED = (7 << RX_CFG_DMA_SHIFT),

	/* rx ring buffer length */
	RX_CFG_RCV_8K	= 0,
	RX_CFG_RCV_16K	= (1 << 11),
	RX_CFG_RCV_32K	= (1 << 12),
	RX_CFG_RCV_64K	= (1 << 11) | (1 << 12),

	/* Disable packet wrap at end of Rx buffer. (not possible with 64k) */
	RX_NO_WRAP	= (1 << 7),
};

/* Twister tuning parameters from RealTek.
   Completely undocumented, but required to tune bad links on some boards. */
enum cscr_bits {
	CSCR_LINK_OK		= 0x0400,
	CSCR_LINK_CHANGE	= 0x0800,
	CSCR_LINK_STATUS	= 0x0f000,
	CSCR_LINK_DOWN_OFF_CMD	= 0x003c0,
	CSCR_LINK_DOWN_CMD	= 0x0f3c0,
};

enum config9346_bits {
	CFG9346_LOCK	= 0x00,
	CFG9346_UNLOCK	= 0xC0,
};

typedef enum {
	CH_8139	= 0,
	CH_8139_K,
	CH_8139A,
	CH_8139A_G,
	CH_8139B,
	CH_8130,
	CH_8139C,
	CH_8100,
	CH_8100B_8139D,
	CH_8101,
} chip_t;

enum chip_flags {
	HAS_HLT_CLK	= (1 << 0),
	HAS_LWAKE	= (1 << 1),
};

#define HW_REVID(b30, b29, b28, b27, b26, b23, b22) \
	(b30<<30 | b29<<29 | b28<<28 | b27<<27 | b26<<26 | b23<<23 | b22<<22)
#define HW_REVID_MASK	HW_REVID(1, 1, 1, 1, 1, 1, 1)

#define CHIP_INFO_NR    10


/* directly indexed by chip_t, above */
static const struct {
	const char *name;
	u32 version; /* from RTL8139C/RTL8139D docs */
	u32 flags;
} rtl_chip_info[CHIP_INFO_NR] = {
	{ "RTL-8139",
	  HW_REVID(1, 0, 0, 0, 0, 0, 0),
	  HAS_HLT_CLK,
	},

	{ "RTL-8139 rev K",
	  HW_REVID(1, 1, 0, 0, 0, 0, 0),
	  HAS_HLT_CLK,
	},

	{ "RTL-8139A",
	  HW_REVID(1, 1, 1, 0, 0, 0, 0),
	  HAS_HLT_CLK, /* XXX undocumented? */
	},

	{ "RTL-8139A rev G",
	  HW_REVID(1, 1, 1, 0, 0, 1, 0),
	  HAS_HLT_CLK, /* XXX undocumented? */
	},

	{ "RTL-8139B",
	  HW_REVID(1, 1, 1, 1, 0, 0, 0),
	  HAS_LWAKE,
	},

	{ "RTL-8130",
	  HW_REVID(1, 1, 1, 1, 1, 0, 0),
	  HAS_LWAKE,
	},

	{ "RTL-8139C",
	  HW_REVID(1, 1, 1, 0, 1, 0, 0),
	  HAS_LWAKE,
	},

	{ "RTL-8100",
	  HW_REVID(1, 1, 1, 1, 0, 1, 0),
 	  HAS_LWAKE,
 	},

	{ "RTL-8100B/8139D",
	  HW_REVID(1, 1, 1, 0, 1, 0, 1),
	  HAS_HLT_CLK /* XXX undocumented? */
	| HAS_LWAKE,
	},

	{ "RTL-8101",
	  HW_REVID(1, 1, 1, 0, 1, 1, 1),
	  HAS_LWAKE,
	},
};


struct rtl_extra_status {
	unsigned long early_rx;
	unsigned long txBufMapped;
	unsigned long txTimeouts;
	unsigned long rx_lost_in_ring;
};


struct rtl8139_status {
	u64	packets;
	u64	bytes;
};


/* 网络框架结构 */

/* feature */
#define NET_FEATURE_RXALL     (1 << 0)        // 接收所有包
#define NET_FEATURE_RXFCS     (1 << 1)        // 接收所有包

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

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */

    unsigned int io_addr;
    int drv_flags;           /* 驱动标志 */
    unsigned int irq;
    unsigned char mac_addr[6];
	flags_t flags;
	pci_device_t *pci_device;

    unsigned int dev_features;       /* 设备结构特征 */

	struct net_device_status stats;  /* 设备状态 */
    struct rtl_extra_status xstats;    /* 扩展状态 */            

    unsigned char *rx_buffer;
	unsigned char *rx_ring;
    unsigned char  current_rx;   /* CAPR, Current Address of Packet Read */
    flags_t rx_flags;
    dma_addr_t rx_ring_dma;       /* dma物理地址 */
    struct rtl8139_status	rx_status;

    unsigned char *tx_buffers;
    unsigned char *tx_buffer[NUM_TX_DESC];
    unsigned long   current_tx;     /* 当前的传输 */
    unsigned long   dirty_tx;       /* 使用过的传输 */
    atomic_t   tx_free_counts;   /* 传输空闲数量 */
    flags_t tx_flags;
    dma_addr_t tx_buffer_dma;       /* dma物理地址 */
    struct rtl8139_status	tx_status;

    chip_t chipset;     /* 芯片集 */

    spinlock_t lock;        /* 普通锁 */
	spinlock_t rx_lock;      /* 接受锁 */

    uint32_t rx_config;      /* 接收配置 */

    device_queue_t rx_queue;    /* 接收队列 */

} device_extension_t;

struct rx_packet_header {
    /* 头信息 */
    uint16_t status;    /* 状态 */
    /*
    uint8_t pad2: 1;
    uint8_t pad3: 1;
    uint8_t pad4: 1;
    uint8_t pad5: 1;
    uint8_t pad6: 1;
    uint8_t BAR: 1;
    uint8_t PAM: 1;
    uint8_t MAR: 1;

    uint8_t ROK: 1;
    uint8_t FAE: 1;
    uint8_t CRC: 1;
    uint8_t LONG: 1;
    uint8_t RUNT: 1;
    uint8_t ISE: 1;
    uint8_t pad0: 1;
    uint8_t pad1: 1;*/
    /* 数据长度 */
    uint16_t length;    /* 数据长度 */
};

/* 接收配置 */
static const unsigned int rtl8139_rx_config =
	RX_CFG_RCV_32K | RX_NO_WRAP |
	(RX_FIFO_THRESH << RX_CFG_FIFO_SHIFT) |
	(RX_DMA_BURST << RX_CFG_DMA_SHIFT);

/* 传输配置 */
static const unsigned int rtl8139_tx_config =
	TX_IFG96 | (TX_DMA_BURST << TX_DMA_SHIFT) | (TX_RETRY << TX_RETRY_SHIFT);

/* 中断屏蔽配置 */
static const u16 rtl8139_intr_mask =
	PCI_ERR | PCS_TIMEOUT | RX_UNDERRUN | RX_OVERFLOW | RX_FIFO_OVER |
	TX_ERR | TX_OK | RX_ERR | RX_OK;
#if 0
/* 没有接收的中断屏蔽 */
static const u16 rtl8139_no_rx_intr_mask =
	PCI_ERR | PCS_TIMEOUT | RX_UNDERRUN |
	TX_ERR | TX_OK | RX_ERR ;
#endif

static int rtl8139_get_pci_info(device_extension_t *ext)
{
    /* get pci device */
    pci_device_t *device = pci_get_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (device == NULL) {
        printk(KERN_ERR "RTL8139 init failed: not find pci device.\n");
        return -1;
    }
	ext->pci_device = device;
#ifdef DEBUG_DRV    
	
    printk(KERN_DEBUG "find rtl8139 device, vendor id: 0x%x, device id: 0x%x\n",\
            device->vendor_id, device->device_id);
#endif
    /* enable bus mastering */
	pci_enable_bus_mastering(device);
    
    /* get io address */
    ext->io_addr = pci_device_get_io_addr(device);
    if (ext->io_addr == 0) {
        printk(KERN_ERR "RTL8139 init failed: INVALID pci device io address.\n");
        return -1;
    }
#ifdef DEBUG_DRV    
    printk(KERN_DEBUG "rlt8139 io address: 0x%x\n", ext->io_addr);
#endif
    /* get irq */
    ext->irq = pci_device_get_irq_line(device);
    if (ext->irq == 0xff) {
        printk(KERN_DEBUG "RTL8139 init failed: INVALID irq.\n");
        return -1;
    }
#ifdef DEBUG_DRV    	
    printk(KERN_DEBUG "rlt8139 irq: %d\n", ext->irq);
#endif
    return 0;
}

/**
 * rtl8139_next_desc - 获取下一个传输项
 * @current_desc: 当前传输项
 * 
 * 让传输项的取值处于0~3
 */
static int rtl8139_next_desc(int current_desc)
{
    return (current_desc == NUM_TX_DESC - 1) ? 0 : (current_desc + 1);
}

int rtl8139_transmit(device_extension_t *ext, uint8_t *buf, uint32 len)
{
    uint32_t entry;
    uint32_t length = len;

    /* 获取当前传输项 */
    entry = ext->current_tx;
    /* 改变传输项状态，开始数据传输 */
    //enum InterruptStatus flags = InterruptDisable();
    
    unsigned long flags;
    save_intr(flags);

    /* 如果还有剩余的传输项 */
    if (atomic_get(&ext->tx_free_counts) > 0) {
        //printk(KERN_DEBUG "!TX\n");
#ifdef DEBUG_DRV
        printk(KERN_DEBUG "Start TX, free %d\n", atomic_get(&ext->tx_free_counts));
#endif

#ifdef DEBUG_DRV
        
        printk("\n");
        printk(KERN_DEBUG "Transmit frame size %d, contents:\n", length);
        int i;
        for (i = 0; i < length; i++) {
            printk("%x ", buf[i]);
        }
        printk("\n");
#endif

        /* 如果长度是在传输范围内 */
        if (likely(length < TX_BUF_SIZE)) {
            /* 比最小数据帧还少 */
            if (length < ETH_ZLEN)
                memset(ext->tx_buffer[entry], 0, ETH_ZLEN);  /* 前面的部分置0 */

            /* 复制数据 */
            memcpy(ext->tx_buffer[entry], buf, length);

        } else {    /* 长度过长 */
            /* 丢掉数据包 */
            ext->stats.tx_dropped++; 
            printk(KERN_DEBUG "dropped a packed!\n");
            return 0;
        }

        /*
        * Writing to tx_status triggers a DMA transfer of the data
        * copied to ext->tx_buffer[entry] above. Use a memory barrier
        * to make sure that the device sees the updated data.
        * 上面复制了数据，为了让设备知道更新了数据，这里用一个写内存屏障
        */
        wmb();

        /* 传输至少60字节的数据 */
        out32(ext->io_addr + TX_STATUS0 + (entry * 4),
                ext->tx_flags | MAX(length, (uint32_t )ETH_ZLEN));
        in32(ext->io_addr + TX_STATUS0 + (entry * 4)); // flush

        /* 指向下一个传输描述符 */
        ext->current_tx = rtl8139_next_desc(ext->current_tx);

        /* 减少传输项数量 */
        atomic_dec(&ext->tx_free_counts);
#ifdef DEBUG_DRV
        printk(KERN_DEBUG "Start OK, free %d\n", atomic_get(&ext->tx_free_counts));
#endif        
        //printk(KERN_DEBUG "~TX\n");
    } else {
        // 停止传输 */
#ifdef DEBUG_DRV
        printk(KERN_DEBUG "Stop Tx packet!\n");
#endif
        //netif_stop_queue (dev);
        restore_intr(flags);
        return -1;
    }
    restore_intr(flags);

#ifdef DEBUG_DRV 
    printk(KERN_DEBUG "Queued Tx packet size %d to slot %d\n",
		    length, entry);
#endif
    return 0;
}

static int rtl8139_tx_interrupt(device_extension_t *ext)
{
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "TX\n");
#endif
    /* 空闲数量小于传输描述符数量才能进行处理 */
    while (atomic_get(&ext->tx_free_counts) < NUM_TX_DESC) {
#ifdef DEBUG_DRV        
        printk(KERN_DEBUG "TX, free %d\n", atomic_get(&ext->tx_free_counts));
#endif
        /* 获取第一个脏传输 */
        int entry = ext->dirty_tx;
        int tx_status;

        /* 读取传输状态 */
        tx_status = in32(ext->io_addr + TX_STATUS0 + (entry * 4));
        
        /* 如果传输状态不是下面的状态之一，就跳出，说明还被处理 */
        if (!(tx_status & (TX_STAT_OK | TX_UNDERRUN | TX_ABORTED))) {
            printk(KERN_DEBUG "tx status not we want!\n");
            break;
        }

        /* Note: TxCarrierLost is always asserted at 100mbps.
        如果是超出窗口，出错
         */
        if (tx_status & (TX_OUT_OF_WINDOW | TX_ABORTED)) {
            /* 有错误，记录下来 */
            printk(KERN_DEBUG "Transmit error, Tx status %x\n",
				    tx_status);    
            ext->stats.tx_errors++;
            if (tx_status & TX_ABORTED) {
                ext->stats.tx_aborted_errors++;
                /* 清除传输控制的abort位 */
                out32(ext->io_addr + TX_CONFIG, TX_CLEAR_ABT);

                /* 中断状态设置成传输错误 */
                out16(ext->io_addr + INTR_STATUS, TX_ERR);
                /* 写内存屏障 */
                wmb();
            }
            /* 携带丢失 */
            if (tx_status & TX_CARRIER_LOST)
                ext->stats.tx_carrier_errors++;
            
            /* 超过窗口大小 */
            if (tx_status & TX_OUT_OF_WINDOW)
                ext->stats.tx_window_errors++;
            
        } else {
            /* 传输正在运行中 */
            if (tx_status & TX_UNDERRUN) {
                /* Add 64 to the Tx FIFO threshold.
                传输阈值增加64字节
                 */
                if (ext->tx_flags < 0x00300000) {
                    ext->tx_flags += 0x00020000;
                }
                ext->stats.tx_fifo_errors++;

            }    
            /* 记录碰撞次数 */        
            ext->stats.collisions += (tx_status >> 24) & 15;

            /* 增加包数量 */
            ext->tx_status.packets++;
            /* 增加传输的字节数 */
            ext->tx_status.bytes += tx_status & 0x7ff;
        }

        /* 指向下一个传输描述符 */
        ext->dirty_tx = rtl8139_next_desc(ext->dirty_tx);

        /* 如果空闲数量是0，表明需要唤醒传输队列 */
        if (atomic_get(&ext->tx_free_counts) == 0) {
            /* 执行唤醒传输队列操作 */
            mb();
            // 唤醒队列 
		    //netif_wake_queue (dev);
#ifdef DEBUG_DRV
            printk(KERN_DEBUG "Wake up queue!\n");
#endif        
        }

        /* 空闲数量又增加 */
        atomic_inc(&ext->tx_free_counts);
#ifdef DEBUG_DRV
        printk(KERN_DEBUG "OK, free %d\n", atomic_get(&ext->tx_free_counts));
#endif
    }
#ifdef DEBUG_DRV    
    printk(KERN_DEBUG "TX, end free %d\n", atomic_get(&ext->tx_free_counts));
#endif
    return 0;
}

/**
 * rtl8139_other_interrupt - 非传输和接收中断处理
 * 
 */
static void rtl8139_other_interrupt(device_extension_t *ext,
        int status, int link_changed)
{
    ASSERT(ext != NULL);

#ifdef DEBUG_DRV
    printk(KERN_DEBUG "Abnormal interrupt, status %x\n", status);
#endif
    /* Update the error count.
    更新错过的包数
     */
	ext->stats.rx_missed_errors += in32(ext->io_addr + RX_MISSED);
	/* 归零，下次计算 */
    out32(ext->io_addr + RX_MISSED, 0);

	if ((status & RX_UNDERRUN) && link_changed &&
	    (ext->drv_flags & HAS_LNK_CHNG)) {
		
        /* 需要检测设备 */
        //rtl_check_media(dev, 0);
		status &= ~RX_UNDERRUN; /* 清除运行下状态位 */
	}

	if (status & (RX_UNDERRUN | RX_ERR))
		ext->stats.rx_errors++;

	if (status & PCS_TIMEOUT)
		ext->stats.rx_length_errors++;
	if (status & RX_UNDERRUN)
		ext->stats.rx_fifo_errors++;
	if (status & PCI_ERR) {
        /* pci错误 */
		u32 pci_cmd_status;
        pci_cmd_status = pci_device_read(ext->pci_device, PCI_STATUS_COMMAND);
        pci_device_write(ext->pci_device, PCI_STATUS_COMMAND, pci_cmd_status);
		/* 只需要高16位 */
        printk(KERN_ERR "PCI Bus error %x\n", pci_cmd_status >> 16);
	}
}

static void rtl8139_rx_error(u32 rx_status, device_extension_t *ext)
{
    u8 tmp8;
#ifdef DEBUG_DRV
    printk(KERN_NOTICE "Ethernet frame had errors, status %x\n", rx_status);
#endif
    ext->stats.rx_errors++;
    
    /* 如果没有接收成功 */
    if (!(rx_status & RX_STATUS_OK)) {
        /*if (rx_status & RX_TOO_LONG) {
            printk(KERN_DEBUG "Oversized Ethernet frame, status %x!\n", rx_status);
            
            // A.C.: The chip hangs here.
            Panic("rtl8139 card hangs!\n");
        }*/
        /* 帧错误 */
        if (rx_status & (RX_BAD_SYMBOL | RX_BAD_Align))
            ext->stats.rx_frame_errors++;
        
        /* 长度错误 */
        if (rx_status & (RX_RUNT | RX_TOO_LONG))
            ext->stats.rx_length_errors++;
        
        /* CRC错误 */
        if (rx_status & RX_CRC_ERR)
            ext->stats.rx_crc_errors++;
    } else {
        /* 接收成功，但会被丢掉 */
        ext->xstats.rx_lost_in_ring++;
    }

    /* ---接收重置---- */

    tmp8 = in8(ext->io_addr + CHIP_CMD);
    /* 清除接收位 */
    out8(ext->io_addr + CHIP_CMD, tmp8 & ~CMD_RX_ENABLE);
    /* 再写入接收位 */
    out8(ext->io_addr + CHIP_CMD, tmp8);
    /* 写入接收配置 */
    out32(ext->io_addr + RX_CONFIG, ext->rx_config);
    ext->current_rx = 0;

}

static void rtl8139_isr_ack(device_extension_t *ext)
{
    u16 status;

    status = in16(ext->io_addr + INTR_STATUS) & RX_ACK_BITS;

    /* Clear out errors and receive interrupts */
    if (likely(status != 0)) {
        /* 如果有错误，就记录错误信息 */
        if (unlikely(status & (RX_FIFO_OVER | RX_OVERFLOW))) {
            ext->stats.rx_errors++;
            if (status & RX_FIFO_OVER)
                ext->stats.rx_fifo_errors++;   
        }
        /* 写入接收回答位 */
        out16(ext->io_addr + INTR_STATUS, RX_ACK_BITS);
        in16(ext->io_addr + INTR_STATUS); // for flush

    }
}

static int rtl8139_rx_interrupt(device_extension_t *ext)
{
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "RX\n");
#endif
    int received = 0;
    unsigned char *rx_ring = ext->rx_ring;
    unsigned int current_rx = ext->current_rx;
    unsigned int rx_size = 0;
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "In %s(), current %x BufAddr %x, free to %x, Cmd %x\n",
		    __func__, (u16)current_rx,
		    in16(ext->io_addr + RX_BUF_ADDR),
            in16(ext->io_addr + RX_BUF_PTR),
            in8(ext->io_addr + CHIP_CMD));
#endif
    /* 当队列在运行中，并且接收缓冲区不是空 */
    while (!(in8(ext->io_addr + CHIP_CMD) & RX_BUFFER_EMPTY)) {
        /* 获取数据的偏移 */
        u32 ring_offset = current_rx % RX_BUF_LEN;
        u32 rx_status;

        unsigned int pkt_size;

        /* 要读取DMA内存 */
        rmb();
        
        /* read size+status of next frame from DMA ring buffer
        读取接收的状态
         */
        rx_status = le32_to_cpu(*(uint32_t *)(rx_ring + ring_offset));
        /* 接收的大小在高16位 */
        rx_size = rx_status >> 16;

#ifdef DEBUG_DRV
        printk(KERN_DEBUG "Rx packet status: ");
        if (rx_status & 1) printk(KERN_DEBUG "ROK ");
        if (rx_status & (1<<1)) printk(KERN_DEBUG "FAE ");
        if (rx_status & (1<<2)) printk(KERN_DEBUG "CRC ");
        if (rx_status & (1<<3)) printk(KERN_DEBUG "LONG ");
        if (rx_status & (1<<4)) printk(KERN_DEBUG "RUNT ");
        if (rx_status & (1<<5)) printk(KERN_DEBUG "ISE ");
        if (rx_status & (1<<13)) printk(KERN_DEBUG "BAR ");
        if (rx_status & (1<<14)) printk(KERN_DEBUG "PAM ");
        if (rx_status & (1<<15)) printk(KERN_DEBUG "MAR ");
        printk(KERN_DEBUG "\n");
#endif

        /* 如果没有不接收CRC特征，包的大小就是接收大小-4
        如果有，那么包的大小就是接收到的大小
         */
        if (likely(!(ext->dev_features & NET_FEATURE_RXFCS))) {
            //printk(KERN_DEBUG "\n<NO CRC>");
            pkt_size = rx_size - 4;
        } else {
            //printk(KERN_DEBUG "\n<CRC>");
            pkt_size = rx_size;
        }
        //printk(KERN_DEBUG "%x\n", rx_ring[ring_offset + pkt_size - 1]);
        
#ifdef DEBUG_DRV
        printk(KERN_DEBUG "%s() status %x, size %x, cur %x\n",
			  __func__, rx_status, rx_size, current_rx);
#endif
        
        /* Packet copy from FIFO still in progress.
		 * Theoretically, this should never happen
		 * since early_rx is disabled.
         * 如果还在FIFO传输，就发生了此中断，就在此处理。
         * 但是如果关闭了early_rx，那么这个就不会发生
		 */
        if (unlikely(rx_size == 0xfff0)) {
            /* fifo 复制超时处理 */

            /* 一般处理 */
            //printk(KERN_DEBUG "fifo copy in progress\n");
            ext->xstats.early_rx++;
            /* 跳出运行 */
            break;
        }
        /* If Rx err or invalid rx_size/rx_status received
		 * (which happens if we get lost in the ring),
		 * Rx process gets reset, so we abort any further
		 * Rx processing.
         * 接收大小过大或者过小，并且不是接收成功
		 */
        if (unlikely((rx_size > (MAX_ETH_FRAME_SIZE + 4)) ||
                (rx_size < 8) ||
                (!(rx_status & RX_STATUS_OK)))) {
            
            /* 如果接收所有包 */
            if ((ext->dev_features & NET_FEATURE_RXALL) &&
                (rx_size <= (MAX_ETH_FRAME_SIZE + 4)) &&
                (rx_size >= 8) &&
                (!(rx_status & RX_STATUS_OK))) {
                ext->stats.rx_errors++;
                if (rx_status & RX_CRC_ERR) {
                    ext->stats.rx_crc_errors++;
                    goto keep_pkt;
                }
                if (rx_status & RX_RUNT) {
                    ext->stats.rx_length_errors++;
                    goto keep_pkt;
                }
            }
            /* 接收的错误处理 */
            rtl8139_rx_error(rx_status,ext);
            received = -1;
            goto out;
        }
        
keep_pkt:

/* 如果要打印收到的数据，就可以在此打印 */
#ifdef DEBUG_DRV
        printk(KERN_DEBUG "Receive frame size %d, contents: \n", pkt_size);
        int i;
        for (i = 0; i < pkt_size; i++) {
            printk(KERN_DEBUG "%x ", rx_ring[4 + ring_offset + i]);
        }
        printk(KERN_DEBUG "\n");
#endif


        /* Malloc up new buffer, compatible with net-2e. */
		/* Omit the four octet CRC from the length. */
#ifdef DEBUG_DRV
        printk(KERN_DEBUG "RX: upload packet.\n");
#endif    
        /* 接受数据包 */
        //rtl8139_packet_receive(ext, &rx_ring[ring_offset + 4], pkt_size);
        io_device_queue_append(&ext->rx_queue, &rx_ring[ring_offset + 4], pkt_size);

        //NlltReceive(&rx_ring[ring_offset + 4], pkt_size);
        /* 创建接收缓冲区，并把数据复制进去 */
        //if (!NlltReceive(&rx_ring[ring_offset + 4], pkt_size)) {
            /* 更新状态 */
            ext->rx_status.packets++;
            ext->rx_status.bytes += pkt_size;
            
        /*} else {
            ext->stats.rx_dropped++;
        }*/
        received++;

        /* 接收指针指向下一个位置
        4:for header length(length include 4 bytes CRC)
        3:for dword alignment
         */
        current_rx = (current_rx + rx_size + 4 + 3) & ~3;
        out16(ext->io_addr + RX_BUF_PTR, (u16)(current_rx - 16));

        rtl8139_isr_ack(ext);

    }

    /* 如果没有接收到或者大小出错，上面的ACK就不会执行到，在这里再次调用 */
    if (unlikely(!received || rx_size == 0xfff0))
        rtl8139_isr_ack(ext);
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "Done %s(), current %04x BufAddr %04x, free to %04x, Cmd %02x\n",
		    __func__, current_rx,
		    in16(ext->io_addr + RX_BUF_ADDR),
            in16(ext->io_addr + RX_BUF_PTR),
            in8(ext->io_addr + CHIP_CMD));
#endif
    ext->current_rx = current_rx;
out:
    return received;
}

/**
 * KeyboardHandler - 时钟中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int rtl8139_handler(unsigned long irq, unsigned long data)
{
    //struct Task *cur = CurrentTask();
    //printk(KERN_DEBUG "in task %s.\n", cur->name);
	//printk(KERN_DEBUG "rtl8139_handler occur!\n");
    //disable_intr();
    device_extension_t *ext = (device_extension_t *) data;

    u16 status, ackstat;

    int link_changed = 0; /* avoid bogus "uninit" warning */

    spin_lock(&ext->lock);

    /* 读取中断状态 */
    status = in16(ext->io_addr + INTR_STATUS);
    //out16(ext->io_addr + INTR_STATUS, status);
    
    /* 如果一个状态位也没有，就退出 */
    if (unlikely((status & rtl8139_intr_mask) == 0)) {
        printk(KERN_DEBUG "[rtl8139]: no intr occur!\n");
        goto out;
    }
    //printk(KERN_DEBUG "[rtl8139]: int status:%x\n", status);

    /* 如果网络没有运行的时候发生中断，那么就退出 */
    /*if (unlikely(!netif_running(dev))) {
        // 中断屏蔽位置0，从此不接收任何中断 
        out16(ext->io_addr + INTR_MASK, 0);
        goto out;
    }*/

    /* Acknowledge all of the current interrupt sources ASAP, but
	   an first get an additional status bit from CSCR.
       中断状态是接收的运行下，就查看连接是否改变
     */
	if (unlikely(status & RX_UNDERRUN))
		link_changed = in16(ext->io_addr + CSCR) & CSCR_LINK_CHANGE;

    /* 读取状态中非ACK和ERR的状态 */
    ackstat = status & ~(RX_ACK_BITS | TX_ERR);
	if (ackstat)    /* 有则写入该状态 */
		out16(ext->io_addr + INTR_STATUS, ackstat);

	/* 
    处理接收中断
    */
    spin_lock(&ext->rx_lock);
	/* 如果有接收状态，那么就处理接收包 */
    if (status & RX_ACK_BITS){
        /* 调用接收处理函数 */
        rtl8139_rx_interrupt(ext);
	}
    spin_unlock(&ext->rx_lock);
    
	/* Check uncommon events with one test. */
	if (unlikely(status & (PCI_ERR | PCS_TIMEOUT | RX_UNDERRUN | RX_ERR)))
        rtl8139_other_interrupt(ext, status, link_changed);

    /* 处理接收 */
    if (status & (TX_OK | TX_ERR)) {
		rtl8139_tx_interrupt(ext);

        /* 如果状态有错误，那么就把错误写入状态寄存器 */
		if (status & TX_ERR)
			out16(ext->io_addr + INTR_STATUS, TX_ERR);
    }

out:
    spin_unlock(&ext->lock);
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "exiting interrupt, intr_status=%x\n",
		   in16(ext->io_addr + INTR_STATUS));
#endif    
    return 0;
}

static void rtl8139_chip_reset(device_extension_t *ext)
{
    /* software reset, to clear the RX and TX buffers and set everything back to defaults
        重置网卡
     */
    out8(ext->io_addr + CHIP_CMD, CMD_RESET);
    
    /* 循环检测芯片是否完成重置 */
    while (1) {
		barrier();
        /* 如果重置完成，该位会被置0 */
		if ((in8(ext->io_addr + CHIP_CMD) & CMD_RESET) == 0)
			break;
        cpu_lazy();
	}

}

static int rtl8139_init_board(device_extension_t *ext)
{

    /* check for missing/broken hardware
    检查丢失/损坏的硬件
     */
	if (in32(ext->io_addr + TX_CONFIG) == 0xFFFFFFFF) {
		printk(KERN_DEBUG "Chip not responding, ignoring board\n");
		return -1;
	}

    uint32_t version = in32(ext->io_addr + TX_CONFIG) & HW_REVID_MASK;
    int i;
    for (i = 0; i < CHIP_INFO_NR; i++)
		if (version == rtl_chip_info[i].version) {
			ext->chipset = i;
			goto chip_match;
		}
    
    /* if unknown chip, assume array element #0, original RTL-8139 in this case */
	i = 0;
#ifdef DEBUG_DRV	
    printk(KERN_DEBUG "unknown chip version, assuming RTL-8139\n");
	printk(KERN_DEBUG "TxConfig = 0x%x\n", in32(ext->io_addr + TX_CONFIG));
#endif 
	ext->chipset = 0;

chip_match:
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "chipset id (%x) == index %d, '%s'\n",
            version, i, rtl_chip_info[i].name);
#endif

    /* 给网卡加电，启动它 */
	if (ext->chipset >= CH_8139B) {
		printk(KERN_DEBUG "PCI PM wakeup, not support now!\n");
	} else {
		//printk(KERN_DEBUG "Old chip wakeup\n");
		uint8_t tmp8 = in8(ext->io_addr + CONFIG1);
		tmp8 &= ~(SLEEP | PWRDN);
        /* 启动网卡 */
        out8(ext->io_addr + CONFIG1, tmp8);
	}

    /* 重置芯片 */
    rtl8139_chip_reset(ext);

    return 0;
}

/* Initialize the Rx and Tx rings, along with various 'dev' bits. */
static void rtl8139_init_ring(device_extension_t *ext)
{
	int i;

	ext->current_rx = 0;
	ext->current_tx = 0;
	ext->dirty_tx = 0;

    /* 空闲的传输项数量 */
    atomic_set(&ext->tx_free_counts, NUM_TX_DESC);

	for (i = 0; i < NUM_TX_DESC; i++)
		ext->tx_buffer[i] = (unsigned char *)&ext->tx_buffers[i * TX_BUF_SIZE];
}

static void __set_rx_mode(device_extension_t *ext)
{
#ifdef DEBUG_DRV    
	printk(KERN_DEBUG "rtl8139_set_rx_mode(%x) done -- Rx config %x\n",
		    ext->flags,
            in32(ext->io_addr + RX_CONFIG));
#endif 
    /* 一般内容，接收多播，广播，发给自己的物理包 */
    int rx_mode = ACCEPT_BROADCAST | ACCEPT_MY_PHYS | ACCEPT_MULTICAST;
    
    /* 其它所有内容 */
    if (ext->dev_features & NET_FEATURE_RXALL) {
        rx_mode |= (ACCEPT_ERR | ACCEPT_RUNT);
    }
    
    uint32_t tmp;
    tmp = rtl8139_rx_config | rx_mode;
    if (ext->rx_config != tmp) {
        /* 写入寄存器 */
        out32(ext->io_addr + RX_CONFIG, tmp);
        /* 刷新流水线 */
        in32(ext->io_addr + RX_CONFIG);
        
        ext->rx_config = tmp;
    }

    /* 过滤处理 */
    u32 mac_filter[2];

    mac_filter[0] = mac_filter[1] = 0;

    /* 写入过滤到寄存器 */
    out32(ext->io_addr + MAR0 + 0, mac_filter[0]);
    in32(ext->io_addr + MAR0 + 0);
    
    out32(ext->io_addr + MAR0 + 4, mac_filter[1]);
    in32(ext->io_addr + MAR0 + 4);
    
}

static void rtl8139_set_rx_mode(device_extension_t *ext)
{
	unsigned long flags;
    spin_lock_irqsave(&ext->lock, flags);

	__set_rx_mode(ext);

    spin_unlock_irqrestore(&ext->lock, flags);
}

static void rtl8139_hardware_start(device_extension_t *ext)
{

    /* Bring old chips out of low-power mode.
    把老芯片带出低耗模式
     */
	if (rtl_chip_info[ext->chipset].flags & HAS_HLT_CLK)
		out8(ext->io_addr + HLT_CTL, 'R');

    /* 重置芯片 */
    rtl8139_chip_reset(ext);

    /* unlock Config[01234] and BMCR register writes
    解锁配置寄存器
     */
    out8(ext->io_addr + CFG9346, CFG9346_UNLOCK);
    in8(ext->io_addr + CFG9346);   // flush

    /* Restore our rtl8139a of the MAC address.
    重载mac地址
     */
    out32(ext->io_addr + MAC0, 
            le32_to_cpu(*(uint32_t *)(ext->mac_addr + 0)));
    in32(ext->io_addr + MAC0);
    
    out16(ext->io_addr + MAC0 + 4, 
            le32_to_cpu(*(uint16_t *)(ext->mac_addr + 4)));
    in16(ext->io_addr + MAC0 + 4);
    
    ext->current_rx = 0;

    /* init Rx ring buffer DMA address
    写接收缓冲区的dma地址到寄存器 */
	out32(ext->io_addr + RX_BUF, ext->rx_ring_dma);
    in32(ext->io_addr + RX_BUF);

    /* Must enable Tx/Rx before setting transfer thresholds!
    必须在设置传输阈值前打开TX/RX功能
     */
	out8(ext->io_addr + CHIP_CMD, CMD_RX_ENABLE | CMD_TX_ENABLE);
    
    /* 设定接收配置 */
    ext->rx_config = rtl8139_rx_config | ACCEPT_BROADCAST | ACCEPT_MY_PHYS;
	
    /* 把传输配置和接收配置都写入寄存器 */
    out32(ext->io_addr + RX_CONFIG, ext->rx_config);
    out32(ext->io_addr + TX_CONFIG, rtl8139_tx_config);

    if (ext->chipset >= CH_8139B) {
		/* Disable magic packet scanning, which is enabled
		 * when PM is enabled in Config1.  It can be reenabled
		 * via ETHTOOL_SWOL if desired.
         * 清除MAGIC位
         */
		out8(ext->io_addr + CONFIG3,
            in8(ext->io_addr + CONFIG3) & ~CFG3_MAGIC);
	}

    //printk(KERN_DEBUG "init buffer addresses\n");

    /* Lock Config[01234] and BMCR register writes
    上锁，不可操作这些寄存器
     */
    out8(ext->io_addr + CFG9346, CFG9346_LOCK);

	/* init Tx buffer DMA addresses
    写入传输缓冲区的DMA地址到寄存器
     */
    int i;
	for (i = 0; i < NUM_TX_DESC; i++) {
        out32(ext->io_addr + TX_ADDR0 + (i * 4),
            ext->tx_buffer_dma + (ext->tx_buffer[i] - ext->tx_buffers));
        /* flush */
        in32(ext->io_addr + TX_ADDR0 + (i * 4));
    }

    /* 接收missed归零 */
    out32(ext->io_addr + RX_MISSED, 0);

    /* 设置接收模式 */
    rtl8139_set_rx_mode(ext);

    /* 没有early-rx中断 */
    out16(ext->io_addr + MULTI_INTR,
            in16(ext->io_addr + MULTI_INTR) & MULTI_INTR_CLEAR);
    
    /* 确保传输和接收都已经打开了 */
    uint8_t tmp = in8(ext->io_addr + CHIP_CMD);
    if (!(tmp & CMD_RX_ENABLE) || !(tmp & CMD_TX_ENABLE))
        out8(ext->io_addr + CHIP_CMD,
                CMD_RX_ENABLE | CMD_TX_ENABLE);
    
    /* 打开已知配置好的中断 */
    out16(ext->io_addr + INTR_MASK, rtl8139_intr_mask);

}

static inline void rtl8139_tx_clear(device_extension_t *ext)
{
	ext->current_tx = 0;
	ext->dirty_tx = 0;

    atomic_set(&ext->tx_free_counts, NUM_TX_DESC);

	/* XXX account for unsent Tx packets in tp->stats.tx_dropped */
}


static int rtl8139_init(device_extension_t *ext)
{
    static int board_idx = -1;

    ASSERT(ext);

    board_idx++;

    pci_device_t *pdev = ext->pci_device;
    
    /* 对版本进行检测 */

    /* 如果是增强的版本，输出提示信息 */
    if (pdev->vendor_id  == RTL8139_VENDOR_ID &&
	    pdev->device_id == RTL8139_DEVICE_ID && pdev->revision_id >= 0x20) {
#ifdef DEBUG_DRV    
		printk(KERN_DEBUG "This (id %04x:%04x rev %02x) is an enhanced 8139C+ chip, use 8139cp\n",
		        pdev->vendor_id, pdev->device_id, pdev->revision_id);
#endif		
        /* 本来在这里是该返回的，return -1;
        相当于不支持，不过貌似不用增强版本代码，也可以 */
	}

    /* 初始化版本设定 */
    if (rtl8139_init_board(ext)) {
        return -1;  /* 初始化失败 */
    }

    /* 从配置空间中读取MAC地址 */
	int i;
    for (i = 0; i < ETH_ALEN; i++) {
        ext->mac_addr[i] = in8(ext->io_addr + MAC0 + i);
    }
#ifdef DEBUG_DRV    
    printk(KERN_DEBUG "mac addr: %x:%x:%x:%x:%x:%x\n", ext->mac_addr[0], ext->mac_addr[1],
            ext->mac_addr[2], ext->mac_addr[3], 
            ext->mac_addr[4], ext->mac_addr[5]);
#endif
    printk(KERN_INFO "netcard rtl8139: mac addr: %x:%x:%x:%x:%x:%x\n", ext->mac_addr[0], ext->mac_addr[1],
            ext->mac_addr[2], ext->mac_addr[3], 
            ext->mac_addr[4], ext->mac_addr[5]);
    
    /* 设置硬件特征为接收所有包，接收包不带CRC */
    ext->dev_features = NET_FEATURE_RXALL;

    ext->drv_flags = 0;

    /* 初始化自旋锁 */
    spinlock_init(&ext->lock);
    spinlock_init(&ext->rx_lock);

    /* Put the chip into low-power mode.
    把芯片设置成低耗模式
     */
	if (rtl_chip_info[ext->chipset].flags & HAS_HLT_CLK)
		out8(ext->io_addr + HLT_CTL, 'H');	/* 'R' would leave the clock running. */

    return 0;
}

static iostatus_t rtl8139_open(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *ext = device->device_extension;
    
     /* 分配传输缓冲区 */
    ext->tx_buffers = (unsigned char *) kmalloc(TX_BUF_TOT_LEN);
    if (ext->tx_buffers == NULL) {
        printk(KERN_DEBUG "kmalloc for rtl8139 tx buffer failed!\n");
        
        return -1;
    }

    /* 分配接收缓冲区 */
    ext->rx_ring = (unsigned char *) kmalloc(RX_BUF_TOT_LEN);
    if (ext->rx_ring == NULL) {
        printk(KERN_DEBUG "kmalloc for rtl8139 rx buffer failed!\n");
        kfree(ext->tx_buffers);
        return -1;
    }

    /* 把虚拟地址转换成物理地址，即DMA地址 */
    ext->tx_buffer_dma = v2p(ext->tx_buffers);
    ext->rx_ring_dma = v2p(ext->rx_ring);

    /* 传输标志设置 */
    ext->tx_flags = (TX_FIFO_THRESH << 11) & 0x003f0000;

    /* 初始化传输和接收缓冲区环 */
    rtl8139_init_ring(ext);
    /* 设置硬件信息 */
    rtl8139_hardware_start(ext);

    register_irq(ext->irq, rtl8139_handler, IRQF_SHARED, "IRQ-Network", DEV_NAME, (unsigned int)ext);
    
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

static iostatus_t rtl8139_close(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *ext = device->device_extension;

    /* 先停止队列传输 */
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "Shutting down ethercard, status was %x\n",
		    in16(ext->io_addr + INTR_STATUS));
#endif    
    /* 需要关闭中断 */
    unsigned long flags;
    spin_lock_irqsave(&ext->lock, flags);

    /* 先停止命令的传输和接收 */
    out8(ext->io_addr + CHIP_CMD, 0);

    /* 屏蔽网卡可以触发的所有中断 */
    out16(ext->io_addr + INTR_MASK, 0);

    /* 更新错误计数，丢失的数量，并把寄存器中丢失归零 */
    ext->stats.rx_missed_errors += in32(ext->io_addr + RX_MISSED);
    out32(ext->io_addr + RX_MISSED, 0);

    /* 注销中断 */
    unregister_irq(ext->irq, ext);
    spin_unlock_irqrestore(&ext->lock, flags);

    /* 清除传输的项 */
    rtl8139_tx_clear(ext);

    /* 释放缓冲区 */
    kfree(ext->rx_ring);
    kfree(ext->tx_buffers);

    ext->rx_ring = NULL;
    ext->tx_buffers = NULL;
    
    out8(ext->io_addr + CFG9346, CFG9346_UNLOCK); // 先解锁
    /* 把芯片设置回低耗模式 */
    if (rtl_chip_info[ext->chipset].flags & HAS_HLT_CLK) 
        out8(ext->io_addr + HLT_CTL, 'H'); 
    
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

static iostatus_t rtl8139_read(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len;
    device_extension_t *ext = device->device_extension;
    iostatus_t status = IO_SUCCESS;

    uint8_t *buf = (uint8_t *)ioreq->user_buffer; 
    int flags = 0;

#ifdef DEBUG_DRV    
    printk(KERN_DEBUG "rtl8139_write: receive data=%x len=%d flags=%x\n",
        buf, ioreq->parame.read.length, ioreq->parame.read.offset);
#endif
    len = ioreq->parame.read.length;

    if (ioreq->parame.read.offset & DEV_NOWAIT) {
        flags |= IO_NOWAIT;
    }
    /* 从网络接收队列中获取一个包 */
    len = io_device_queue_pickup(&ext->rx_queue, buf, len, flags);
    if (len < 0)
        status = IO_FAILED;

#ifdef DEBUG_DRV    
    if (len > 0) {
        buf = (uint8_t *)ioreq->user_buffer; 
        dump_buffer(buf, 32, 1);
    }
#endif
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = len;
    
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return status;
}

static iostatus_t rtl8139_write(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len = ioreq->parame.write.length;
    
    uint8_t *buf = (uint8_t *)ioreq->user_buffer; 
#ifdef DEBUG_DRV    
    printk(KERN_DEBUG "rtl8139_write: transmit data=%x len=%d flags=%x\n",
        buf, ioreq->parame.write.length, ioreq->parame.write.offset);
#endif
  
    if (rtl8139_transmit(device->device_extension, buf, len))
        len = -1;

    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

static iostatus_t rtl8139_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;
    unsigned long arg = ioreq->parame.devctl.arg;
    device_extension_t *extension = (device_extension_t *) device->device_extension;
    iostatus_t status;
    unsigned char *mac;

    switch (ctlcode)
    {
    case NETIO_GETMAC:
        mac = (unsigned char *) arg;
        *mac++ = extension->mac_addr[0];
        *mac++ = extension->mac_addr[1];
        *mac++ = extension->mac_addr[2];
        *mac++ = extension->mac_addr[3];
        *mac++ = extension->mac_addr[4];
        *mac++ = extension->mac_addr[5];
#ifdef DEBUG_DRV
        printk(KERN_DEBUG "rtl8139_devctl: copy mac addr to addr %x\n", ioreq->parame.devctl.arg);
#endif
        status = IO_SUCCESS;
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

static iostatus_t rtl8139_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *devext;

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_PHYSIC_NETCARD, &devobj);

    if (status != IO_SUCCESS) {
        printk(KERN_DEBUG KERN_ERR "rtl8139_enter: create device failed!\n");
        return status;
    }
    /* neither io mode */
    devobj->flags = 0;
    
    devext = (device_extension_t *)devobj->device_extension;
    devext->device_object = devobj;
    /* 初始化接收队列，用内核队列结构保存，等待被读取 */
    io_device_queue_init(&devext->rx_queue);

    if (rtl8139_get_pci_info(devext)) {
        status = IO_FAILED;
        io_delete_device(devobj);
        return status;
    }

    if (rtl8139_init(devext)) {
        status = IO_FAILED;
        io_delete_device(devobj);
        return status;
    }

    return status;
}

static iostatus_t rtl8139_exit(driver_object_t *driver)
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

iostatus_t rtl8139_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = rtl8139_enter;
    driver->driver_exit = rtl8139_exit;

    driver->dispatch_function[IOREQ_OPEN] = rtl8139_open;
    driver->dispatch_function[IOREQ_CLOSE] = rtl8139_close;
    driver->dispatch_function[IOREQ_READ] = rtl8139_read;
    driver->dispatch_function[IOREQ_WRITE] = rtl8139_write;
    driver->dispatch_function[IOREQ_DEVCTL] = rtl8139_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "rtl8139_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    return status;
}

static __init void rtl8139_driver_entry(void)
{
    if (driver_object_create(rtl8139_driver_func) < 0) {
        printk(KERN_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(rtl8139_driver_entry);
