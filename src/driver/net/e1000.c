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
#include <sys/res.h>

#include <net/e1000_hw.h>
#include <net/e1000_osdep.h>
#include <net/e1000.h>

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
typedef struct _e1000_extension {
    device_object_t *device_object;   //设备对象

    timer_t tx_fifo_stall_timer;
    timer_t watchdog_timer;
    timer_t phy_info_timer;
    
    /* TX */
	struct e1000_desc_ring tx_ring;
	spinlock_t tx_lock;
	uint32_t txd_cmd;
	uint32_t tx_int_delay;
	uint32_t tx_abs_int_delay;
	uint32_t gotcl;
	uint64_t gotcl_old;
	uint64_t tpt_old;
	uint64_t colc_old;
	uint32_t tx_fifo_head;
	uint32_t tx_head_addr;
	uint32_t tx_fifo_size;
	atomic_t tx_fifo_stall;
	boolean_t pcix_82544;

    /* RX */
	struct e1000_desc_ring rx_ring;
	uint64_t hw_csum_err;
	uint64_t hw_csum_good;
	uint32_t rx_int_delay;
	uint32_t rx_abs_int_delay;
	boolean_t rx_csum;
	uint32_t gorcl;
	uint64_t gorcl_old;

    /* structs defined in e1000_hw.h */
    struct e1000_hw hw;
    struct e1000_hw_stats stats;
    struct e1000_phy_info phy_info;
    struct e1000_phy_stats phy_stats;

    uint32_t part_num;

    uint32_t io_addr;   //IO基地址
    int drv_flags;   //驱动标志
    uint32_t irq;
    flags_t flags;

    pci_device_t* pci_device;

    uint32_t dev_features;   //设备结构特征

    uint32_t rx_buffer_len;
    uint32_t max_frame_size;
    uint32_t min_frame_size;
    
    spinlock_t stats_lock;
    atomic_t irq_sem;

    // uint8_t *rx_buffer;   //接收缓冲区
    // uint8_t *rx_ring;   //接收环
    // uint8_t current_rx;
    // flags_t rx_flags;
    // dma_addr_t rx_ring_dma;   //dma物理地址

    spinlock_t lock;   //普通锁
    spinlock_t rx_lock;   //接收锁

    device_queue_t rx_queue;   //接收队列

}e1000_extension_t;

/**
 * 1. 申请pci结构(pci_device_t)并初始化厂商号和设备号
 * 2. 启动总线控制
 * 3. 申请设备io空间，在pci_device中登记io空间基地址
 * 4. 申请中断，并在pci_device中登记中断号
**/
static int e1000_get_pci_info(e1000_extension_t* ext)
{
    /* get pci device */
    pci_device_t* pci_device = pci_get_device(PCI_VENDOR_ID_INTEL, E1000_DEV_ID_82540EM);
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
    ext->hw.io_base = pci_device_get_io_addr(pci_device);
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

/* e1000_82547_tx_stall - timer call_back */
static void e1000_82547_tx_fifo_stall(unsigned long data)
{
    e1000_extension_t* ext = (e1000_extension_t*)data;
    uint32_t tctl;

    if(atomic_get(&ext->tx_fifo_stall)) {
		if((E1000_READ_REG(&ext->hw, TDT) ==
		    E1000_READ_REG(&ext->hw, TDH)) &&
		   (E1000_READ_REG(&ext->hw, TDFT) ==
		    E1000_READ_REG(&ext->hw, TDFH)) &&
		   (E1000_READ_REG(&ext->hw, TDFTS) ==
		    E1000_READ_REG(&ext->hw, TDFHS))) {
			tctl = E1000_READ_REG(&ext->hw, TCTL);
			E1000_WRITE_REG(&ext->hw, TCTL,
					tctl & ~E1000_TCTL_EN);
			E1000_WRITE_REG(&ext->hw, TDFT,
					ext->tx_head_addr);
			E1000_WRITE_REG(&ext->hw, TDFH,
					ext->tx_head_addr);
			E1000_WRITE_REG(&ext->hw, TDFTS,
					ext->tx_head_addr);
			E1000_WRITE_REG(&ext->hw, TDFHS,
					ext->tx_head_addr);
			E1000_WRITE_REG(&ext->hw, TCTL, tctl);
			E1000_WRITE_FLUSH(&ext->hw);

			ext->tx_fifo_head = 0;
			atomic_set(&ext->tx_fifo_stall, 0);
			//netif_wake_queue(netdev);
		} else {
			timer_mod(&ext->tx_fifo_stall_timer, systicks + 1);
		}
	}
}

static int e1000_sw_init(e1000_extension_t* ext)
{
    struct e1000_hw* hw = &ext->hw;
    pci_device_t* pdev = ext->pci_device;

    /* pci config space info */
    hw->vendor_id = pdev->vendor_id;
    hw->device_id = pdev->device_id;
    hw->subsystem_vendor_id = pdev->subsystem_vendor_id;
    hw->subsystem_id = pdev->subsystem_device_id;
    hw->revision_id = pdev->revision_id;
    hw->pci_cmd_word = pdev->command;

    ext->rx_buffer_len = E1000_RXBUFFER_2048;
    //error
    hw->max_frame_size = ENET_HEADER_SIZE + ETHERNET_FCS_SIZE;
    hw->min_frame_size = MINIMUM_ETHERNET_FRAME_SIZE;

    /* identify the MAC，根据pci_device_id确定mac类型 */
    if(e1000_set_mac_type(hw)) {
        printk(KERN_DEBUG "Unknown MAC type\n");
        return -1;
    }

    /* initialize eeprom parameters，根据mac类型确定eeprom信息*/
    e1000_init_eeprom_params(hw);

    switch(hw->mac_type) {
        default:
            break;
        case e1000_82541:
        case e1000_82547:
        case e1000_82541_rev_2:
        case e1000_82547_rev_2:
            hw->phy_init_script = 1;
            break;
    }

    /* 根据mac_type确定media_type*/
    e1000_set_media_type(hw);

    hw->wait_autoneg_complete = FALSE;
    hw->tbi_compatibility_en = TRUE;
    hw->adaptive_ifs = TRUE;

    /* copper options 根据media_type确定mdix,disable_polarity_correction,master_slave*/
    if(hw->media_type == e1000_media_type_copper) {
        hw->mdix = AUTO_ALL_MODES;
        hw->disable_polarity_correction = FALSE;
        hw->master_slave = E1000_MASTER_SLAVE;
    }

    atomic_set(&ext->irq_sem, 1);
    spinlock_init(&ext->stats_lock);
    spinlock_init(&ext->tx_lock);

    return 0;
}

static int e1000_init_board(e1000_extension_t* ext)
{
    /* setup the private structrue */
    if(e1000_sw_init(ext)) {
        return -1;
    }

    //初始化网卡信息、LED、media、recv_reg、hash_table、link、statistics_reg
    //if(e1000_init_hw(&ext->hw) != E1000_SUCCESS) {
    //    printk(KERN_DEBUG "e1000_init_hw failed\n");
    //    return -1;
    //}

    /* 网卡复位 */
    if(e1000_reset_hw(&ext->hw) != E1000_SUCCESS) {
        printk(KERN_DEBUG "e1000_reset_hw failed\n");
        return -1;
    }

    /* 确保eeprom良好 */
    if(e1000_validate_eeprom_checksum(&ext->hw) < 0) {
        printk(KERN_DEBUG "the eeprom checksum is not valid\n");
        return -1;
    }

    return 0;
}

int e1000_reset(e1000_extension_t* ext)
{
    uint32_t pba;

	/* Repartition Pba for greater than 9k mtu
	 * To take effect CTRL.RST is required.
	 */
    if(ext->hw.mac_type < e1000_82547) {
        if(ext->rx_buffer_len > E1000_RXBUFFER_8192) {
            pba = E1000_PBA_40K;
        } else {
            pba = E1000_PBA_48K;
        }
    } else {
        if(ext->rx_buffer_len > E1000_RXBUFFER_8192) {
            pba = E1000_PBA_22K;
        } else {
            pba = E1000_PBA_30K;
        }
        ext->tx_fifo_head = 0;
        ext->tx_head_addr = pba << E1000_TX_HEAD_ADDR_SHIFT;
        ext->tx_fifo_size = 
            (E1000_PBA_40K - pba) << E1000_PBA_BYTES_SHIFT;
        atomic_set(&ext->tx_fifo_stall, 0);
    }
    E1000_WRITE_REG(&ext->hw, PBA, pba);

    /* flow control settings */
    ext->hw.fc_high_water = (pba << E1000_PBA_BYTES_SHIFT) - E1000_FC_HIGH_DIFF;
    ext->hw.fc_low_water = (pba << E1000_PBA_BYTES_SHIFT) - E1000_FC_LOW_DIFF;
    ext->hw.fc_pause_time = E1000_FC_PAUSE_TIME;
    ext->hw.fc = ext->hw.original_fc;

    e1000_reset_hw(&ext->hw);
    if(ext->hw.mac_type >= e1000_82544) {
        E1000_WRITE_REG(&ext->hw, WUC, 0);
    }
    if(e1000_init_hw(&ext->hw)) {
        printk(KERN_DEBUG "Hardware error\n");
    }

    /* Enable h/w to recognize an 802.1Q VLAN Ethernet packet */
    E1000_WRITE_REG(&ext->hw, VET, ETHERNET_IEEE_VLAN_TYPE);

    e1000_reset_adaptive(&ext->hw);
    e1000_phy_get_info(&ext->hw, &ext->phy_info);

    return 0;
}

static int e1000_init(e1000_extension_t* ext)
{
    device_object_t* net_dev;
    uint16_t eeprom_data;

    ASSERT(ext);

    pci_device_t* pdev = ext->pci_device;


    /* 对版本进行检测，仅支持82540em网卡 */
    if(pdev->vendor_id != E1000_VENDOR_ID && pdev->device_id != E1000_DEV_ID_82540EM) {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "this deiver only support 82540em netcard.\n");
#endif
        return -1;
    }

    /* 初始化版本设定 */
    if(e1000_init_board(ext)) {
        return -1;
    }

    /* 复制网卡中的mac地址*/
    if(e1000_read_mac_addr(&ext->hw)) {
        printk(KERN_DEBUG "EEPEOM read error\n");
        return -1;
    }
    //打印mac地址
    printk(KERN_DEBUG "e1000_mac_addr:");
    for(int i=0; i<5; i++) {
        printk(KERN_DEBUG "%d-", ext->hw.mac_addr[i]);
    }
    printk(KERN_DEBUG "%d\n", ext->hw.mac_addr[5]);

    e1000_read_part_num(&ext->hw, &(ext->part_num));

    /* 获取总线类型和速度 */
    e1000_get_bus_info(&ext->hw);

    //定时器设置

    //通知系统网卡不可用

    //参数检查

    /* Initial Wake on LAN setting
	 * If APM wake is enabled in the EEPROM,
	 * enable the ACPI Magic Packet filter
	 */
	switch(ext->hw.mac_type) {
	case e1000_82542_rev2_0:
	case e1000_82542_rev2_1:
	case e1000_82543:
		break;
	case e1000_82546:
	case e1000_82546_rev_3:
		if((E1000_READ_REG(&ext->hw, STATUS) & E1000_STATUS_FUNC_1)
		   && (ext->hw.media_type == e1000_media_type_copper)) {
			e1000_read_eeprom(&ext->hw,
				EEPROM_INIT_CONTROL3_PORT_B, 1, &eeprom_data);
			break;
		}
		/* Fall Through */
	default:
		e1000_read_eeprom(&ext->hw,
			EEPROM_INIT_CONTROL3_PORT_A, 1, &eeprom_data);
		break;
	}

    /* reset the hardware with the new setting */
    e1000_reset(ext);

    strcpy(net_dev->name.text, "eth%d");

    //设备加入设备链表

    printk(KERN_DEBUG "Intel(R) PRO/1000 Network Connection\n");

    return 0;
}

static iostatus_t e1000_enter(driver_object_t* driver)
{
    iostatus_t status;

    device_object_t* devobj;
    e1000_extension_t* devext;
    
    /* 初始化一些其他内容-初始化devobj(未扩展的内容) */
    status = io_create_device(driver, sizeof(e1000_extension_t), DEV_NAME, DEVICE_TYPE_PHYSIC_NETCARD, &devobj);

    if(status != IO_SUCCESS) {
        printk(KERN_DEBUG KERN_ERR "e1000_enter: create device failed!\n");
        return status;
    }

    /* neither io mode*/
    devobj->flags = 0;

    devext = (e1000_extension_t*)devobj->device_extension;   //设备扩展部分位于devobj的尾部
    devext->device_object = devobj;
    devext->hw.back = devext;

    /*初始化接收队列，用内核队列结构保存，等待被读取*/
    io_device_queue_init(&devext->rx_queue);

    /* 申请并初始化pci_device_t，io_addr、中断号*/
    if(e1000_get_pci_info(devext)) {
        status = IO_FAILED;
        io_delete_device(devobj);
        return status;
    }

    if(e1000_init(devext)) {
        status = IO_FAILED;
        io_delete_device(devobj);
        return status;
    }

    return status;
}

iostatus_t e1000_driver_vine(driver_object_t* driver)
{
    iostatus_t status = IO_SUCCESS;

    /* 绑定驱动信息 */
    driver->driver_enter = e1000_enter;

    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
    printk(KERN_DEBUG "e1000_driver_vim: driver name=%s\n", driver->name.text);
    
    return status;
}

void e1000_pci_clear_mwi(struct e1000_hw* hw)
{
	e1000_extension_t* ext = hw->back;
    uint32_t value;
    uint16_t cmd;

    cmd = ext->hw.pci_cmd_word & (~(PCI_COMMAND_INVALIDATE));
    value = pci_device_read(ext->pci_device, PCI_STATUS_COMMAND);
    value &= cmd;
    
	pci_device_write(ext->pci_device, PCI_STATUS_COMMAND, value);
}

void e1000_pci_set_mwi(struct e1000_hw* hw)
{
    e1000_extension_t* ext = hw->back;
    uint32_t value;
    uint16_t cmd;

    cmd = ext->hw.pci_cmd_word | PCI_COMMAND_INVALIDATE;
    value = pci_device_read(ext->pci_device, PCI_STATUS_COMMAND);
    value &= cmd;

    pci_device_write(ext->pci_device, PCI_STATUS_COMMAND, value);
}