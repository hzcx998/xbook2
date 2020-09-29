#include <xbook/debug.h>
#include <xbook/kernel.h>
#include <const.h>
#include <math.h>
#include <xbook/softirq.h>

#include <xbook/driver.h>
#include <string.h>
#include <xbook/clock.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>
#include <arch/ioremap.h>
#include <arch/memory.h>
#include <xbook/kmalloc.h>
#include <xbook/pci.h>
#include <xbook/mutexlock.h>
#include <xbook/cpu.h>
#include <xbook/bitops.h>
#include <xbook/dma.h>
#include <xbook/task.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <stdio.h>

/* 配置开始 */
/* #define DEBUG_AHCI */

/* 配置结束 */

#define DRV_NAME "ahci-disk"
#define DRV_VERSION "0.1"

#define DEV_NAME "sata"

/* AHCI设备最多的磁盘数量 */
#define MAX_AHCI_DISK_NR			4

/* AHCI磁盘数在BIOS阶段可以储存到这个地址，直接从里面或取就可以了 */
#define AHCI_DISK_NR_ADDR		(KERN_VADDR + 0x0475)

typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

struct fis_reg_host_to_device {
	uint8_t	fis_type;
	
	uint8_t pmport:4;
	uint8_t reserved0:3;
	uint8_t c:1;
	
	uint8_t command;
	uint8_t feature_l;
	
	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;
	
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t feature_h;
	
	uint8_t count_l;
	uint8_t count_h;
	uint8_t icc;
	uint8_t control;
	
	uint8_t reserved1[4];
}__attribute__ ((packed));

struct fis_reg_device_to_host {
	uint8_t fis_type;
	
	uint8_t pmport:4;
	uint8_t reserved0:2;
	uint8_t interrupt:1;
	uint8_t reserved1:1;
	
	uint8_t status;
	uint8_t error;
	
	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;
	
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t reserved2;
	
	uint8_t count_l;
	uint8_t count_h;
	uint8_t reserved3[2];
	
	uint8_t reserved4[4];
}__attribute__ ((packed));

struct fis_data {
	uint8_t fis_type;
	uint8_t pmport:4;
	uint8_t reserved0:4;
	uint8_t reserved1[2];
	
	uint32_t data[1];
}__attribute__ ((packed));

struct fis_pio_setup {
	uint8_t fis_type;
	
	uint8_t pmport:4;
	uint8_t reserved0:1;
	uint8_t direction:1;
	uint8_t interrupt:1;
	uint8_t reserved1:1;
	
	uint8_t status;
	uint8_t error;
	
	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;
	
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t reserved2;
	
	uint8_t count_l;
	uint8_t count_h;
	uint8_t reserved3;
	uint8_t e_status;
	
	uint16_t transfer_count;
	uint8_t reserved4[2];
}__attribute__ ((packed));

struct fis_dma_setup {
	uint8_t fis_type;
	
	uint8_t pmport:4;
	uint8_t reserved0:1;
	uint8_t direction:1;
	uint8_t interrupt:1;
	uint8_t auto_activate:1;
	
	uint8_t reserved1[2];
	
	uint64_t dma_buffer_id;
	
	uint32_t reserved2;
	
	uint32_t dma_buffer_offset;
	
	uint32_t transfer_count;
	
	uint32_t reserved3;
}__attribute__ ((packed));

struct fis_dev_bits {
	volatile uint8_t fis_type;
	
	volatile uint8_t pmport:4;
	volatile uint8_t reserved0:2;
	volatile uint8_t interrupt:1;
	volatile uint8_t notification:1;
	
	volatile uint8_t status;
	volatile uint8_t error;
	
	volatile uint32_t protocol;
}__attribute__ ((packed));

struct hba_port {
	volatile uint32_t command_list_base_l;
	volatile uint32_t command_list_base_h;
	volatile uint32_t fis_base_l;
	volatile uint32_t fis_base_h;
	volatile uint32_t interrupt_status;
	volatile uint32_t interrupt_enable;
	volatile uint32_t command;
	volatile uint32_t reserved0;
	volatile uint32_t task_file_data;
	volatile uint32_t signature;
	volatile uint32_t sata_status;
	volatile uint32_t sata_control;
	volatile uint32_t sata_error;
	volatile uint32_t sata_active;
	volatile uint32_t command_issue;
	volatile uint32_t sata_notification;
	volatile uint32_t fis_based_switch_control;
	volatile uint32_t reserved1[11];
	volatile uint32_t vendor[4];
}__attribute__ ((packed));

struct hba_memory {
	volatile uint32_t capability;
	volatile uint32_t global_host_control;
	volatile uint32_t interrupt_status;
	volatile uint32_t port_implemented;
	volatile uint32_t version;
	volatile uint32_t ccc_control;
	volatile uint32_t ccc_ports;
	volatile uint32_t em_location;
	volatile uint32_t em_control;
	volatile uint32_t ext_capabilities;
	volatile uint32_t bohc;
	
	volatile uint8_t reserved[0xA0 - 0x2C];
	
	volatile uint8_t vendor[0x100 - 0xA0];
	
	volatile struct hba_port ports[1];
}__attribute__ ((packed));

struct hba_received_fis {
	volatile struct fis_dma_setup fis_ds;
	volatile uint8_t pad0[4];
	
	volatile struct fis_pio_setup fis_ps;
	volatile uint8_t pad1[12];
	
	volatile struct fis_reg_device_to_host fis_r;
	volatile uint8_t pad2[4];
	
	volatile struct fis_dev_bits fis_sdb;
	volatile uint8_t ufis[64];
	volatile uint8_t reserved[0x100 - 0xA0];
}__attribute__ ((packed));

struct hba_command_header {
	uint8_t fis_length:5;
	uint8_t atapi:1;
	uint8_t write:1;
	uint8_t prefetchable:1;
	
	uint8_t reset:1;
	uint8_t bist:1;
	uint8_t clear_busy_upon_r_ok:1;
	uint8_t reserved0:1;
	uint8_t pmport:4;
	
	uint16_t prdt_len;
	
	volatile uint32_t prdb_count;
	
	uint32_t command_table_base_l;
	uint32_t command_table_base_h;
	
	uint32_t reserved1[4];
}__attribute__ ((packed));

struct hba_prdt_entry {
	uint32_t data_base_l;
	uint32_t data_base_h;
	uint32_t reserved0;
	
	uint32_t byte_count:22;
	uint32_t reserved1:9;
	uint32_t interrupt_on_complete:1;
}__attribute__ ((packed));

struct hba_command_table {
	uint8_t command_fis[64];
	uint8_t acmd[16];
	uint8_t reserved[48];
	struct hba_prdt_entry prdt_entries[1];
}__attribute__ ((packed));

#define HBA_COMMAND_HEADER_NUM 32

struct ata_identify {
	uint16_t ata_device;
	
	uint16_t dont_care[48];
	
	uint16_t cap0;
	uint16_t cap1;
	
	uint16_t obs[2];
	
	uint16_t free_fall;
	
	uint16_t dont_care_2[8];
	
	uint16_t dma_mode0;
	
	uint16_t pio_modes;
	
	uint16_t dont_care_3[4];
	
	uint16_t additional_supported;
	
	uint16_t rsv1[6];
	
	uint16_t serial_ata_cap0;
	
	uint16_t rsv2;
	
	uint16_t serial_ata_features;
	
	uint16_t serial_ata_features_enabled;
	
	uint16_t maj_ver;
	
	uint16_t min_ver;
	
	uint16_t features0;
	
	uint16_t features1;
	
	uint16_t features2;
	
	uint16_t features3;
	
	uint16_t features4;
	
	uint16_t features5;
	
	uint16_t udma_modes;
	
	uint16_t dont_care_4[11];
	
	uint64_t lba48_addressable_sectors;
	
	uint16_t wqewqe[2];
	
	uint16_t ss_1;
	
	uint16_t rrrrr[4];
	
	uint32_t ss_2;
	
	/* ...and more */
};

#define HBA_PxCMD_ST  (1 << 0)
#define HBA_PxCMD_CR  (1 << 15)
#define HBA_PxCMD_FR  (1 << 14)
#define HBA_PxCMD_FRE (1 << 4)

#define HBA_GHC_AHCI_ENABLE (1 << 31)
#define HBA_GHC_INTERRUPT_ENABLE (1 << 1)
#define HBA_GHC_RESET (1 << 0)

#define ATA_CMD_IDENTIFY 0xEC

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
#define ATA_DEV_ERR 0x01

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35

#define PRDT_MAX_COUNT 0x1000

#define PRDT_MAX_ENTRIES 65535

#define ATA_TFD_TIMEOUT  1000000
#define AHCI_CMD_TIMEOUT 1000000

#define ATA_SECTOR_SIZE 512

#define AHCI_DEFAULT_INT 0

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	    0x96690101	// Port multiplier
 
/* AHCI device type */
#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4
 
#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3
 

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
	unsigned int size;		// Size in Sectors.

	/* 状态信息 */
	unsigned int read_sectors;	// 读取了多少扇区
	unsigned int write_sectors;	// 写入了多少扇区

    uint32_t type;
	int idx;
	mutexlock_t lock;
	void *fis_virt, *clb_virt;
	struct dma_region dma_clb, dma_fis;
	void *ch[HBA_COMMAND_HEADER_NUM];
	struct dma_region ch_dmas[HBA_COMMAND_HEADER_NUM];
	struct ata_identify identify;
	uint32_t slots;
	int created;
} device_extension_t;

/* 声明驱动全局变量 */
static pci_device_t *ahci_pci;
static int ahci_int = 0;
static struct hba_memory *hba_mem;
static device_extension_t *ports[32];
static int ahci_next_device = 0;

pci_device_t *get_ahci_pci (void)
{
	pci_device_t *ahci = pci_locate_class(0x1, 0x6);
	if(!ahci) ahci = pci_locate_device(0x8086, 0x8c03);
	if(!ahci) ahci = pci_locate_device(0x8086, 0x2922);
	if(!ahci)
		return NULL;
    #ifdef DEBUG_AHCI
    pr_dbg("[ahci]: device vendorID %x deviceID %x class code %x\n", ahci->vendor_id, ahci->device_id, ahci->class_code);
	#endif
    //pci_device_dump(ahci);
    
	hba_mem = (void *)(addr_t)ahci->bar[5].base_addr;
    pci_enable_bus_mastering(ahci);

    /* 映射IO物理内存地址到虚拟地址中，才能对设备映射到内存的地址进行操作 */
    if (__ioremap((addr_t)hba_mem, (addr_t)ahci->bar[5].base_addr, ahci->bar[5].length) < 0) {
        pr_err("[ahci] device ioremap on %x length %x failed!\n", ahci->bar[5].base_addr, ahci->bar[5].length);
        return NULL;
    }
    flush_tlb();    // 刷新快表
    #ifdef DEBUG_AHCI
	pr_dbg("[ahci]: mapping hba_mem to %x -> %x\n", hba_mem, ahci->bar[5].base_addr);
	pr_dbg("[ahci]: using interrupt %d\n", ahci->irq_line);
	#endif
    ahci_int = ahci->irq_line;

	return ahci;
}

uint32_t ahci_flush_commands(struct hba_port *port)
{
	/* the commands may not take effect until the command
	 * register is read again by software, because reasons.
	 */
	volatile uint32_t c = port->command;
	c=c;
	return c;
}

void ahci_stop_port_command_engine(volatile struct hba_port *port)
{
	port->command &= ~HBA_PxCMD_ST;
	port->command &= ~HBA_PxCMD_FRE;
	while((port->command & HBA_PxCMD_CR) || (port->command & HBA_PxCMD_FR))
		cpu_pause();
}

void ahci_start_port_command_engine(volatile struct hba_port *port)
{
	while(port->command & HBA_PxCMD_CR);
	port->command |= HBA_PxCMD_FRE;
	port->command |= HBA_PxCMD_ST; 
	ahci_flush_commands((struct hba_port *)port);
}

void ahci_init_hba(struct hba_memory *abar)
{
	if(abar->ext_capabilities & 1) {
		/* request BIOS/OS ownership handoff */
		printk(KERN_NOTICE "[ahci]: requesting AHCI ownership change\n");
		abar->bohc |= (1 << 1);
		while((abar->bohc & 1) || !(abar->bohc & (1<<1))) cpu_pause();
		printk(KERN_NOTICE "[ahci]: ownership change completed\n");
	}
	
	/* enable the AHCI and reset it */
	abar->global_host_control |= HBA_GHC_AHCI_ENABLE;
	abar->global_host_control |= HBA_GHC_RESET;
	/* wait for reset to complete */
	while(abar->global_host_control & HBA_GHC_RESET) cpu_pause();
	/* enable the AHCI and interrupts */
	abar->global_host_control |= HBA_GHC_AHCI_ENABLE;
	abar->global_host_control |= HBA_GHC_INTERRUPT_ENABLE;
    mdelay(20);
    #ifdef DEBUG_AHCI
	printk(KERN_INFO "[ahci]: caps: %x %x ver:%x ctl: %x\n", abar->capability, abar->ext_capabilities, abar->version, abar->global_host_control);
    #endif
}


struct hba_command_header *ahci_initialize_command_header(struct hba_memory *abar, struct hba_port *port, device_extension_t *dev, int slot, int write, int atapi, int prd_entries, int fis_len)
{
	struct hba_command_header *h = (struct hba_command_header *)dev->clb_virt;
	h += slot;
	h->write=write ? 1 : 0;
	h->prdb_count=0;
	h->atapi=atapi ? 1 : 0;
	h->fis_length = fis_len;
	h->prdt_len=prd_entries;
	h->prefetchable=0;
	h->bist=0;
	h->pmport=0;
	h->reset=0;
	return h;
}

struct fis_reg_host_to_device *ahci_initialize_fis_host_to_device(struct hba_memory *abar, struct hba_port *port, device_extension_t *dev, int slot, int cmdctl, int ata_command)
{
	struct hba_command_table *tbl = (struct hba_command_table *)(dev->ch[slot]);
	struct fis_reg_host_to_device *fis = (struct fis_reg_host_to_device *)(tbl->command_fis);
	
	memset(fis, 0, sizeof(*fis));
	fis->fis_type = FIS_TYPE_REG_H2D;
	fis->command = ata_command;
	fis->c=cmdctl?1:0;
	return fis;
}

void ahci_send_command(struct hba_port *port, int slot)
{
	port->interrupt_status = ~0;
	port->command_issue = (1 << slot);
	ahci_flush_commands(port);
}

int ahci_write_prdt(struct hba_memory *abar, struct hba_port *port, device_extension_t *dev, int slot, int offset, int length, addr_t virt_buffer)
{
	int num_entries = ((length-1) / PRDT_MAX_COUNT) + 1;
	struct hba_command_table *tbl = (struct hba_command_table *)(dev->ch[slot]);
	int i;
	struct hba_prdt_entry *prd;
	for(i=0;i<num_entries-1;i++)
	{
		/* TODO: do we need to do this? */
		addr_t phys_buffer;
        phys_buffer = addr_v2p(virt_buffer);
		//mm_virtual_getmap(virt_buffer, &phys_buffer, 0);
		prd = &tbl->prdt_entries[i+offset];
		prd->byte_count = PRDT_MAX_COUNT-1;
		prd->data_base_l = low32(phys_buffer);
		prd->data_base_h = 0;
		prd->interrupt_on_complete=0;
		
		length -= PRDT_MAX_COUNT;
		virt_buffer += PRDT_MAX_COUNT;
	}
	addr_t phys_buffer;
    phys_buffer = addr_v2p(virt_buffer);
	//mm_virtual_getmap(virt_buffer, &phys_buffer, 0);
	prd = &tbl->prdt_entries[i+offset];
	prd->byte_count = length-1;
	prd->data_base_l = low32(phys_buffer);
	prd->data_base_h = 0;
	prd->interrupt_on_complete=0;
	
	return num_entries;
}

void ahci_reset_device(struct hba_memory *abar, struct hba_port *port, device_extension_t *dev)
{
	/* TODO: This needs to clear out old commands and lock properly so that new commands can't get sent
	 * while the device is resetting */
	#ifdef DEBUG_AHCI
    printk(KERN_NOTICE "[ahci]: device %d: sending COMRESET and reinitializing\n", dev->idx);
	#endif
    ahci_stop_port_command_engine(port);
	port->sata_error = ~0;
	/* power on, spin up */
	port->command |= 2;
	port->command |= 4;
	ahci_flush_commands(port);
	mdelay(1);
	/* initialize state */
	port->interrupt_status = ~0; /* clear pending interrupts */
	port->interrupt_enable = AHCI_DEFAULT_INT; /* we want some interrupts */
	port->command &= ~((1 << 27) | (1 << 26)); /* clear some bits */
	port->sata_control |= 1;
	mdelay(10);
	port->sata_control |= (~1);
	mdelay(10);
	port->interrupt_status = ~0; /* clear pending interrupts */
	port->interrupt_enable = AHCI_DEFAULT_INT; /* we want some interrupts */
	ahci_start_port_command_engine(port);
	dev->slots=0;
	port->sata_error = ~0;
}

int ahci_port_dma_data_transfer(struct hba_memory *abar, struct hba_port *port, device_extension_t *dev, int slot, int write, addr_t virt_buffer, int sectors, uint64_t lba)
{
	int timeout;
	int fis_len = sizeof(struct fis_reg_host_to_device) / 4;
	int ne = ahci_write_prdt(abar, port, dev,
			slot, 0, ATA_SECTOR_SIZE * sectors, virt_buffer);
	ahci_initialize_command_header(abar,
			port, dev, slot, write, 0, ne, fis_len);
	struct fis_reg_host_to_device *fis = ahci_initialize_fis_host_to_device(abar,
			port, dev, slot, 1, write ? ATA_CMD_WRITE_DMA_EX : ATA_CMD_READ_DMA_EX);
	fis->device = 1<<6;
	/* WARNING: assumes little-endian */
	fis->count_l = sectors & 0xFF;
	fis->count_h = (sectors >> 8) & 0xFF;
	
	fis->lba0 = (unsigned char)( lba        & 0xFF);
	fis->lba1 = (unsigned char)((lba >> 8)  & 0xFF);
	fis->lba2 = (unsigned char)((lba >> 16) & 0xFF);
	fis->lba3 = (unsigned char)((lba >> 24) & 0xFF);
	fis->lba4 = (unsigned char)((lba >> 32) & 0xFF);
	fis->lba5 = (unsigned char)((lba >> 40) & 0xFF);
	port->sata_error = ~0;
	timeout = ATA_TFD_TIMEOUT;
	while ((port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && --timeout)
	{
		//tm_schedule();
        // cpu yeild
        //cpu_pause();
        task_yeild();
	}
	if(!timeout) goto port_hung;
	
	port->sata_error = ~0;
	ahci_send_command(port, slot);
	timeout = ATA_TFD_TIMEOUT;
	while ((port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && --timeout)
	{
		//cpu_pause();
        task_yeild();
	}
	if(!timeout) goto port_hung;
	
	timeout = AHCI_CMD_TIMEOUT;
	while(--timeout)
	{
		if(!((port->sata_active | port->command_issue) & (1 << slot)))
			break;
		//cpu_pause();
        task_yeild();
	}
	if(!timeout) goto port_hung;
	if(port->sata_error)
	{
		printk(KERN_ERR "[ahci]: device %d: ahci error\n", dev->idx);
		goto error;
	}
	if(port->task_file_data & ATA_DEV_ERR)
	{
		printk(KERN_ERR "[ahci]: device %d: task file data error\n", dev->idx);
		goto error;
	}
	return 1;
	port_hung:
	printk(KERN_ERR "[ahci]: device %d: port hung\n", dev->idx);
	error:
	printk(KERN_ERR "[ahci]: device %d: tfd=%x, serr=%x\n",
			dev->idx, port->task_file_data, port->sata_error);
	ahci_reset_device(abar, port, dev);
	return 0;
}

int ahci_device_identify_ahci(struct hba_memory *abar,
		struct hba_port *port, device_extension_t *dev)
{
	int fis_len = sizeof(struct fis_reg_host_to_device) / 4;
	struct dma_region dma;
	dma.p.size = 0x1000;
	dma.p.alignment = 0x1000;
	alloc_dma_buffer(&dma);
	ahci_write_prdt(abar, port, dev, 0, 0, 512, (addr_t)dma.v);
	ahci_initialize_command_header(abar,
			port, dev, 0, 0, 0, 1, fis_len);
	ahci_initialize_fis_host_to_device(abar,
			port, dev, 0, 1, ATA_CMD_IDENTIFY);
	int timeout = ATA_TFD_TIMEOUT;
	port->sata_error = ~0;
	while ((port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && --timeout)
		cpu_pause();
	if(!timeout)
	{
		printk(KERN_ERR "[ahci]: device %d: identify 1: port hung\n", dev->idx);
		printk(KERN_ERR "[ahci]: device %d: identify 1: tfd=%x, serr=%x\n",
				dev->idx, port->task_file_data, port->sata_error);
		free_dma_buffer(&dma);
		return 0;
	}

	ahci_send_command(port, 0);
    #ifdef DEBUG_AHCI
    printk(KERN_DEBUG "[AHCI]: port %d sata active %x command issue %x\n", dev->idx, port->sata_active, port->command_issue);
	#endif
    timeout = AHCI_CMD_TIMEOUT;
	while(--timeout)
	{
		if(!((port->sata_active | port->command_issue) & 1))
			break;
	}
    #ifdef DEBUG_AHCI
    printk(KERN_DEBUG "[AHCI]: port %d sata active %x command issue %x\n", dev->idx, port->sata_active, port->command_issue);
	#endif
	if(!timeout)
	{
		printk(KERN_ERR "[ahci]: device %d: identify 2: port hung\n", dev->idx);
		printk(KERN_ERR "[ahci]: device %d: identify 2: tfd=%x, serr=%x\n",
				dev->idx, port->task_file_data, port->sata_error);
		free_dma_buffer(&dma);
		return 0;
	}
	
	memcpy(&dev->identify, (void *)dma.v, sizeof(struct ata_identify));
	free_dma_buffer(&dma);
    #ifdef DEBUG_AHCI
	printk(KERN_INFO "[ahci]: device %d: num sectors=%d: %x, %x\n", dev->idx,
			dev->identify.lba48_addressable_sectors, dev->identify.ss_2);
	#endif
    return 1;
}

uint32_t ahci_check_type(volatile struct hba_port *port)
{
	port->command &= ~1;
	while(port->command & (1 << 15)) cpu_pause();
	port->command &= ~(1 << 4);
	while(port->command & (1 << 14)) cpu_pause();
	io_mfence();
    port->command |= 2;
    io_mfence();
	mdelay(10);

	uint32_t s = port->sata_status;

	//printk(KERN_INFO "[ahci]: port data: sig=%x, stat=%x, ctl=%x, sac=%x\n", port->signature, s, port->command, port->sata_active);
	uint8_t ipm, det;
	ipm = (s >> 8) & 0x0F;
	det = s & 0x0F;
	//printk(KERN_INFO "[ahci]: port check: ipm=%x, det=%x\n", ipm, det);
	if(ipm != HBA_PORT_IPM_ACTIVE || det != HBA_PORT_DET_PRESENT)
		return AHCI_DEV_NULL;
    switch (port->signature)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
	return AHCI_DEV_SATA;
}

int ahci_initialize_device(struct hba_memory *abar, device_extension_t *dev)
{
    #ifdef DEBUG_AHCI
	printk(KERN_INFO "[ahci]: initializing device %d\n", dev->idx);
	#endif
    struct hba_port *port = (struct hba_port *)&abar->ports[dev->idx];
	ahci_stop_port_command_engine(port);
	port->sata_error = ~0;
	/* power on, spin up */
	port->command |= (2 | 4);
	ahci_flush_commands(port);
	mdelay(1);
	/* initialize state */
	port->interrupt_status = ~0; /* clear pending interrupts */
	port->interrupt_enable = AHCI_DEFAULT_INT; /* we want some interrupts */

	port->command &= ~1;
	while(port->command & (1 << 15)) cpu_pause();
	port->command &= ~((1 << 27) | (1 << 26) | 1); /* clear some bits */
	ahci_flush_commands(port);
    #ifdef DEBUG_AHCI
    pr_dbg("[AHCI]: step1: port %d sata status %x control %x.\n", dev->idx, 
        port->sata_status, port->sata_control);
    #endif        
    /* start reset sata */
	port->sata_control |= 1;
	mdelay(20);
    #ifdef DEBUG_AHCI
    pr_dbg("[AHCI]: step2: port %d sata status %x control %x.\n", dev->idx, 
        port->sata_status, port->sata_control);
    #endif
    /* close DET, after init sata device done. */
	port->sata_control &= (~1);
	mdelay(10);
    #ifdef DEBUG_AHCI
    pr_dbg("[AHCI]: step3: port %d sata status %x control %x.\n", dev->idx, 
        port->sata_status, port->sata_control);
    #endif
	while(!(port->sata_status & 1)) cpu_pause();
	port->sata_error = ~0;
	port->command |= (1 << 28); /* set interface to active */
	while((port->sata_status >> 8) != 1) cpu_pause();
	port->interrupt_status = ~0; /* clear pending interrupts */
	port->interrupt_enable = AHCI_DEFAULT_INT; /* we want some interrupts */
	#ifdef DEBUG_AHCI
    pr_dbg("[AHCI]: map command list dma addr and fis dma addr start.\n");
    #endif
    /* map memory */
	addr_t clb_phys, fis_phys;
	
	dev->dma_clb.p.size = 0x2000;
	dev->dma_clb.p.alignment = 0x1000;
	dev->dma_fis.p.size = 0x1000;
	dev->dma_fis.p.alignment = 0x1000;

	alloc_dma_buffer(&dev->dma_clb);
	alloc_dma_buffer(&dev->dma_fis);

	dev->clb_virt = (void *)dev->dma_clb.v;
	dev->fis_virt = (void *)dev->dma_fis.v;
	clb_phys = dev->dma_clb.p.address;
	fis_phys = dev->dma_fis.p.address;
	dev->slots=0;
	struct hba_command_header *h = (struct hba_command_header *)dev->clb_virt;
	int i;
	for(i=0;i<HBA_COMMAND_HEADER_NUM;i++) {
		dev->ch_dmas[i].p.size = 0x1000;
		dev->ch_dmas[i].p.alignment = 0x1000;
		alloc_dma_buffer(&dev->ch_dmas[i]);
		dev->ch[i] = (void *)dev->ch_dmas[i].v;
		memset(h, 0, sizeof(*h));
		h->command_table_base_l = low32(dev->ch_dmas[i].p.address);
		h->command_table_base_h = 0;
		h++;
	}
	
	port->command_list_base_l = low32(clb_phys);
	port->command_list_base_h = 0;
	
	port->fis_base_l = low32(fis_phys);
	port->fis_base_h = 0;
 	ahci_start_port_command_engine(port);
	port->sata_error = ~0;
    #ifdef DEBUG_AHCI
    pr_dbg("[AHCI]: map command list dma addr and fis dma addr done.\n");
    #endif
	return ahci_device_identify_ahci(abar, port, dev);
}

iostatus_t ahci_create_device(driver_object_t *driver, device_extension_t *dev)
{
    iostatus_t status ;
    device_object_t *devobj;
    char devname[DEVICE_NAME_LEN] = {0};

    sprintf(devname, "%s%d", DEV_NAME, ahci_next_device++);
    /* 初始化一些其它内容 */
    status = io_create_device(driver, 0, devname, DEVICE_TYPE_DISK, &devobj);
    if (status != IO_SUCCESS) {
        printk(KERN_ERR "[ahci]: create device on port %d failed!\n", dev->idx);
        return status;
    }
    /* buffered io mode */
    devobj->flags = DO_BUFFERED_IO;
    devobj->device_extension = dev;
    dev->device_object = devobj;
	dev->created = 1;

    return status;
}

int ahci_probe_ports(driver_object_t *driver, struct hba_memory *abar)
{
	uint32_t pi = abar->port_implemented;
    #ifdef DEBUG_AHCI
	printk(KERN_DEBUG "[ahci]: ports implemented: %x\n", pi);
	#endif
    int counts = 0; /* exist device count */
    int i=0;
	while(i < 32) {
		if(pi & 1) {
			uint32_t type = ahci_check_type(&abar->ports[i]);
			if(type == AHCI_DEV_SATA) { /* SATA device */
                #ifdef DEBUG_AHCI
				printk(KERN_DEBUG "[ahci]: detected SATA device on port %d\n", i);
                #endif
                /* 创建设备扩展 */
				ports[i] = kmalloc(sizeof(device_extension_t));
				ports[i]->type = type;
				ports[i]->idx = i;
                mutexlock_init(&(ports[i]->lock));
				if(ahci_initialize_device(abar, ports[i])) {
					/* create one device on port i */
                    #ifdef DEBUG_AHCI
                    printk(KERN_DEBUG "[ahci]: success to initialize device %d, disabling port\n", i);
                    #endif
			        if (ahci_create_device(driver, ports[i]) < 0) {
                        printk(KERN_ERR "[ahci]: failed to create device %d, disabling port\n", i);    
                    } 
                    counts++;
				} else {
					printk(KERN_ERR "[ahci]: failed to initialize device %d, disabling port\n", i);
                }
            } else if(type == AHCI_DEV_SATAPI) { /* SATA device */
                printk(KERN_WARING "[ahci]: not support SATAPI device on port %d now!\n", i);
            } else if(type == AHCI_DEV_PM) { /* SATA device */
                printk(KERN_WARING "[ahci]: not support Port multiplier on port %d now!\n", i);
            } else if(type == AHCI_DEV_SEMB) { /* SATA device */
                printk(KERN_WARING "[ahci]: not support Enclosure management bridge on port %d now!\n", i);
            }
            /* 暂时不处理其它类型的设备 */
		}
		i++;
		pi >>= 1;
	}
    return counts;
}

int ahci_port_acquire_slot(device_extension_t *dev)
{
	while(1) {
		int i;
		mutex_lock(&dev->lock);
		for(i=0;i<32;i++)
		{
			if(!(dev->slots & (1 << i))) {
				dev->slots |= (1 << i);
				mutex_unlock(&dev->lock);
				return i;
			}
		}
		mutex_unlock(&dev->lock);
		// yeild
        //cpu_pause();
        task_yeild();
	}
}

void ahci_port_release_slot(device_extension_t *dev, int slot)
{
	mutex_lock(&dev->lock);
	dev->slots &= ~(1 << slot);
	mutex_unlock(&dev->lock);
}

/* since a DMA transfer must write to contiguous physical RAM, we need to allocate
 * buffers that allow us to create PRDT entries that do not cross a page boundary.
 * That means that each PRDT entry can transfer a maximum of PAGE_SIZE bytes (for
 * 0x1000 page size, that's 8 sectors). Thus, we allocate a buffer that is page aligned, 
 * in a multiple of PAGE_SIZE, so that the PRDT will write to contiguous physical ram
 * (the key here is that the buffer need not be contiguous across multiple PRDT entries).
 */
int ahci_rw_multiple_do(int rw, int min, uint64_t blk, unsigned char *out_buffer, int count)
{
	uint32_t length = count * ATA_SECTOR_SIZE;
	int d = min;
	device_extension_t *dev = ports[d];
	uint64_t end_blk = dev->identify.lba48_addressable_sectors;
	if(blk >= end_blk)
		return 0;
	if((blk+count) > end_blk)
		count = end_blk - blk;
	if(!count)
		return 0;
	int num_pages = ((ATA_SECTOR_SIZE * (count-1)) / PAGE_SIZE) + 1;
	ASSERT(length <= (unsigned)num_pages * 0x1000);
	struct dma_region dma;
	dma.p.size = 0x1000 * num_pages;
	dma.p.alignment = 0x1000;
	alloc_dma_buffer(&dma);
	int num_read_blocks = count;
	struct hba_port *port = (struct hba_port *)&hba_mem->ports[dev->idx];
	if(rw == 1)
		memcpy((void *)dma.v, out_buffer, length);
	
	int slot=ahci_port_acquire_slot(dev);
	if(!ahci_port_dma_data_transfer(hba_mem, port, dev, slot, rw == 1 ? 1 : 0, (addr_t)dma.v, count, blk))
		num_read_blocks = 0;
	
	ahci_port_release_slot(dev, slot);
	
	if(rw == 0 && num_read_blocks)
		memcpy(out_buffer, (void *)dma.v, length);
	
	free_dma_buffer(&dma);
	return num_read_blocks * ATA_SECTOR_SIZE;
}

/* and then since there is a maximum transfer amount because of the page size
 * limit, wrap the transfer function to allow for bigger transfers than that even.
 */
int ahci_rw_multiple(int rw, int min, uint64_t blk, unsigned char *out_buffer, int count)
{
	int i=0;
	int ret=0;
	int c = count;
	for(i=0;i<count;i+=(PRDT_MAX_ENTRIES * PRDT_MAX_COUNT) / ATA_SECTOR_SIZE)
	{
		int n = (PRDT_MAX_ENTRIES * PRDT_MAX_COUNT) / ATA_SECTOR_SIZE;
		if(n > c)
			n=c;
		ret += ahci_rw_multiple_do(rw, min, blk+i, out_buffer + ret, n);
		c -= n;
	}
	return ret;
}

/**
 * ahci_read_sector - 读扇区
 * @dev: 设备
 * @lba: 逻辑扇区地址
 * @count: 扇区数
 * @buf: 扇区缓冲
 * 
 * 数据读取磁盘，成功返回读取到的数据数量，失败返回0
 */
static int ahci_read_sector(device_extension_t *ext,
	unsigned int lba,
	void *buf,
	unsigned int count)
{
    return ahci_rw_multiple(0, ext->idx, lba, buf, count);
}

/**
 * ahci_write_sector - 写扇区
 * @dev: 设备
 * @lba: 逻辑扇区地址
 * @count: 扇区数
 * @buf: 扇区缓冲
 * 
 * 把数据写入磁盘，成功返回写入的数据数量，失败返回0
 */
static int ahci_write_sector(
    device_extension_t *ext,
	unsigned int lba,
	void *buf,
	unsigned int count
) {
	return ahci_rw_multiple(1, ext->idx, lba, buf, count);
}

iostatus_t ahci_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;
    unsigned long arg = ioreq->parame.devctl.arg;
    device_extension_t *ext = device->device_extension;

    iostatus_t status = IO_SUCCESS;
    int infomation = 0;
    switch (ctlcode)
    {
    case DISKIO_GETSIZE:
        *((unsigned int *) arg) = ext->size; 
        break;
    case DISKIO_CLEAR:
        //ahci_clean_disk(device->device_extension, arg);
        break;
    default:
        infomation = -1;
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = infomation;
    io_complete_request(ioreq);
    return status;
}

iostatus_t ahci_read(device_object_t *device, io_request_t *ioreq)
{
    long len;
    iostatus_t status = IO_SUCCESS;
    sector_t sectors = DIV_ROUND_UP(ioreq->parame.read.length, SECTOR_SIZE);
#ifdef DEBUG_AHCI
    printk(KERN_DEBUG "ahci_read: buf=%x sectors=%d off=%x\n", 
        ioreq->system_buffer, sectors, ioreq->parame.read.offset);
#endif    
    len = ahci_read_sector(device->device_extension, ioreq->parame.read.offset,
        ioreq->system_buffer, sectors);
    
    if (!len) { /* 执行失败 */
        status = IO_FAILED;
        len = 0;
    }
    //loop_delay(1);

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = len;
    
    io_complete_request(ioreq);

    return status;
}

iostatus_t ahci_write(device_object_t *device, io_request_t *ioreq)
{
    long len;
    iostatus_t status = IO_SUCCESS;
    sector_t sectors = DIV_ROUND_UP(ioreq->parame.write.length, SECTOR_SIZE);
#ifdef DEBUG_AHCI
    printk(KERN_DEBUG "ahci_write: buf=%x sectors=%d off=%x\n", 
        ioreq->system_buffer, sectors, ioreq->parame.write.offset);
#endif    

    len = ahci_write_sector(device->device_extension, ioreq->parame.write.offset,
        ioreq->system_buffer, sectors);
    
    if (!len) { /* 执行失败 */
        status = IO_FAILED;
        len = 0;
    }

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = len;
    
    io_complete_request(ioreq);

    return status;
}

/**
 * ahci_handler - ahci硬盘中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int ahci_handler(unsigned long irq, unsigned long data)
{
    int i;
	for(i=0;i<32;i++) {
		if(hba_mem->interrupt_status & (1 << i)) {
            pr_dbg("ahci: interrupt %d occur!\n", i);
			hba_mem->ports[i].interrupt_status = ~0;
			hba_mem->interrupt_status = (1 << i);
			ahci_flush_commands((struct hba_port *)&hba_mem->ports[i]);
		}
	}
    return 0;
}

static iostatus_t ahci_enter(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    printk(KERN_INFO "[ahci]: initializing ahci driver...\n");
	if(!(ahci_pci = get_ahci_pci())) {
		printk(KERN_ERR "[ahci]: no AHCI controllers present!\n");
		status = IO_FAILED;
        return status;
	}
    
    if (register_irq(ahci_int, ahci_handler, IRQF_SHARED, "ahci", "ahci driver", (addr_t)driver) < 0) {
		printk(KERN_ERR "[ahci]: register interrupt failed!\n");
		/* 需要取消内存映射以及关闭ahci总线 */
        status = IO_FAILED;
        return status;
	}
    ahci_init_hba(hba_mem);
    if (!ahci_probe_ports(driver, hba_mem)) {
        printk(KERN_INFO "[ahci]: initializing ahci driver failed!.\n");
        unregister_irq(ahci_int, (addr_t)driver);
        status = IO_FAILED;
        return status;
    }
    
    printk(KERN_INFO "[ahci]: initializing ahci driver done.\n");
    //spin("ahci");
    return status;
}

static iostatus_t ahci_exit(driver_object_t *driver)
{
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    device_extension_t *ext;
    unregister_irq(ahci_int, driver);

    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        ext = devobj->device_extension;
        
        free_dma_buffer(&(ext->dma_clb));
        free_dma_buffer(&(ext->dma_fis));
        int j;
        for(j = 0; j < HBA_COMMAND_HEADER_NUM; j++)
            free_dma_buffer(&(ext->ch_dmas[j]));

        kfree(ext);
        io_delete_device(devobj);   /* 删除每一个设备 */
    }
    
    string_del(&driver->name); /* 删除驱动名 */
    return IO_SUCCESS;
}

iostatus_t ahci_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = ahci_enter;
    driver->driver_exit = ahci_exit;

    driver->dispatch_function[IOREQ_READ] = ahci_read;
    driver->dispatch_function[IOREQ_WRITE] = ahci_write;
    driver->dispatch_function[IOREQ_DEVCTL] = ahci_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_AHCI
    printk(KERN_DEBUG "ahci_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    return status;
}

static __init void ahci_driver_entry(void)
{
    if (driver_object_create(ahci_driver_func) < 0) {
        printk(KERN_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(ahci_driver_entry);
