#ifndef _ARCH_PCI_H
#define _ARCH_PCI_H

/*
【简介】
	PCI是什么？PCI是Peripheral Component Interconnect(外设部件互连标准)的缩写，
它是目前个人电脑中使用最为广泛的接口，几乎所有的主板产品上都带有这种插槽。

	PCI有三个相互独立的物理地址空间：设备存储器地址空间、I/O地址空间和配置空间。
配置空间是PCI所特有的一个物理空间。由于PCI支持设备即插即用，所以PCI设备不占
用固定的内存地址空间或I/O地址空间，而是由操作系统决定其映射的基址。

	PCI总线规范定义的配置空间总长度为256个字节，配置信息按一定的顺序和大小依次存放。
前64个字节的配置空间称为配置头，对于所有的设备都一样，配置头的主要功能是用来识别设备、
定义主机访问PCI卡的方式（I/O访问或者存储器访问，还有中断信息）。
其余的192个字节称为本地配置空间（设备有关区），主要定义卡上局部总线的特性、本地空间基地址及范围等。

【配置空间中的重要信息】

	Vendor ID：厂商ID。知名的设备厂商的ID。FFFFh是一个非法厂商ID，可它来判断PCI设备是否存在。

	Device ID：设备ID。某厂商生产的设备的ID。操作系统就是凭着 Vendor ID和Device ID 找到对应驱动程序的。

	Class Code：类代码。共三字节，分别是 类代码、子类代码、编程接口。类代码不仅用于区分设备类型，还是编程接口的规范，这就是为什么会有通用驱动程序。

	IRQ Line：IRQ编号。PC机以前是靠两片8259芯片来管理16个硬件中断。
	现在为了支持对称多处理器，有了APIC（高级可编程中断控制器），它支持管理24个中断。

	IRQ Pin：中断引脚。PCI有4个中断引脚，该寄存器表明该设备连接的是哪个引脚。


【访问配置空间】	
	如何访问配置空间呢？可通过访问0xCF8h、0xCFCh端口来实现。
	0xCF8h: CONFIG_ADDRESS。PCI配置空间地址端口。
	0xCFCh: CONFIG_DATA。PCI配置空间数据端口。
	
【CONFIG_ADDRESS寄存器格式】
	31　位： Enabled位。
	23:16 位： 总线编号。
	15:11 位： 设备编号。
	10: 8 位：功能编号。
	7: 2 位：配置空间寄存器编号。
	1: 0 位：恒为“00”。这是因为CF8h、CFCh端口是32位端口。
	
*/

#include <stdint.h>

#define PCI_CONFIG_ADDR	0xCF8	/*PCI配置空间地址端口*/
#define PCI_CONFIG_DATA	0xCFC	/*PCI配置空间数据端口*/

/*PCI配置空间数据的偏移*/
#define PCI_DEVICE_VENDER								0x00	
#define PCI_STATUS_COMMAND								0x04	
#define PCI_CLASS_CODE_REVISION_ID						0x08	
#define PCI_BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE	0x0C
#define PCI_BASS_ADDRESS0								0x10
#define PCI_BASS_ADDRESS1								0x14
#define PCI_BASS_ADDRESS2								0x18
#define PCI_BASS_ADDRESS3								0x1C
#define PCI_BASS_ADDRESS4								0x20
#define PCI_BASS_ADDRESS5								0x24
#define PCI_CARD_BUS_POINTER							0x28
#define PCI_SUBSYSTEM_ID							    0x2C
#define PCI_EXPANSION_ROM_BASE_ADDR                     0x30
#define PCI_CAPABILITY_LIST                             0x34
#define PCI_RESERVED							        0x38
#define PCI_MAX_LNT_MIN_GNT_IRQ_PIN_IRQ_LINE			0x3C

#define  PCI_COMMAND_IO		0x1	/* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY	0x2	/* Enable response in Memory space */
#define  PCI_COMMAND_MASTER	0x4	/* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL	0x8	/* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE	0x10	/* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#define  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#define  PCI_COMMAND_WAIT	0x80	/* Enable address/data stepping */
#define  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#define  PCI_COMMAND_FAST_BACK	0x200	/* Enable back-to-back writes */
#define  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

/*IO地址和MEM地址的地址mask*/
#define PCI_BASE_ADDR_MEM_MASK           (~0x0FUL)
#define PCI_BASE_ADDR_IO_MASK            (~0x03UL)

#define PCI_BAR_TYPE_INVALID 	0
#define PCI_BAR_TYPE_MEM 		1
#define PCI_BAR_TYPE_IO 		2

#define PCI_MAX_BAR 6		/*每个设备最多有6地址信息*/
#define PCI_MAX_BUS 256		/*PCI总共有256个总线*/
#define PCI_MAX_DEV 32		/*PCI每条总线上总共有32个设备*/
#define PCI_MAX_FUN 8		/*PCI设备总共有8个功能号*/

#define PCI_MAX_DEVICE_NR 256	/*系统最大支持检测多少个设备*/

#ifndef PCI_ANY_ID
#define PCI_ANY_ID (~0)
#endif

/* PCI设备ID */
struct pci_device_id
{
	uint32_t vendor, device;   //vendor and device id or PCI_ANY_ID
	uint32_t subvendor, subdevice;   //subsystem's id or PCI_ANY_ID
	uint32_t class, class_mask;
};

/*PCI地址bar结构体，保存Bass Address （0~5）的信息*/
typedef struct pci_device_bar
{
	unsigned int type;		    /*地址bar的类型（IO地址/MEM地址）*/
    unsigned int base_addr;	    /*地址的值*/
    unsigned int length;	    /*地址的长度*/
} pci_device_bar_t;

#define PCI_DEVICE_INVALID 		0
#define PCI_DEVICE_USING		 	1

/*
PCI设备结构体，用于保存我们所需要的pci信息，并不是和硬件的一样
*/
typedef struct pci_device
{
	int flags; 		/*device flags*/
	
	unsigned char bus;				/*bus总线号*/
	unsigned char dev;				/*device号*/
	unsigned char function;		/*功能号*/
	
	unsigned short vendor_id;		/*配置空间:Vendor ID*/
    unsigned short device_id;		/*配置空间:Device ID*/
    unsigned short command;		    /*配置空间:Command*/
    unsigned short status;		    /*配置空间:Status*/
    
    unsigned int class_code;		/*配置空间:Class Code*/
	unsigned char revision_id;		/*配置空间:Revision ID*/
    unsigned char multi_function;	/*多功能标志*/
    unsigned int card_bus_pointer;
    unsigned short subsystem_vendor_id;
    unsigned short subsystem_device_id;
    unsigned int expansion_rom_base_addr;
    unsigned int capability_list;
    
    unsigned char irq_line;			/*配置空间:IRQ line*/
    unsigned char irq_pin;			/*配置空间:IRQ pin*/
    unsigned char min_gnt;
    unsigned char max_lat;
    pci_device_bar_t bar[PCI_MAX_BAR];	/*有6个地址信息*/
} pci_device_t;

unsigned int pci_device_get_io_addr(pci_device_t *device);
unsigned int pci_device_get_mem_addr(pci_device_t *device);
unsigned int pci_device_get_mem_len(pci_device_t *device);
unsigned int pci_device_get_irq_line(pci_device_t *device);
void pci_enable_bus_mastering(pci_device_t *device);
pci_device_t* pci_get_device(unsigned int vendor_id, unsigned int device_id);
pci_device_t* pci_get_device_by_class_code(unsigned int class, unsigned int sub_class);

pci_device_t *pci_locate_device(unsigned short vendor, unsigned short device);
pci_device_t *pci_locate_class(unsigned short class, unsigned short _subclass);

void pci_device_bar_dump(pci_device_bar_t *bar);
void pci_device_dump(pci_device_t *device);

unsigned int pci_device_read(pci_device_t *device, unsigned int reg);
void pci_device_write(pci_device_t *device, unsigned int reg, unsigned int value);

void init_pci();

#endif  /* _ARCH_PCI_H */
