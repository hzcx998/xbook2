#ifndef _X86_PCI_H
#define _X86_PCI_H

#include <stdint.h>

#define PCI_CONFIG_ADDR	0xCF8	/*PCI配置空间地址端口*/
#define PCI_CONFIG_DATA	0xCFC	/*PCI配置空间数据端口*/

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

#define  PCI_COMMAND_IO		        0x1	    /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY	        0x2	    /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER	        0x4	    /* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL	    0x8	    /* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE	    0x10	/* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE    0x20	/* Enable palette snooping */
#define  PCI_COMMAND_PARITY	        0x40	/* Enable parity checking */
#define  PCI_COMMAND_WAIT	        0x80	/* Enable address/data stepping */
#define  PCI_COMMAND_SERR	        0x100	/* Enable SERR */
#define  PCI_COMMAND_FAST_BACK	    0x200	/* Enable back-to-back writes */
#define  PCI_COMMAND_INTX_DISABLE   0x400   /* INTx Emulation Disable */

#define PCI_BASE_ADDR_MEM_MASK           (~0x0FUL)
#define PCI_BASE_ADDR_IO_MASK            (~0x03UL)

#define PCI_BAR_TYPE_INVALID 	0
#define PCI_BAR_TYPE_MEM 		1
#define PCI_BAR_TYPE_IO 		2

#define PCI_MAX_BAR_NR 6		/*每个设备最多有6地址信息*/
#define PCI_MAX_BUS_NR 256		/*PCI总共有256个总线*/
#define PCI_MAX_DEV_NR 32		/*PCI每条总线上总共有32个设备*/
#define PCI_MAX_FUN_NR 8		/*PCI设备总共有8个功能号*/

#define PCI_MAX_DEV_NRICE_NR 256	/* kernel最大支持检测多少个设备*/

#ifndef PCI_ANY_ID
#define PCI_ANY_ID (~0)
#endif

struct pci_device_id
{
	uint32_t vendor, device;   //vendor and device id or PCI_ANY_ID
	uint32_t subvendor, subdevice;   //subsystem's id or PCI_ANY_ID
	uint32_t class, class_mask;
};

typedef struct pci_device_bar
{
	unsigned int type;		    /*地址bar的类型（IO地址/MEM地址）*/
    unsigned int base_addr;	    /*地址的值*/
    unsigned int length;	    /*地址的长度*/
} pci_device_bar_t;

#define PCI_DEVICE_INVALID 		    0
#define PCI_DEVICE_USING		 	1

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
    pci_device_bar_t bar[PCI_MAX_BAR_NR];	/*有6个地址信息*/
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

void pci_init();

#endif  /* _X86_PCI_H */
