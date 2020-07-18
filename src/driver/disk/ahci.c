#include <xbook/debug.h>
#include <xbook/kernel.h>
#include <const.h>
#include <math.h>
#include <xbook/softirq.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <vsprintf.h>
#include <xbook/clock.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>
#include <arch/ioremap.h>
#include <arch/memory.h>
#include <xbook/kmalloc.h>
#include <xbook/pci.h>
#include <sys/ioctl.h>

/* 配置开始 */
#define DEBUG_LOCAL 1

/* 配置结束 */

#define DRV_NAME "ahci-disk"
#define DRV_VERSION "0.1"

#define DEV_NAME "sata"

/* AHCI设备最多的磁盘数量 */
#define MAX_AHCI_DISK_NR			4

/* AHCI磁盘数在BIOS阶段可以储存到这个地址，直接从里面或取就可以了 */
#define AHCI_DISK_NR_ADDR		(KERN_VADDR + 0x0475)


//////SATA & ATA Command
#define ATA_CMD_READ_SECTOR			0x20
#define ATA_CMD_READ_SECTOR_EXT			0x24

#define ATA_CMD_READ_DMA			0xC8
#define ATA_CMD_READ_DMA_EXT			0x25

#define ATA_CMD_WRITE_SECTOR			0x30
#define ATA_CMD_WRITE_SECTOR_EXT		0x34

#define ATA_CMD_WRITE_DMA			0xCA
#define ATA_CMD_WRITE_DMA_EXT			0x35

#define ATA_CMD_IDENTIFY_DISK			0xEC

//////FIS Type value assignments
#define	AHCI_FIS_TYPE_HOST2DEVICE_FIS		0x27
#define	AHCI_FIS_TYPE_DEVICE2HOST_FIS		0x34
#define	AHCI_FIS_TYPE_DMA_ACTIVE_FIS		0x39
#define	AHCI_FIS_TYPE_DMA_SETUP_FIS		0x41
#define	AHCI_FIS_TYPE_DATA_FIS			0x46
#define	AHCI_FIS_TYPE_BIST_ACTIVE_FIS		0x58
#define	AHCI_FIS_TYPE_PIO_SETUP_FIS		0x5F
#define	AHCI_FIS_TYPE_SET_DEVICE_BIT_FIS	0xA1

struct Generic_Host_Control
{
	unsigned int CAP;	//Host Capabilities
	unsigned int GHC;	//Global Host Control
	unsigned int IS;	//Interrupt Status
	unsigned int PI;	//Ports Implemented
	unsigned int VS;	//Version
	unsigned int CCC_CTL;	//Command Completion Coalescing Control
	unsigned int CCC_PORTs;	//Command Completion Coalsecing Ports
	unsigned int EM_LOC;	//Enclosure Management Location
	unsigned int EM_CTL;	//Enclosure Management Control
	unsigned int CAP2;	//Host Capabilities Extended
	unsigned int BOHC;	//BIOS/OS Handoff Control and Status
}__attribute__((packed));

struct Port_X_Control_Registers
{
	unsigned int PxCLB;		//Port x Command List Base Address
	unsigned int PxCLBU;		//Port x Command List Base Address Upper 32-Bits
	unsigned int PxFB;		//Port x FIS Base Address
	unsigned int PxFBU;		//Port x FIS Base Address Upper 32-Bits

	unsigned int PxIS;		//Port x Interrupt Status
	unsigned int PxIE;		//Port x Interrupt Enable
	unsigned int PxCMD;		//Port x Command and Status
	unsigned int Reserved0;		//Reserved

	unsigned int PxTFD;		//Port x Task File Data
	unsigned int PxSIG;		//Port x Signature
	unsigned int PxSSTS;		//Port x Serial ATA Status (SCR0: SStatus)
	unsigned int PxSCTL;		//Port x Serial ATA Control (SCR2: SControl)

	unsigned int PxSERR;		//Port x Serial ATA Error (SCR1: SError)
	unsigned int PxSACT;		//Port x Serial ATA Active (SCR3: SActive)
	unsigned int PxCI;		//Port x Command Issue
	unsigned int PxSNTF;		//Port x Serial ATA Notification (SCR4: SNotification)

	unsigned int PxFBS;		//Port x FIS-based Switching Control
	unsigned int PxDEVSLP;		//Port x Device Sleep
	unsigned int Reserved1[10];	//Reserved
	unsigned long PxVS[2];		//Port x Vendor Specific
}__attribute__((packed));

struct HBA_Memory_Registers
{
    
	pci_device_t *PCI_Dev;              /* pci设备指针 */
	struct Generic_Host_Control *GHC;
	struct Port_X_Control_Registers *PxCR;
}__attribute__((packed));

/*	DMA Setup – Device to Host FIS or Host to Device FIS (bidirectional),FIS Type (41h)	*/

struct DsFIS
{
	unsigned char	FIS_Type;
	unsigned char	PM_Port:4,
			 :1,
			D:1,
			I:1,
			A:1;
	unsigned char	reserved0[2];

	unsigned long	DMA_Buffer_ID;

	unsigned int	reserved1;

	unsigned int	DMA_Buffer_Offset;

	unsigned int	DMA_Transfer_Count;

	unsigned int	reserved2;
}__attribute__((packed));

/*	PIO Setup – Device to Host FIS,FIS Type (5Fh)	*/

struct PsFIS
{
	unsigned char	FIS_Type;
	unsigned char	PM_Port:4,
			 :1,
			D:1,
			I:1,
			 :1;
	unsigned char	Status;
	unsigned char	Error;

	unsigned char	LBA0;
	unsigned char	LBA1;
	unsigned char	LBA2;
	unsigned char	Device;

	unsigned char	LBA3;
	unsigned char	LBA4;
	unsigned char	LBA5;
	unsigned char	reserved0;

	unsigned short	Count;
	unsigned char	reserved1;
	unsigned char	E_Status;

	unsigned short	Transfer_count;
	unsigned short	reserved2;	
}__attribute__((packed));

/*	Register Host to Device FIS,FIS Type (27h)	*/

struct H2DFIS
{
	unsigned char	FIS_Type;
	unsigned char	PM_Port:4,
			 :3,
			C:1;
	unsigned char	Command;
	unsigned char	Features0;

	unsigned char	LBA0;
	unsigned char	LBA1;
	unsigned char	LBA2;
	unsigned char	Device;

	unsigned char	LBA3;
	unsigned char	LBA4;
	unsigned char	LBA5;
	unsigned char	Features1;

	unsigned short	Count;
	unsigned char	ICC;
	unsigned char	Control;

	unsigned int	reserved;
}__attribute__((packed));

/*	Register Device to Host FIS,FIS Type (34h)	*/

struct D2HFIS
{
	unsigned char	FIS_Type;
	unsigned char	PM_Port:4,
			 :2,
			I:1,
			 :1;
	unsigned char	Status;
	unsigned char	Error;

	unsigned char	LBA0;
	unsigned char	LBA1;
	unsigned char	LBA2;
	unsigned char	Device;

	unsigned char	LBA3;
	unsigned char	LBA4;
	unsigned char	LBA5;
	unsigned char	reserved0;

	unsigned short	Count;
	unsigned short	reserved1;

	unsigned int	reserved2;
}__attribute__((packed));

/*	Set Device Bits - Device to Host FIS,FIS Type (A1h)	*/

struct SDBFIS
{
	unsigned char	FIS_Type;
	unsigned char	PM_Port:4,
			 :2,
			I:1,
			N:1;
	unsigned char	Status_Lo:3,
			 :1,
			Status_Hi:3,
			 :1;
	unsigned char	Error;

	unsigned int	Protocol_Specific;
}__attribute__((packed));

/*	BIST Activate FIS - bidirectional,FIS Type (58h)	*/

struct BISTFIS
{
	unsigned char	FIS_Type;
	unsigned char	PM_Port:4,
			 :4;

	unsigned char	V:1,
			 :1,
			P:1,
			F:1,
			L:1,
			S:1,
			A:1,
			T:1;
	unsigned char	reserved;

	unsigned long	Data;
}__attribute__((packed));

struct Received_FIS
{
	struct DsFIS	DMA_Setup_FIS;
	unsigned char	reserved0[4];
	struct PsFIS	PIO_Setup_FIS;
	unsigned char	reserved1[12];
	struct D2HFIS	RFIS;
	unsigned char	reserved2[4];
	struct SDBFIS	Set_Device_Bits_FIS;
	unsigned char	UFIS[64];
	unsigned char	reserved3[96];
}__attribute__((packed));

/*	Physical Region Descriptor Table	*/

struct PRDT
{
	unsigned int	DBA;
	unsigned int	DBAU;
	unsigned int	reserved;
	unsigned int	DBC:22,
			R:9,
			I:1;
}__attribute__((packed));

struct Command_Table
{
	unsigned char CMD_FIS[64];
	unsigned char CMD_APAPI[16];
	unsigned char reserved[48];
	struct PRDT CMD_PRDT[];
}__attribute__((packed));

struct Command_Table_Header
{
	unsigned int	CFL:5,
			A:1,
			W:1,
			P:1,
			R:1,
			B:1,
			C:1,
			 :1,
			PMP:4,
			PRDTL:16; 
	unsigned int	PRDBC;
	unsigned int	CTBA;
	unsigned int	CTBAU;

	unsigned int	Reserved0;
	unsigned int	Reserved1;
	unsigned int	Reserved2;
	unsigned int	Reserved3;
}__attribute__((packed));

struct Command_list
{
	struct Command_Table_Header CTBL[32];
}__attribute__((packed));

typedef struct _device_extension {
    string_t device_name;           /* 设备名字 */
    device_object_t *device_object; /* 设备对象 */
	unsigned int size;		// Size in Sectors.

	/* 状态信息 */
	unsigned int read_sectors;	// 读取了多少扇区
	unsigned int write_sectors;	// 写入了多少扇区

/// 
    
    struct HBA_Memory_Registers HBA;

    struct Command_list *cmdheader;
    struct Command_Table *cmdtbl;
    struct PRDT *prdt;

    struct H2DFIS *h2dfis;

    struct Received_FIS *RecFIS;


} device_extension_t;

/**
 * ahci_read_sector - 读扇区
 * @dev: 设备
 * @lba: 逻辑扇区地址
 * @count: 扇区数
 * @buf: 扇区缓冲
 * 
 * 数据读取磁盘，成功返回0，失败返回-1
 */
static int ahci_read_sector(device_extension_t *ext,
	unsigned int lba,
	void *buf,
	unsigned int count)
{
    struct Command_list *cmdheader = ext->cmdheader;
    struct Command_Table *cmdtbl = ext->cmdtbl;
    struct H2DFIS *h2dfis = ext->h2dfis;
    struct PRDT *prdt = ext->prdt;

    ///config Command List Structure(include a lot of Command Header,the Command Header point to a Command Table)
    memset(cmdheader,0,sizeof(struct Command_Table_Header));	///set Physical Region Descriptor Byte Count (PRDBC) == 0
    cmdheader->CTBL[0].CFL = (sizeof(struct H2DFIS) / sizeof(int));	////Command FIS Length (CFL)
    cmdheader->CTBL[0].A = 0;	////1:ATAPI (A)
    cmdheader->CTBL[0].W = 0;	////0:read,1:write
    cmdheader->CTBL[0].C = 1;	////1:clear busy upon R_OK
    cmdheader->CTBL[0].P = 1;	////1:Prefetchable
    cmdheader->CTBL[0].PRDTL = 1;	////Physical Region Descriptor Table Length (PRDTL)

    pr_dbg("%s: cmdtbl %x\n", __func__, cmdtbl);

    cmdheader->CTBL[0].CTBA = (unsigned int)addr_v2p((unsigned long) cmdtbl) & 0xffffffff;
    cmdheader->CTBL[0].CTBAU = 0;
    //cmdheader->CTBL[0].CTBAU = (unsigned int) (((unsigned long)Virt_To_Phy(cmdtbl) >> 32) & 0xffffffff);

    ///Command Table(include CFIS/ACMD/PRDT Structures)
    ///config CFIS(int Command Table)
    memset(cmdtbl,0,sizeof(struct Command_Table) + sizeof(struct PRDT) * cmdheader->CTBL[0].PRDTL);	
    h2dfis = (struct H2DFIS *)cmdtbl->CMD_FIS;
    h2dfis->FIS_Type = AHCI_FIS_TYPE_HOST2DEVICE_FIS;			////h2d
    h2dfis->C = 1;			///1:Command register,0:Device Control register
    h2dfis->Command = ATA_CMD_READ_DMA;	///ATA CMD WRITE DMA
    h2dfis->LBA0 = lba & 0xff;
    h2dfis->LBA1 = (lba >> 8)  & 0xff;
    h2dfis->LBA2 = (lba >> 16) & 0xff;
    h2dfis->LBA3 = (lba >> 24) & 0xff;
    /*h2dfis->LBA4 = (lba >> 32) & 0xff;
    h2dfis->LBA5 = (lba >> 40) & 0xff;
    */
    h2dfis->LBA4 = 0;
    h2dfis->LBA5 = 0;
    h2dfis->Count = count;
    h2dfis->Device = 0xe0;

    ///config PRDT (int Command Table)
    prdt->DBA = (unsigned int)addr_v2p((unsigned long) buf) & 0xffffffff;
    prdt->DBAU = 0;
    //prdt->DBAU = (unsigned int) (((unsigned long)Virt_To_Phy(node->buffer) >> 32) & 0xffffffff);
    prdt->DBC = count * 512 - 1;	////Data Byte Count (DBC)
    prdt->I = 1;	////Interrupt on Completion (I)

/// read set done

//////////////////////////////////////////////////////////////////////////
	///bit 28-31:Interface Communication Control (ICC)
	///bit 04:FIS Receive Enable (FRE)
	///bit 01:Spin-Up Device (SUD)
	///bit 00:Start (ST)
	ext->HBA.PxCR[0].PxCMD = ext->HBA.PxCR[0].PxCMD | 0x10000003;

	///Port x Serial ATA Active
	ext->HBA.PxCR[0].PxSACT = 1;

//	color_printk(PURPLE,BLACK,"1.port:%#010x(PxCLB),%#010x(PxCLBU),%#010x(PxFB),%#010x(PxFBU),%#010x(PxIS),%#010x(PxIE),%#010x(PxCMD),%#010x(PxTFD),%#010x(PxSIG),%#010x(PxSSTS),%#010x(PxSCTL),%#010x(PxSERR),%#010x(PxSACT),%#010x(PxCI),%#010x(PxSNTF),%#010x(PxFBS),%#010x(PxDEVSLP),%#018lx,%#018lx(PxVS)\n",HBA.PxCR[0].PxCLB,HBA.PxCR[0].PxCLBU,HBA.PxCR[0].PxFB,HBA.PxCR[0].PxFBU,HBA.PxCR[0].PxIS,HBA.PxCR[0].PxIE,HBA.PxCR[0].PxCMD,HBA.PxCR[0].PxTFD,HBA.PxCR[0].PxSIG, HBA.PxCR[0].PxSSTS,HBA.PxCR[0].PxSCTL,HBA.PxCR[0].PxSERR,HBA.PxCR[0].PxSACT,HBA.PxCR[0].PxCI,HBA.PxCR[0].PxSNTF,HBA.PxCR[0].PxFBS,HBA.PxCR[0].PxDEVSLP,HBA.PxCR[0].PxVS[1], HBA.PxCR[0].PxVS[0]);	

	///Port x Command Issue
	ext->HBA.PxCR[0].PxCI = 1;

	return 0;
}

/**
 * ahci_write_sector - 写扇区
 * @dev: 设备
 * @lba: 逻辑扇区地址
 * @count: 扇区数
 * @buf: 扇区缓冲
 * 
 * 把数据写入磁盘，成功返回0，失败返回-1
 */
static int ahci_write_sector(
    device_extension_t *ext,
	unsigned int lba,
	void *buf,
	unsigned int count
) {
	struct Command_list *cmdheader = ext->cmdheader;
    struct Command_Table *cmdtbl = ext->cmdtbl;
    struct H2DFIS *h2dfis = ext->h2dfis;
    struct PRDT *prdt = ext->prdt;

    ///config Command List Structure(include a lot of Command Header,the Command Header point to a Command Table)
    memset(cmdheader,0,sizeof(struct Command_Table_Header));	///set Physical Region Descriptor Byte Count (PRDBC) == 0
    cmdheader->CTBL[0].CFL = (sizeof(struct H2DFIS) / sizeof(int));	////Command FIS Length (CFL)
    cmdheader->CTBL[0].A = 0;	////1:ATAPI (A)
    cmdheader->CTBL[0].W = 1;	////0:read,1:write
    cmdheader->CTBL[0].C = 1;	////1:clear busy upon R_OK
    cmdheader->CTBL[0].P = 1;	////1:Prefetchable
    cmdheader->CTBL[0].PRDTL = 1;	////Physical Region Descriptor Table Length (PRDTL)

    pr_dbg("%s: cmdtbl %x\n", __func__, cmdtbl);

    cmdheader->CTBL[0].CTBA = (unsigned int)addr_v2p((unsigned long) cmdtbl) & 0xffffffff;
    cmdheader->CTBL[0].CTBAU = 0;
    //cmdheader->CTBL[0].CTBAU = (unsigned int) (((unsigned long)Virt_To_Phy(cmdtbl) >> 32) & 0xffffffff);

    ///Command Table(include CFIS/ACMD/PRDT Structures)
    ///config CFIS(int Command Table)
    memset(cmdtbl,0,sizeof(struct Command_Table) + sizeof(struct PRDT) * cmdheader->CTBL[0].PRDTL);	
    h2dfis = (struct H2DFIS *)cmdtbl->CMD_FIS;
    h2dfis->FIS_Type = AHCI_FIS_TYPE_HOST2DEVICE_FIS;			////h2d
    h2dfis->C = 1;			///1:Command register,0:Device Control register
    h2dfis->Command = ATA_CMD_WRITE_DMA;	///ATA CMD WRITE DMA
    h2dfis->LBA0 = lba & 0xff;
    h2dfis->LBA1 = (lba >> 8)  & 0xff;
    h2dfis->LBA2 = (lba >> 16) & 0xff;
    h2dfis->LBA3 = (lba >> 24) & 0xff;
    /*h2dfis->LBA4 = (lba >> 32) & 0xff;
    h2dfis->LBA5 = (lba >> 40) & 0xff;
    */
    h2dfis->LBA4 = 0;
    h2dfis->LBA5 = 0;
    h2dfis->Count = count;
    h2dfis->Device = 0xe0;

    ///config PRDT (int Command Table)
    prdt->DBA = (unsigned int)addr_v2p((unsigned long) buf) & 0xffffffff;
    prdt->DBAU = 0;
    //prdt->DBAU = (unsigned int) (((unsigned long)Virt_To_Phy(node->buffer) >> 32) & 0xffffffff);
    prdt->DBC = count * 512 - 1;	////Data Byte Count (DBC)
    prdt->I = 1;	////Interrupt on Completion (I)

/// read set done

//////////////////////////////////////////////////////////////////////////
	///bit 28-31:Interface Communication Control (ICC)
	///bit 04:FIS Receive Enable (FRE)
	///bit 01:Spin-Up Device (SUD)
	///bit 00:Start (ST)
	ext->HBA.PxCR[0].PxCMD = ext->HBA.PxCR[0].PxCMD | 0x10000003;

	///Port x Serial ATA Active
	ext->HBA.PxCR[0].PxSACT = 1;

//	color_printk(PURPLE,BLACK,"1.port:%#010x(PxCLB),%#010x(PxCLBU),%#010x(PxFB),%#010x(PxFBU),%#010x(PxIS),%#010x(PxIE),%#010x(PxCMD),%#010x(PxTFD),%#010x(PxSIG),%#010x(PxSSTS),%#010x(PxSCTL),%#010x(PxSERR),%#010x(PxSACT),%#010x(PxCI),%#010x(PxSNTF),%#010x(PxFBS),%#010x(PxDEVSLP),%#018lx,%#018lx(PxVS)\n",HBA.PxCR[0].PxCLB,HBA.PxCR[0].PxCLBU,HBA.PxCR[0].PxFB,HBA.PxCR[0].PxFBU,HBA.PxCR[0].PxIS,HBA.PxCR[0].PxIE,HBA.PxCR[0].PxCMD,HBA.PxCR[0].PxTFD,HBA.PxCR[0].PxSIG, HBA.PxCR[0].PxSSTS,HBA.PxCR[0].PxSCTL,HBA.PxCR[0].PxSERR,HBA.PxCR[0].PxSACT,HBA.PxCR[0].PxCI,HBA.PxCR[0].PxSNTF,HBA.PxCR[0].PxFBS,HBA.PxCR[0].PxDEVSLP,HBA.PxCR[0].PxVS[1], HBA.PxCR[0].PxVS[0]);	

	///Port x Command Issue
	ext->HBA.PxCR[0].PxCI = 1;

	return 0;
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
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "ahci_read: buf=%x sectors=%d off=%x\n", 
        ioreq->system_buffer, sectors, ioreq->parame.read.offset);
#endif    
    len = ahci_read_sector(device->device_extension, ioreq->parame.read.offset,
        ioreq->system_buffer, sectors);
    
    /// 循环等待中断产生
    while (1)
    {
        /* code */
    }
    
    if (!len) { /* 执行成功 */
        len = sectors * SECTOR_SIZE;
    } else {
        status = IO_FAILED;
    }
    loop_delay(1);

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
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "ahci_write: buf=%x sectors=%d off=%x\n", 
        ioreq->system_buffer, sectors, ioreq->parame.write.offset);
#endif    

    len = ahci_write_sector(device->device_extension, ioreq->parame.write.offset,
        ioreq->system_buffer, sectors);
    if (!len) { /* 执行成功 */
        len = sectors * SECTOR_SIZE;
    } else {
        status = IO_FAILED;
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
    pr_dbg("ahci: interrupt occur!\n");

    return 0;
}

/**
 * ahci_probe - 探测设备
 * @disks: 找到的磁盘数
 * 
 * 根据磁盘数初始化对应的磁盘的信息
 */
static int ahci_probe(device_extension_t *ext, int id)
{
	unsigned int index = 0;
	unsigned int value = 0;

	/* 获取PCI设备信息 */
    pci_device_t *pcidev = pci_get_device_by_class_code(1, 6, 1);
    if (pcidev == NULL) {
        pr_err("ahci not found pci device!\n");
        return -1;
    }
    
    pci_device_dump(pcidev);

    pr_dbg("ahci: start map memory addr.\n");
    /* 检测参数，bar5是内存映射总线 */
    if (pcidev->bar[5].type != PCI_BAR_TYPE_MEM || !pcidev->bar[5].base_addr || !pcidev->bar[5].length) {
        pr_err("ahci device mem addr error!\n");
        return -1;
    }
    pr_dbg("ahci: map memory addr on %x about %x length.\n", pcidev->bar[5].base_addr, pcidev->bar[5].length);

    /* 映射IO物理内存地址到虚拟地址中，才能对设备映射到内存的地址进行操作 */
    if (__ioremap(pcidev->bar[5].base_addr, pcidev->bar[5].base_addr, pcidev->bar[5].length) < 0) {
        pr_err("ahci device ioremap on %x length %x failed!\n", pcidev->bar[5].base_addr, pcidev->bar[5].length);
        return -1;
    }
    flush_tlb();    // 刷新快表
    pr_dbg("ahci: map memory addr done.\n");

	///get GHC address and first Port register address
    ext->HBA.PCI_Dev = pcidev;
    ext->HBA.GHC = (struct Generic_Host_Control *)pcidev->bar[5].base_addr;
	pr_dbg("CHC %x\n", ext->HBA.GHC);
    ext->HBA.PxCR = (struct Port_X_Control_Registers *)(pcidev->bar[5].base_addr + 0x100);
	pr_dbg("PxCR %x\n", ext->HBA.PxCR);

	///bit 31:AHCI Enable (AE)
	///bit 01:Interrupt Enable (IE)
	///bit 00:HBA Reset (HR)
	ext->HBA.GHC->GHC = 0x80000002;
    
	io_mfence();

	pr_dbg("Generic Host Control(00h ~ ffh)\n");
	pr_dbg("%#010x(CAP),%#010x(GHC),%#010x(IS),%#010x(PI)\n",ext->HBA.GHC->CAP,ext->HBA.GHC->GHC,ext->HBA.GHC->IS,ext->HBA.GHC->PI);
	pr_dbg("%#010x(VS),%#010x(CCC_CTL),%#010x(CCC_PORTS),%#010x(EM_LOC)\n",ext->HBA.GHC->VS,ext->HBA.GHC->CCC_CTL,ext->HBA.GHC->CCC_PORTs,ext->HBA.GHC->EM_LOC);
	pr_dbg("%#010x(struct HBA_Memory_RegistersEM_CTL),%#010x(CAP2),%#010x(BOHC)\n",ext->HBA.GHC->EM_CTL,ext->HBA.GHC->CAP2,ext->HBA.GHC->BOHC);

	///bit 00-04:Number of Ports (NP)
	///color_printk(WHITE,BLACK,"ext->HBA.GHC.CAP(NP) & 0x1f:%#02d\n",ext->HBA.GHC->CAP & 0x1f);

	///>>>>>>>>get Received FIS Structure address
	//ext->RecFIS = (struct Received_FIS *)((unsigned long)ext->HBA.PxCR[0].PxFBU << 32) | ext->HBA.PxCR[0].PxFB;
    ext->RecFIS = (struct Received_FIS *)ext->HBA.PxCR[0].PxFB;

    pr_dbg("PxCR[0].PxFBU: %x, PxCR[0].PxFB: %x\n", ext->HBA.PxCR[0].PxFBU, ext->HBA.PxCR[0].PxFB);
	///>>>>>>>>get Command List Structure address(include a lot of Command Header,the Command Header point to a Command Table)
	//ext->cmdheader = (struct Command_list *)((unsigned long)ext->HBA.PxCR[0].PxCLBU << 32) | ext->HBA.PxCR[0].PxCLB;
	ext->cmdheader = (struct Command_list *)ext->HBA.PxCR[0].PxCLB;
    pr_dbg("PxCR[0].PxCLBU: %x, PxCR[0].PxCLB: %x\n", ext->HBA.PxCR[0].PxCLBU, ext->HBA.PxCR[0].PxCLB);

    /* 映射物理地址 */
    //ffdfc00 -> 0xffdf000

    pr_dbg("map addr %x\n", ((unsigned long) ext->RecFIS) & PAGE_MASK);

    if (__ioremap(((unsigned long) ext->RecFIS) & PAGE_MASK, ((unsigned long) ext->RecFIS) & PAGE_MASK,PAGE_SIZE) < 0) {
        pr_err("ahci device ioremap on %x failed!\n", ((unsigned long) ext->RecFIS) & PAGE_MASK);
        return -1;
    }
    
	///>>>>>>>>alloc Command Table(include CFIS/ACMD/PRDT Structures)
	ext->cmdtbl = (struct Command_Table *)kmalloc(sizeof(struct Command_Table) + sizeof(struct PRDT) * ext->cmdheader->CTBL[0].PRDTL);
	
    pr_dbg("Cmdtable %x\n", ext->cmdtbl);
    
	///>>>>>>>>get PRDT(int Command Table)<<<<<<<<<<<<
	ext->prdt = ext->cmdtbl->CMD_PRDT;

	///detect MSI capability
	index = ext->HBA.PCI_Dev->capability_list;

	while(index != 0)
	{
		value = pci_device_read(pcidev,index);
		if((value & 0xff) == 0x05)
			break;

		index = (value >> 8) & 0xff;
	}

	pr_dbg("Capability ID:%#04x,Pointer:%#04x,%#06x,",value & 0xff,((value >> 8) & 0xff),(value >> 16) & 0xffff);
	pr_dbg("%#010x,",pci_device_read(pcidev,index + 4));
	pr_dbg("%#010x,",pci_device_read(pcidev,index + 8));
	pr_dbg("%#010x\n",pci_device_read(pcidev,index + 12));

	///configuration MSI
	value = 0xfee00000;	///MSI address Lower
	pci_device_write(pcidev,index + 4,value);
	value = 0;		///MSI address Upper
	pci_device_write(pcidev,index + 8,value);
	value = 0x2e;		///MSI data
	pci_device_write(pcidev,index + 12,value);
	
	///MSI control
	///bit 07:64 Bit Address Capable (C64)
	///bit 04-06:Multiple Message Enable (MME)
	///bit 01-03:Multiple Message Capable (MMC)
	///bit 00:MSI Enable (MSIE)
	value = pci_device_read(pcidev,index);
	value = value | 0x10000;
	pci_device_write(pcidev,index,value);

	///register interrupt
	//register_irq(0x2e, NULL , &AHCI_handler, (unsigned long)&AHCI_request, &AHCI_int_controller, "AHCI");
	
    register_irq(IRQ14_HARDDISK, &ahci_handler, IRQF_DISABLED, "ahci", "ahci", (unsigned long) ext);
    register_irq(pcidev->irq_line, &ahci_handler, IRQF_DISABLED, "ahci0", "ahci0", (unsigned long) ext);
    register_irq(IRQ14_HARDDISK + 1, &ahci_handler, IRQF_DISABLED, "ahci1", "ahci1", (unsigned long) ext);
    
	///bit 28-31:Interface Communication Control (ICC)
	///bit 04:FIS Receive Enable (FRE)
	///bit 01:Spin-Up Device (SUD)
	///bit 00:Start (ST)
	ext->HBA.PxCR[0].PxCMD = ext->HBA.PxCR[0].PxCMD | 0x10000003;

	///Port x Serial ATA Error
	value = ext->HBA.PxCR[0].PxSERR;
	if(value)
		ext->HBA.PxCR[0].PxSERR = value;
	
	///Port x Interrupt Status
	value = ext->HBA.PxCR[0].PxIS;
	if(value)
		ext->HBA.PxCR[0].PxIS = value;

	io_mfence();

	///Port x Interrupt Enable
	///bit 05:Descriptor Processed Interrupt Enable (DPE)
	///bit 00:Device to Host Register FIS Interrupt Enable (DHRE)
	ext->HBA.PxCR[0].PxIE = 0x20;

    return 0;
}

static iostatus_t ahci_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *devext;

    int id;
    char devname[DEVICE_NAME_LEN] = {0};

    /* 获取已经配置了的硬盘数量
	这种方法的设备检测需要磁盘安装顺序不能有错误，
	可以用轮训的方式来改变这种情况。 
	 */
	unsigned char disk_foud = *((unsigned char *)AHCI_DISK_NR_ADDR);
	printk(KERN_INFO "ahci_enter: found %d hard disks.\n", disk_foud);

	/* 有磁盘才初始化磁盘 */
	if (disk_foud > 0) {    
        for (id = 0; id < disk_foud; id++) {
            sprintf(devname, "%s%d", DEV_NAME, id);
            /* 初始化一些其它内容 */
            status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_DISK, &devobj);

            if (status != IO_SUCCESS) {
                printk(KERN_ERR "ahci_enter: create device failed!\n");
                return status;
            }
            /* buffered io mode */
            devobj->flags = DO_BUFFERED_IO;

            devext = (device_extension_t *)devobj->device_extension;
            string_new(&devext->device_name, devname, DEVICE_NAME_LEN);
            devext->device_object = devobj;
    #if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "ahci_enter: device extension: device name=%s object=%x\n",
                devext->device_name.text, devext->device_object);
    #endif
            /* 填写设备扩展信息 */
            if (ahci_probe(devext, id) < 0) {
                string_del(&devext->device_name); /* 删除驱动名 */
                io_delete_device(devobj);
                status = IO_FAILED;
                continue;
            }
        }
	}
    
    return status;
}

static iostatus_t ahci_exit(driver_object_t *driver)
{
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    device_extension_t *ext;
    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        ext = devobj->device_extension;
        /* 释放分配的信息缓冲区 */
        /*if (ext->info) {
            kfree(ext->info);
        }*/
        //if (ext->drive == 0) {  /* 通道上第一个磁盘的时候才注销中断 */
            /* 注销中断 */
    		//unregister_irq(ext->channel->irqno, ext->channel);
        //}
        
        io_delete_device(devobj);   /* 删除每一个设备 */
    }

    string_del(&driver->name); /* 删除驱动名 */
    return IO_SUCCESS;
}

iostatus_t ahci_driver_vine(driver_object_t *driver)
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
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "ahci_driver_vine: driver name=%s\n",
        driver->name.text);
#endif
    return status;
}
