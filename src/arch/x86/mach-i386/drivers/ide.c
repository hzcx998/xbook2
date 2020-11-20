#include <xbook/debug.h>
#include <xbook/kernel.h>
#include <const.h>
#include <math.h>
#include <xbook/softirq.h>

#include <xbook/driver.h>
#include <string.h>
#include <xbook/clock.h>
#include <arch/io.h>
#include <xbook/hardirq.h>
#include <arch/cpu.h>
#include <xbook/memalloc.h>
#include <sys/ioctl.h>
#include <stdio.h>

/* 配置开始 */
// #define DEBUG_DRV

/* 配置结束 */

#define DRV_NAME "ide-disk"
#define DRV_VERSION "0.1"

#define DEV_NAME "ide"

/* IDE设备最多的磁盘数量 */
#define MAX_IDE_DISK_NR			4

/* IDE磁盘数在BIOS阶段可以储存到这个地址，直接从里面或取就可以了 */
#define IDE_DISK_NR_ADDR		(KERN_VADDR + 0x0475)

/*8 bit main status registers*/
#define	ATA_STATUS_BUSY		0x80	//Disk busy
#define	ATA_STATUS_READY	0x40	//Disk ready for 
#define	ATA_STATUS_DF		0x20	//Disk write default
#define	ATA_STATUS_SEEK		0x10	//Seek end
#define	ATA_STATUS_DRQ		0x08	//Request data
#define	ATA_STATUS_CORR		0x04	//Correct data
#define	ATA_STATUS_INDEX	0x02	//Receive index
#define	ATA_STATUS_ERR		0x01	//error

/* The Features/Error Port, which returns the most recent error upon read,
 has these possible bit masks */
#define ATA_ERROR_BBK      0x80    // Bad block
#define ATA_ERROR_UNC      0x40    // Uncorrectable data
#define ATA_ERROR_MC       0x20    // Media changed
#define ATA_ERROR_IDNF     0x10    // ID mark not found
#define ATA_ERROR_MCR      0x08    // Media change request
#define ATA_ERROR_ABRT     0x04    // Command aborted
#define ATA_ERROR_TK0NF    0x02    // Track 0 not found
#define ATA_ERROR_AMNF     0x01    // No address mark

/*AT disk controller command*/
#define	ATA_CMD_RESTORE				0x10	//driver retsore
#define ATA_CMD_READ_PIO          	0x20
#define ATA_CMD_READ_PIO_EXT      	0x24
#define ATA_CMD_READ_DMA          	0xC8
#define ATA_CMD_READ_DMA_EXT      	0x25
#define ATA_CMD_WRITE_PIO         	0x30
#define ATA_CMD_WRITE_PIO_EXT     	0x34
#define ATA_CMD_WRITE_DMA         	0xCA
#define ATA_CMD_WRITE_DMA_EXT     	0x35
#define ATA_CMD_CACHE_FLUSH       	0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   	0xEA
#define ATA_CMD_PACKET            	0xA0
#define ATA_CMD_IDENTIFY_PACKET   	0xA1
#define ATA_CMD_IDENTIFY          	0xEC

/* ATAPI的命令 */
#define ATAPI_CMD_READ       0xA8
#define ATAPI_CMD_EJECT      0x1B

/* 设备类型 */
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
 
#define IDE_READ       1
#define IDE_WRITE      2

/* 主设备，从设备 */
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

/* 主通道，从通道 */
#define ATA_PRIMARY   	0x00
#define ATA_SECONDARY   0x01

/* 端口 */
#define	ATA_PRIMARY_PORT	0x1F0
#define	ATA_SECONDARY_PORT	0x170

#define ATA_REG_DATA(channel) 			(channel->base + 0)
#define ATA_REG_FEATURE(channel) 		(channel->base + 1)
#define ATA_REG_ERROR(channel) 			ATA_REG_FEATURE(channel)
#define ATA_REG_SECTOR_CNT(channel) 	(channel->base + 2)
#define ATA_REG_SECTOR_LOW(channel) 	(channel->base + 3)
#define ATA_REG_SECTOR_MID(channel) 	(channel->base + 4)
#define ATA_REG_SECTOR_HIGH(channel) 	(channel->base + 5)
#define ATA_REG_DEVICE(channel) 		(channel->base + 6)
#define ATA_REG_STATUS(channel) 		(channel->base + 7)
#define ATA_REG_CMD(channel) 			ATA_REG_STATUS(channel)

#define ATA_REG_ALT_STATUS(channel) 	(channel->base + 0x206)
#define ATA_REG_CTL(channel) 			ATA_REG_ALT_STATUS(channel)

/* 设备寄存器的位 */
#define BIT_DEV_MBS		0xA0	//bit 7 and 5 are 1

/* 生存ATA设备寄存器的值 */
#define ATA_MKDEV_REG(lbaMode, slave, head) (BIT_DEV_MBS | \
		0x40 | \
		(slave << 4) | \
		head)

/* IDE通道结构体 */
struct ide_channel {
   	unsigned short base;    // I/O Base.
	char irqno;		 	// 本通道所用的中断号
   	struct _device_extension *ext;	// 通道上面的设备
	char who;		    /* 通道上主磁盘在活动还是从磁盘在活动 */
	char what;		    /* 执行的是什么操作 */
} channels[2];

typedef struct _device_extension {
    string_t device_name;           /* 设备名字 */
    device_object_t *device_object; /* 设备对象 */

    struct ide_channel *channel;		/* 所在的通道 */
	struct ide_identy *info;	/* 磁盘信息 */
	unsigned char reserved;	// disk exist
	unsigned char drive;	// 0 (Master Drive) or 1 (Slave Drive).
	unsigned char type;		// 0: ATA, 1:ATAPI.
	unsigned char signature;// Drive Signature
	unsigned int capabilities;// Features.
	unsigned int command_sets; // Command Sets Supported.
	unsigned int size;		// Size in Sectors.

    unsigned long rwoffset; // 读写偏移位置
	/* 状态信息 */
	unsigned int read_sectors;	// 读取了多少扇区
	unsigned int write_sectors;	// 写入了多少扇区
} device_extension_t;

/* 磁盘信息结构体 */
struct ide_identy {
	//	0	General configuration bit-significant information
	unsigned short General_Config;

	//	1	Obsolete
	unsigned short Obsolete0;

	//	2	Specific configuration
	unsigned short Specific_Coinfig;

	//	3	Obsolete
	unsigned short Obsolete1;

	//	4-5	Retired
	unsigned short Retired0[2];

	//	6	Obsolete
	unsigned short Obsolete2;

	//	7-8	Reserved for the CompactFlash Association
	unsigned short CompactFlash[2];

	//	9	Retired
	unsigned short Retired1;

	//	10-19	Serial number (20 ASCII characters)
	unsigned short Serial_Number[10];

	//	20-21	Retired
	unsigned short Retired2[2];

	//	22	Obsolete
	unsigned short Obsolete3;

	//	23-26	Firmware revision(8 ASCII characters)
	unsigned short Firmware_Version[4];

	//	27-46	Model number (40 ASCII characters)
	unsigned short Model_Number[20];

	//	47	15:8 	80h 
	//		7:0  	00h=Reserved 
	//			01h-FFh = Maximumnumber of logical sectors that shall be transferred per DRQ data block on READ/WRITE MULTIPLE commands
	unsigned short Max_logical_transferred_per_DRQ;

	//	48	Trusted Computing feature set options
	unsigned short Trusted_Computing_feature_set_options;

	//	49	Capabilities
	unsigned short Capabilities0;

	//	50	Capabilities
	unsigned short Capabilities1;

	//	51-52	Obsolete
	unsigned short Obsolete4[2];

	//	53	15:8	Free-fall Control Sensitivity
	//		7:3 	Reserved
	//		2 	the fields reported in word 88 are valid
	//		1 	the fields reported in words (70:64) are valid
	unsigned short Report_88_70to64_valid;

	//	54-58	Obsolete
	unsigned short Obsolete5[5];

	//	59	15:9	Reserved
	//		8	Multiple sector setting is valid	
	//		7:0	xxh current setting for number of logical sectors that shall be transferred per DRQ data block on READ/WRITE Multiple commands
	unsigned short Mul_Sec_Setting_Valid;

	//	60-61	Total number of user addresssable logical sectors for 28bit CMD
	unsigned short lba28Sectors[2];

	//	62	Obsolete
	unsigned short Obsolete6;

	//	63	15:11	Reserved
	//		10:8=1 	Multiword DMA mode 210 is selected
	//		7:3 	Reserved
	//		2:0=1 	Multiword DMA mode 210 and below are supported
	unsigned short MultWord_DMA_Select;

	//	64	15:8	Reserved
	//		7:0	PIO mdoes supported
	unsigned short PIO_mode_supported;

	//	65	Minimum Multiword DMA transfer cycle time per word
	unsigned short Min_MulWord_DMA_cycle_time_per_word;

	//	66	Manufacturer`s recommended Multiword DMA transfer cycle time
	unsigned short Manufacture_Recommend_MulWord_DMA_cycle_time;

	//	67	Minimum PIO transfer cycle time without flow control
	unsigned short Min_PIO_cycle_time_Flow_Control;

	//	68	Minimum PIO transfer cycle time with IORDY flow control
	unsigned short Min_PIO_cycle_time_IOREDY_Flow_Control;

	//	69-70	Reserved
	unsigned short Reserved1[2];

	//	71-74	Reserved for the IDENTIFY PACKET DEVICE command
	unsigned short Reserved2[4];

	//	75	Queue depth
	unsigned short Queue_depth;

	//	76	Serial ATA Capabilities 
	unsigned short SATA_Capabilities;

	//	77	Reserved for Serial ATA 
	unsigned short Reserved3;

	//	78	Serial ATA features Supported 
	unsigned short SATA_features_Supported;

	//	79	Serial ATA features enabled
	unsigned short SATA_features_enabled;

	//	80	Major Version number
	unsigned short Major_Version;

	//	81	Minor version number
	unsigned short Minor_Version;

	//	82	Commands and feature sets supported
	unsigned short cmdSet0;

	//	83	Commands and feature sets supported	
	unsigned short cmdSet1;

	//	84	Commands and feature sets supported
	unsigned short Cmd_feature_sets_supported2;

	//	85	Commands and feature sets supported or enabled
	unsigned short Cmd_feature_sets_supported3;

	//	86	Commands and feature sets supported or enabled
	unsigned short Cmd_feature_sets_supported4;

	//	87	Commands and feature sets supported or enabled
	unsigned short Cmd_feature_sets_supported5;

	//	88	15 	Reserved 
	//		14:8=1 	Ultra DMA mode 6543210 is selected 
	//		7 	Reserved 
	//		6:0=1 	Ultra DMA mode 6543210 and below are suported
	unsigned short Ultra_DMA_modes;

	//	89	Time required for Normal Erase mode SECURITY ERASE UNIT command
	unsigned short Time_required_Erase_CMD;

	//	90	Time required for an Enhanced Erase mode SECURITY ERASE UNIT command
	unsigned short Time_required_Enhanced_CMD;

	//	91	Current APM level value
	unsigned short Current_APM_level_Value;

	//	92	Master Password Identifier
	unsigned short Master_Password_Identifier;

	//	93	Hardware resset result.The contents of bits (12:0) of this word shall change only during the execution of a hardware reset.
	unsigned short HardWare_Reset_Result;

	//	94	Current AAM value 
	//		15:8 	Vendor’s recommended AAM value 
	//		7:0 	Current AAM value
	unsigned short Current_AAM_value;

	//	95	Stream Minimum Request Size
	unsigned short Stream_Min_Request_Size;

	//	96	Streaming Transger Time-DMA 
	unsigned short Streaming_Transger_time_DMA;

	//	97	Streaming Access Latency-DMA and PIO
	unsigned short Streaming_Access_Latency_DMA_PIO;

	//	98-99	Streaming Performance Granularity (DWord)
	unsigned short Streaming_Performance_Granularity[2];

	//	100-103	Total Number of User Addressable Logical Sectors for 48-bit commands (QWord)
	unsigned short lba48Sectors[4];

	//	104	Streaming Transger Time-PIO
	unsigned short Streaming_Transfer_Time_PIO;

	//	105	Reserved
	unsigned short Reserved4;

	//	106	Physical Sector size/Logical Sector Size
	unsigned short Physical_Logical_Sector_Size;

	//	107	Inter-seek delay for ISO-7779 acoustic testing in microseconds
	unsigned short Inter_seek_delay;

	//	108-111	World wide name	
	unsigned short World_wide_name[4];

	//	112-115	Reserved
	unsigned short Reserved5[4];

	//	116	Reserved for TLC
	unsigned short Reserved6;

	//	117-118	Logical sector size (DWord)
	unsigned short Words_per_Logical_Sector[2];

	//	119	Commands and feature sets supported (Continued from words 84:82)
	unsigned short CMD_feature_Supported;

	//	120	Commands and feature sets supported or enabled (Continued from words 87:85)
	unsigned short CMD_feature_Supported_enabled;

	//	121-126	Reserved for expanded supported and enabled settings
	unsigned short Reserved7[6];

	//	127	Obsolete
	unsigned short Obsolete7;

	//	128	Security status
	unsigned short Security_Status;

	//	129-159	Vendor specific
	unsigned short Vendor_Specific[31];

	//	160	CFA power mode
	unsigned short CFA_Power_mode;

	//	161-167	Reserved for the CompactFlash Association
	unsigned short Reserved8[7];

	//	168	Device Nominal Form Factor
	unsigned short Dev_from_Factor;

	//	169-175	Reserved
	unsigned short Reserved9[7];

	//	176-205	Current media serial number (ATA string)
	unsigned short Current_Media_Serial_Number[30];

	//	206	SCT Command Transport
	unsigned short SCT_Cmd_Transport;

	//	207-208	Reserved for CE-ATA
	unsigned short Reserved10[2];

	//	209	Alignment of logical blocks within a physical block
	unsigned short Alignment_Logical_blocks_within_a_physical_block;

	//	210-211	Write-Read-Verify Sector Count Mode 3 (DWord)
	unsigned short Write_Read_Verify_Sector_Count_Mode_3[2];

	//	212-213	Write-Read-Verify Sector Count Mode 2 (DWord)
	unsigned short Write_Read_Verify_Sector_Count_Mode_2[2];

	//	214	NV Cache Capabilities
	unsigned short NV_Cache_Capabilities;

	//	215-216	NV Cache Size in Logical Blocks (DWord)
	unsigned short NV_Cache_Size[2];

	//	217	Nominal media rotation rate
	unsigned short Nominal_media_rotation_rate;

	//	218	Reserved
	unsigned short Reserved11;

	//	219	NV Cache Options
	unsigned short NV_Cache_Options;

	//	220	Write-Read-Verify feature set current mode
	unsigned short Write_Read_Verify_feature_set_current_mode;

	//	221	Reserved
	unsigned short Reserved12;

	//	222	Transport major version number. 
	//		0000h or ffffh = device does not report version
	unsigned short Transport_Major_Version_Number;

	//	223	Transport Minor version number
	unsigned short Transport_Minor_Version_Number;

	//	224-233	Reserved for CE-ATA
	unsigned short Reserved13[10];

	//	234	Minimum number of 512-byte data blocks per DOWNLOAD MICROCODE command for mode 03h
	unsigned short Mini_blocks_per_CMD;

	//	235	Maximum number of 512-byte data blocks per DOWNLOAD MICROCODE command for mode 03h
	unsigned short Max_blocks_per_CMD;

	//	236-254	Reserved
	unsigned short Reserved14[19];

	//	255	Integrity word
	//		15:8	Checksum
	//		7:0	Checksum Validity Indicator
	unsigned short Integrity_word;
}__attribute__((packed));

/**
 * ide_print_error - 打印错误
 * @dev: 设备
 * @err: 错误码
 * 
 * 根据错误码打印对应的错误
 */
static unsigned char ide_print_error(device_extension_t *ext, unsigned char err)
{
   	if (err == 0)
      	return err;
	
   	printk("IDE:");
   	if (err == 1) {printk("- Device Fault\n     ");}
   	else if (err == 2) {	/* 其它错误 */
		unsigned char state = in8(ATA_REG_ERROR(ext->channel));
		if (state & ATA_ERROR_AMNF)	{printk("- No Address Mark Found\n     ");}
		if (state & ATA_ERROR_TK0NF){printk("- No Media or Media Error\n     ");}
		if (state & ATA_ERROR_ABRT)	{printk("- Command Aborted\n     ");      	}
		if (state & ATA_ERROR_MCR) 	{printk("- No Media or Media Error\n     "); }
		if (state & ATA_ERROR_IDNF) {printk("- ID mark not Found\n     ");      }
		if (state & ATA_ERROR_MC) 	{printk("- No Media or Media Error\n     "); }
		if (state & ATA_ERROR_UNC)  {printk("- Uncorrectable Data Error\n     ");}
		if (state & ATA_ERROR_BBK)  {printk("- Bad Sectors\n     "); }
	} else  if (err == 3) {	/* 读取错误 */
		printk("- Reads Nothing\n     ");
	} else if (err == 4)  {	/* 写错误 */
		printk("- Write Protected\n     ");
	} else if (err == 5)  {	/* 超时 */
		printk("- Time Out\n     ");
	}
	printk("- [%s %s]\n",
		(const char *[]){"Primary", "Secondary"}[ext->channel - channels], // Use the channel as an index into the array
		(const char *[]){"Master", "Slave"}[ext->drive] // Same as above, using the drive
		);

   return err;
}
#ifdef DEBUG_DRV
static void dump_ide_channel(struct ide_channel *channel)
{
	printk(KERN_DEBUG "dump_ide_channel: ext:%x base:%x irq:%d\n", channel, channel->base, channel->irqno);
}

static void dump_ide_extension(device_extension_t *ext)
{
    printk(KERN_DEBUG "dump_ide_extension:  ======================= start\n");
	printk(KERN_DEBUG "ext:%x channel:%x drive:%d type:%s \n",
	 	ext, ext->channel, ext->drive,
		ext->type == IDE_ATA ? "ATA" : "ATAPI");

	printk(KERN_DEBUG "capabilities:%x command_sets:%x signature:%x\n",
		ext->capabilities, ext->command_sets, ext->signature);
	
	if (ext->info->cmdSet1 & 0x0400) {
		printk(KERN_DEBUG "Total Sector(LBA 48):");
	} else {
		printk(KERN_DEBUG "Total Sector(LBA 28):");
	}
	printk("%d\n", ext->size);
    
	printk(KERN_DEBUG "Serial Number:");
	
	int i;
	for (i = 0; i < 10; i++) {
		printk("%c%c", (ext->info->Serial_Number[i] >> 8) & 0xff,
			ext->info->Serial_Number[i] & 0xff);
	}
    printk("\n");
	printk(KERN_DEBUG "Fireware Version:");
	for (i = 0; i < 4; i++) {
		printk("%c%c", (ext->info->Firmware_Version[i] >> 8) & 0xff,
			ext->info->Firmware_Version[i] & 0xff);
	}
    printk("\n");
	printk(KERN_DEBUG "Model Number:");
	for (i = 0; i < 20; i++) {
		printk("%c%c", (ext->info->Model_Number[i] >> 8) & 0xff,
			ext->info->Model_Number[i] & 0xff);
	}
    printk("\n");
	printk(KERN_DEBUG "LBA supported:%s ",(ext->info->Capabilities0 & 0x0200) ? "Yes" : "No");
	printk(KERN_DEBUG "LBA48 supported:%s\n",((ext->info->cmdSet1 & 0x0400) ? "Yes" : "No"));
    printk(KERN_DEBUG "dump_ide_extension: ======================= end \n");
}
#endif

/**
 * read_from_sector - 硬盘读入count个扇区的数据到buf
 * @dev: 设备
 * @buf: 缓冲区
 * @count: 扇区数 
 */
static void read_from_sector(device_extension_t *ext, void* buf, unsigned int count) {
	unsigned int size_in_byte;
	if (count == 0) {
	/* 因为sec_cnt是8位变量,由主调函数将其赋值时,若为256则会将最高位的1丢掉变为0 */
		size_in_byte = 256 * SECTOR_SIZE;
	} else { 
		size_in_byte = count * SECTOR_SIZE; 
	}
	io_read(ATA_REG_DATA(ext->channel), buf, size_in_byte);
}

/**
 * write_to_sector - 将buf中count扇区的数据写入硬盘
 * @dev: 设备
 * @buf: 缓冲区
 * @count: 扇区数 
 */
static void write_to_sector(device_extension_t *ext, void* buf, unsigned int count)
{
   unsigned int size_in_byte;
	if (count == 0) {
	/* 因为sec_cnt是8位变量,由主调函数将其赋值时,若为256则会将最高位的1丢掉变为0 */
		size_in_byte = 256 * 512;
	} else { 
		size_in_byte = count * 512; 
	}
   	io_write(ATA_REG_DATA(ext->channel), buf, size_in_byte);
}

/**
 * send_cmd - 向通道channel发命令cmd 
 * @channel: 要发送命令的通道
 * @cmd: 命令
 * 
 * 执行一个命令
 */
static void send_cmd(struct ide_channel* channel, unsigned char cmd)
{
   	out8(ATA_REG_CMD(channel), cmd);
}


/**
 * busy_wait - 忙等待
 * @dev: 等待的设备
 * 
 * 返回信息：
 * 0表示等待无误
 * 1表示设备故障
 * 2表示状态出错
 * 5表示超时
 */
static unsigned char busy_wait(device_extension_t *ext) {
	struct ide_channel* channel = ext->channel;
	/* 等待1秒
	1000 * 1000 纳秒
	*/
   	unsigned int time_limit = 10 * 1000 * 1000;
    // unsigned int time_limit = 1000;

   	while (--time_limit >= 0) {
      	if (!(in8(ATA_REG_STATUS(channel)) & ATA_STATUS_BUSY)) {
			
			unsigned char state = in8(ATA_REG_STATUS(channel));
			if (state & ATA_STATUS_ERR)
				return 2; // Error.
			
			if (state & ATA_STATUS_DF)
				return 1; // Device Fault.

			/* 数据请求，等待完成 */
			if (state & ATA_STATUS_DRQ) {
				return 0;
			}
      	} else {
			/* 读取一次耗费100ns */
			in8(ATA_REG_ALT_STATUS(channel));
      	}
   	}
   	return 5;	// time out
}

/**
 * ide_polling - 轮询操作
 * @chanel: 通道
 * @advanced_check: 高级状态检测
 */
static int ide_polling(struct ide_channel* channel, unsigned int advanced_check)
{
	// (I) Delay 400 nanosecond for BSY to be set:
	// -------------------------------------------------
	int i;
	for(i = 0; i < 4; i++) {
		/* Reading the Alternate Status port wastes 100ns;
		 loop four times.
		*/
		in8(ATA_REG_ALT_STATUS(channel));
 	}
	// (II) Wait for BSY to be cleared:
	// -------------------------------------------------
	/* time */
    i = 0x1000;
    while ((in8(ATA_REG_STATUS(channel)) & ATA_STATUS_BUSY) && (--i)); // Wait for BSY to be zero.
	
    if (advanced_check) {
		unsigned char state = in8(ATA_REG_STATUS(channel)); // Read Status Register.

		// (III) Check For Errors:
		// -------------------------------------------------
		if (state & ATA_STATUS_ERR) {
			return 2; // Error.
		}

		// (IV) Check If Device fault:
		// -------------------------------------------------
		if (state & ATA_STATUS_DF)
			return 1; // Device Fault.
	
		// (V) Check DRQ:
		// -------------------------------------------------
		// BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
		if ((state & ATA_STATUS_DRQ) == 0)
			return 3; // DRQ should be set
	}
	
	return 0; // No Error.
}

/**
 * select_addr_mode - 选择寻址模式
 * @dev: 设备
 * @lba: 逻辑扇区地址
 * @mode: 传输模式（CHS和LBA模式）
 * @head: CHS的头或者LBA28的高4位
 * @io: lba地址包，又6字节组成
 * 
 * 根据lba选择寻址模式，并生成IO地址包
 */
static void select_addr_mode(device_extension_t *ext,
	unsigned int lba,
	unsigned char *mode,
	unsigned char *head,
	unsigned char *io)
{
	unsigned short cyl;
   	unsigned char sect;
	if (lba >= 0x10000000 && ext->capabilities & 0x200) { // Sure Drive should support LBA in this case, or you are
								// giving a wrong LBA.
		// LBA48:
		*mode  = 2;
		io[0] = (lba & 0x000000FF) >> 0;
		io[1] = (lba & 0x0000FF00) >> 8;
		io[2] = (lba & 0x00FF0000) >> 16;
		io[3] = (lba & 0xFF000000) >> 24;
		io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
		io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
		*head      = 0; // Lower 4-bits of HDDEVSEL are not used here.
	} else if (ext->capabilities & 0x200)  { // Drive supports LBA?
		// LBA28:
		*mode  = 1;
		io[0] = (lba & 0x00000FF) >> 0;
		io[1] = (lba & 0x000FF00) >> 8;
		io[2] = (lba & 0x0FF0000) >> 16;
		io[3] = 0; // These Registers are not used here.
		io[4] = 0; // These Registers are not used here.
		io[5] = 0; // These Registers are not used here.
		*head      = (lba & 0xF000000) >> 24;
	} else {
		// CHS:
		*mode  = 0;
		sect      = (lba % 63) + 1;
		cyl       = (lba + 1  - sect) / (16 * 63);
		io[0] = sect;
		io[1] = (cyl >> 0) & 0xFF;
		io[2] = (cyl >> 8) & 0xFF;
		io[3] = 0;
		io[4] = 0;
		io[5] = 0;
		*head      = (lba + 1  - sect) % (16 * 63) / (63); // Head number is written to HDDEVSEL lower 4-bits.
	}
}

/** 
 * select_disk - 选择操作的磁盘
 * @dev: 设备
 * @mode: 操作模式，0表示CHS模式，非0表示LBA模式
 */
static void select_disk(device_extension_t *ext,
	unsigned char mode, 
	unsigned char head)
{
   	out8(ATA_REG_DEVICE(ext->channel),
	   ATA_MKDEV_REG(mode == 0 ? 0 : 1, ext->drive, head));

	ext->channel->who = ext->drive;	/* 通道上当前的磁盘 */
}

/**
 * select_sector - 选择扇区数，扇区位置
 * @dev: 设备
 * @mode: 传输模式（CHS和LBA模式）
 * @lbaIO: lba地址包，又6字节组成
 * @count: 扇区数
 * 
 * 向硬盘控制器写入起始扇区地址及要读写的扇区数
 */
static void select_sector(device_extension_t *ext,
	unsigned char mode,
	unsigned char *lbaIO,
	sector_t count)
{
  	struct ide_channel* channel = ext->channel;

	/* 如果是LBA48就要写入24高端字节 */
	if (mode == 2) {
		out8(ATA_REG_FEATURE(channel), 0); // PIO mode.

		/* 写入要读写的扇区数*/
		out8(ATA_REG_SECTOR_CNT(channel), 0);

		/* 写入lba地址24~47位(即扇区号) */
		out8(ATA_REG_SECTOR_LOW(channel), lbaIO[3]);
		out8(ATA_REG_SECTOR_MID(channel), lbaIO[4]);
		out8(ATA_REG_SECTOR_HIGH(channel), lbaIO[5]);
	}

	out8(ATA_REG_FEATURE(channel), 0); // PIO mode.

	/* 写入要读写的扇区数*/
	out8(ATA_REG_SECTOR_CNT(channel), count);

	/* 写入lba地址0~23位(即扇区号) */
	out8(ATA_REG_SECTOR_LOW(channel), lbaIO[0]);
	out8(ATA_REG_SECTOR_MID(channel), lbaIO[1]);
	out8(ATA_REG_SECTOR_HIGH(channel), lbaIO[2]);
}

/**
 * select_cmd - 选择要执行的命令
 * @rw: 传输方向（读，写）
 * @mode: 传输模式（CHS和LBA模式）
 * @dma: 直接内存传输
 * @cmd: 保存要执行的命令
 * 
 * 根据模式，dma格式还有传输方向选择一个命令
 */
static void select_cmd(unsigned char rw,
	unsigned char mode,
	unsigned char dma,
	unsigned char *cmd)
{
	// Routine that is followed:
	// If ( DMA & LBA48)   DO_DMA_EXT;
	// If ( DMA & LBA28)   DO_DMA_LBA;
	// If ( DMA & LBA28)   DO_DMA_CHS;
	// If (!DMA & LBA48)   DO_PIO_EXT;
	// If (!DMA & LBA28)   DO_PIO_LBA;
	// If (!DMA & !LBA#)   DO_PIO_CHS;
	if (mode == 0 && dma == 0 && rw == IDE_READ) *cmd = ATA_CMD_READ_PIO;
	if (mode == 1 && dma == 0 && rw == IDE_READ) *cmd = ATA_CMD_READ_PIO;   
	if (mode == 2 && dma == 0 && rw == IDE_READ) *cmd = ATA_CMD_READ_PIO_EXT;   
	if (mode == 0 && dma == 1 && rw == IDE_READ) *cmd = ATA_CMD_READ_DMA;
	if (mode == 1 && dma == 1 && rw == IDE_READ) *cmd = ATA_CMD_READ_DMA;
	if (mode == 2 && dma == 1 && rw == IDE_READ) *cmd = ATA_CMD_READ_DMA_EXT;
	if (mode == 0 && dma == 0 && rw == IDE_WRITE) *cmd = ATA_CMD_WRITE_PIO;
	if (mode == 1 && dma == 0 && rw == IDE_WRITE) *cmd = ATA_CMD_WRITE_PIO;
	if (mode == 2 && dma == 0 && rw == IDE_WRITE) *cmd = ATA_CMD_WRITE_PIO_EXT;
	if (mode == 0 && dma == 1 && rw == IDE_WRITE) *cmd = ATA_CMD_WRITE_DMA;
	if (mode == 1 && dma == 1 && rw == IDE_WRITE) *cmd = ATA_CMD_WRITE_DMA;
	if (mode == 2 && dma == 1 && rw == IDE_WRITE) *cmd = ATA_CMD_WRITE_DMA_EXT;

}

/**
 * rest_driver - 重置驱动
 * @channel: 通道
 * 
 */
static void driver_soft_rest(struct ide_channel *channel)
{
	char ctrl = in8(ATA_REG_CTL(channel));
	//printk("ctrl = %x\n", ctrl);
	
	/* 写入控制寄存器，执行重置，会重置ata通道上的2个磁盘
	把SRST（位2）置1，就可以软重置
	 */
	out8(ATA_REG_CTL(channel), ctrl | (1 << 2));

	/* 等待重置 */
	int i;
	/* 每次读取用100ns，读取50次，就花费5000ns，也就是5us */
	for(i = 0; i < 50; i++) {
		/* Reading the Alternate Status port wastes 100ns;
		 loop four times.
		*/
		in8(ATA_REG_ALT_STATUS(channel));
 	}
	
	/* 重置完后，需要清除该位，还原状态就行 */
	out8(ATA_REG_CTL(channel), ctrl);
}

/**
 * rest_driver - 重置驱动器
 * @dev: 设备
 */
static void rest_driver(device_extension_t *ext)
{

	driver_soft_rest(ext->channel);
}
/**
 * pio_data_transfer - PIO数据传输
 * @dev: 设备
 * @rw: 传输方向（读，写）
 * @mode: 传输模式（CHS和LBA模式）
 * @buf: 扇区缓冲
 * @count: 扇区数
 * 
 * 传输成功返回0，失败返回非0
 */
static int pio_data_transfer(device_extension_t *ext,
	unsigned char rw,
	unsigned char mode,
	unsigned char *buf,
	unsigned short count)
{
	volatile short i = 0; // 不允许被优化
	unsigned char error;
	if (rw == IDE_READ) {	
#ifdef DEBUG_DRV
        printk("PIO read count %d->", count);
#endif
		while (i < count) {
			/* 醒来后开始执行下面代码*/
			if ((error = busy_wait(ext))) {     //  若失败
                printk(KERN_ERR "wait driver failed!\n");
				/* 重置磁盘驱动并返回 */
				rest_driver(ext);
				return error;
			}
			read_from_sector(ext, buf, 1);
            ++i;
			buf += SECTOR_SIZE;
		}
	} else {
#ifdef DEBUG_DRV
        printk("PIO write->");
#endif
		while (i < count) {
			/* 等待硬盘控制器请求数据 */
			if ((error = busy_wait(ext))) {     //  若失败
				/* 重置磁盘驱动并返回 */
				rest_driver(ext);
				return error;
			}
			/* 把数据写入端口，完成1个扇区后会产生一次中断 */
			write_to_sector(ext, buf, 1);
            ++i;
			buf += SECTOR_SIZE;
            //printk("write success! ");
		}
		/* 刷新写缓冲区 */
		out8(ATA_REG_CMD(ext->channel), mode > 1 ?
			ATA_CMD_CACHE_FLUSH_EXT : ATA_CMD_CACHE_FLUSH);
		ide_polling(ext->channel, 0);
	}
#ifdef DEBUG_DRV
        printk("PIO read done\n");
#endif
	return 0;
}

/**
 * ata_type_transfer - ATA类型数据传输
 * @dev: 设备
 * @rw: 传输方向（读，写）
 * @lba: 逻辑扇区地址
 * @count: 扇区数
 * @buf: 扇区缓冲
 * 
 * 传输成功返回0，失败返回非0
 */
static int ata_type_transfer(device_extension_t *ext,
	unsigned char rw,
	unsigned int lba,
	unsigned int count,
	void *buf)
{
	unsigned char mode;	/* 0: CHS, 1:LBA28, 2: LBA48 */
	unsigned char dma; /* 0: No DMA, 1: DMA */
	unsigned char cmd = 0;	
	unsigned char *_buf = (unsigned char *)buf;

	unsigned char lbaIO[6];	/* 由于最大是48位，所以这里数组的长度为6 */

	struct ide_channel *channel = ext->channel;

   	unsigned char head, err;

	/* 要去操作的扇区数 */
	unsigned int todo;
	/* 已经完成的扇区数 */
	unsigned int done = 0;
	
    /* 同步锁加锁 */
	//SyncLock(&channel->lock);

	/* 保存读写操作 */
	channel->what = rw;
	
	while (done < count) {
		/* 获取要去操作的扇区数
		由于一次最大只能操作256个扇区，这里用256作为分界
		 */
		if ((done + 256) <= count) {
			todo = 256;
		} else {
			todo = count - done;
		}

		/* 选择传输模式（PIO或DMA） */
		dma = 0; // We don't support DMA

		/* 选择寻址模式 */
		// (I) Select one from LBA28, LBA48 or CHS;
		select_addr_mode(ext, lba + done, &mode, &head, lbaIO);

		/* 等待驱动不繁忙 */
		// (III) Wait if the drive is busy;
		while (in8(ATA_REG_STATUS(channel)) & ATA_STATUS_BUSY) cpu_idle();// Wait if busy.
		/* 从控制器中选择设备 */
		select_disk(ext, mode, head);

		/* 填写参数，扇区和扇区数 */
		select_sector(ext, mode, lbaIO, count);

		/* 等待磁盘控制器处于准备状态 */
		while (!(in8(ATA_REG_STATUS(channel)) & ATA_STATUS_READY)) cpu_idle();

		/* 选择并发送命令 */
		select_cmd(rw, mode, dma, &cmd);

#ifdef DEBUG_DRV
			printk("lba mode %d num %d io %d %d %d %d %d %d->",
				mode, lba, lbaIO[0], lbaIO[1], lbaIO[2], lbaIO[3], lbaIO[4], lbaIO[5]);
			printk("rw %d dma %d cmd %x head %d\n",
				rw, dma, cmd, head);
#endif
		/* 等待磁盘控制器处于准备状态 */
		while (!(in8(ATA_REG_STATUS(channel)) & ATA_STATUS_READY)) cpu_idle();

		/* 发送命令 */
		send_cmd(channel, cmd);

		/* 根据不同的模式传输数据 */
		if (dma) {	/* DMA模式 */
			if (rw == IDE_READ) {
				// DMA Read.
			} else {
				// DMA Write.
			}
		} else {
			/* PIO模式数据传输 */
			if ((err = pio_data_transfer(ext, rw, mode, _buf, todo))) {
				return err;
			}
			_buf += todo * SECTOR_SIZE;
			done += todo;
		}
	}

	//SyncUnlock(&channel->lock);
	return 0;
}

/**
 * ide_read_sector - 读扇区
 * @dev: 设备
 * @lba: 逻辑扇区地址
 * @count: 扇区数
 * @buf: 扇区缓冲
 * 
 * 数据读取磁盘，成功返回0，失败返回-1
 */
static int ide_read_sector(device_extension_t *ext,
	unsigned int lba,
	void *buf,
	unsigned int count)
{
	unsigned char error;
	/* 检查设备是否正确 */
	if (lba + count > ext->size && ext->type == IDE_ATA) {
        printk(KERN_ERR "ide_read_sector: out of range!\n");
		return -1;
	} else {

		/* 进行磁盘访问 */

		/*如果类型是ATA*/
        error = ata_type_transfer(ext, IDE_READ, lba, count, buf);
		/*如果类型是ATAPI*/
		
		/* 打印驱动错误信息 */
		if(ide_print_error(ext, error)) {
			printk(KERN_ERR "ide_read_sector: ide read error!\n");
            return -1;
		}
	}
	return 0;
}

/**
 * ide_write_sector - 写扇区
 * @dev: 设备
 * @lba: 逻辑扇区地址
 * @count: 扇区数
 * @buf: 扇区缓冲
 * 
 * 把数据写入磁盘，成功返回0，失败返回-1
 */
static int ide_write_sector(
    device_extension_t *ext,
	unsigned int lba,
	void *buf,
	unsigned int count
) {
	unsigned char error;
	if (lba + count > ext->size && ext->type == IDE_ATA) {
		return -1;
	} else {
		/* 进行磁盘访问，如果出错，就 */
		
		/*如果类型是ATA*/
			error = ata_type_transfer(ext, IDE_WRITE, lba, count, buf);
		/*如果类型是ATAPI*/

		/* 打印驱动错误信息 */
		if(ide_print_error(ext, error)) {
			return -1;
		}
	}
	return 0;
}

/**
 * ide_clean_disk - 清空磁盘的数据
 * @dev: 设备
 * @count: 清空的扇区数
 * 
 * 当count为0时，表示清空整个磁盘，不为0时就清空对于的扇区数
 */
static int ide_clean_disk(device_extension_t *ext, sector_t count)
{
	if (count == 0)
		count = ext->size;

	sector_t todo, done = 0;

	/* 每次写入10个扇区 */
	char *buffer = mem_alloc(SECTOR_SIZE *10);
	if (!buffer) {
		printk("mem_alloc for ide buf failed!\n");
		return -1;
	}

	memset(buffer, 0, SECTOR_SIZE *10);

	printk(KERN_DEBUG "ide_clean_disk: count%d\n", count);
	while (done < count) {
		/* 获取要去操作的扇区数这里用10作为分界 */
		if ((done + 10) <= count) {
			todo = 10;
		} else {
			todo = count - done;
		}
		//printk(KERN_DEBUG "ide_clean_disk: done %d todo %d\n", done, todo);
		ide_write_sector(ext, done, buffer, todo);
		done += 10;
	}
	return 0;
}

iostatus_t ide_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;
    unsigned long arg = ioreq->parame.devctl.arg;
    unsigned long off;
    device_extension_t *ext = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    int infomation = 0;
    switch (ctlcode)
    {
    case DISKIO_GETSIZE:
        *((unsigned int *) arg) = ext->size;
        break;
    case DISKIO_CLEAR:
        ide_clean_disk(device->device_extension, arg);
        break;
    case DISKIO_SETOFF:
        off = *((unsigned long *) arg);
        if (off > ext->size - 1)
            off = ext->size - 1;
        ext->rwoffset = off;
        break;
    case DISKIO_GETOFF:
        *((unsigned long *) arg) = ext->rwoffset;
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

iostatus_t ide_read(device_object_t *device, io_request_t *ioreq)
{
    long len;
    iostatus_t status = IO_SUCCESS;
    sector_t sectors = DIV_ROUND_UP(ioreq->parame.read.length, SECTOR_SIZE);
    device_extension_t *ext = device->device_extension;

#ifdef DEBUG_DRV
    printk(KERN_DEBUG "ide_read: buf=%x sectors=%d off=%x\n", 
        ioreq->system_buffer, sectors, ioreq->parame.read.offset);
#endif
    unsigned long off;    
    if (ioreq->parame.read.offset == DISKOFF_MAX) {
        off = ext->rwoffset;
    } else {
        off = ioreq->parame.read.offset;
    }
    len = ide_read_sector(device->device_extension, off,
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

iostatus_t ide_write(device_object_t *device, io_request_t *ioreq)
{
    long len;
    iostatus_t status = IO_SUCCESS;
    sector_t sectors = DIV_ROUND_UP(ioreq->parame.write.length, SECTOR_SIZE);
    device_extension_t *ext = device->device_extension;

#ifdef DEBUG_DRV
    printk(KERN_DEBUG "ide_write: buf=%x sectors=%d off=%x\n", 
        ioreq->system_buffer, sectors, ioreq->parame.write.offset);
#endif    
    unsigned long off;    
    if (ioreq->parame.write.offset == DISKOFF_MAX) {
        off = ext->rwoffset;
    } else {
        off = ioreq->parame.write.offset;
    }
    len = ide_write_sector(device->device_extension, off,
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

static int ide_handler(irqno_t irq, void *data)
{
    struct ide_channel *channel = (struct ide_channel *)data;
	device_extension_t *ext = channel->ext + channel->who;

    
	/* 获取状态，做出错判断 */
	if (in8(ATA_REG_STATUS(channel)) & ATA_STATUS_ERR) {
		/* 尝试重置驱动 */
		rest_driver(ext);
	}

	/* 可以在这里面做一些统计信息 */
	if (channel->what == IDE_READ) {
		ext->read_sectors++;
	} else if (channel->what == IDE_WRITE) {
		ext->write_sectors++;
	}
    return 0;
}

static int ide_probe(device_extension_t *ext, int id)
{
	/* 初始化，并获取信息 */
	int channelno = id / 2;	   // 一个ide通道上有两个硬盘,根据硬盘数量反推有几个ide通道
	int diskno = id % 2;
	struct ide_channel* channel;
	char err;
    
    channel = &channels[channelno];
    /* 为每个ide通道初始化端口基址及中断向量 */
    switch (channelno) {
    case 0:
        channel->base	 = ATA_PRIMARY_PORT;	   // ide0通道的起始端口号是0x1f0
        channel->irqno	 = IRQ14_HARDDISK;	   // 从片8259a上倒数第二的中断引脚,温盘,也就是ide0通道的的中断向量号
        break;
    case 1:
        channel->base	 = ATA_SECONDARY_PORT;	   // ide1通道的起始端口号是0x170
        channel->irqno	 = IRQ14_HARDDISK + 1;	   // 从8259A上的最后一个中断引脚,我们用来响应ide1通道上的硬盘中断
        break;
    }
    channel->who = 0;	// 初始化为0
    channel->what = 0;

    if (diskno == 0) {  /* 通道上第一个磁盘的时候才注册中断 */
        /* 注册中断 */
        irq_register(channel->irqno, ide_handler, IRQF_DISABLED, "harddisk", DEV_NAME, (void *) channel);
    }
    
#ifdef DEBUG_DRV
    dump_ide_channel(channel);
#endif

    channel->ext = ext;

    /* 填写设备信息 */
    ext->channel = channel;
    ext->drive = diskno;
    ext->type = IDE_ATA;

    ext->info = mem_alloc(SECTOR_SIZE);
    if (ext->info == NULL) {
        printk(KERN_ERR "mem_alloc for ide device %d info failed!\n", id);
        irq_unregister(channel->irqno, (void *)channel);
        
        return -1;
    }
   
    /* 重置驱动器 */
    rest_driver(ext);

    /* 获取磁盘的磁盘信息 */
    select_disk(ext, 0, 0);
    
    int timeout = 1000; // 等待超时，如果没有IDE设备存在，就会超时

    //等待硬盘准备好
    while (!(in8(ATA_REG_STATUS(channel)) & ATA_STATUS_READY) && (--timeout) > 0);
    if (timeout <= 0) {
        printk(KERN_NOTICE "[ide]: disk %d maybe not ready or not exist!\n", id);
        irq_unregister(channel->irqno, (void *)channel);
        return -1;
    }

    send_cmd(channel, ATA_CMD_IDENTIFY);
    
    err = ide_polling(channel, 1);
    if (err) {
        ide_print_error(ext, err);
        irq_unregister(channel->irqno, (void *)channel);
        mem_free(ext->info);
        return -1;
    }
    
    /* 读取到缓冲区中 */
    read_from_sector(ext, ext->info, 1);

    /* 根据信息设定一些基本数据 */
    ext->command_sets = (int)((ext->info->cmdSet1 << 16) + ext->info->cmdSet0);

    /* 根据模式设置读取扇区大小 */
    if (ext->command_sets & (1 << 26)) {
        ext->size = ((int)ext->info->lba48Sectors[1] << 16) + \
            (int)ext->info->lba48Sectors[0];

    } else {
        ext->size = ((int)ext->info->lba28Sectors[1] << 16) + 
            (int)ext->info->lba28Sectors[0];

    }

    ext->capabilities = ext->info->Capabilities0;
    ext->signature = ext->info->General_Config;
    ext->reserved = 1;	/* 设备存在 */
    ext->rwoffset = 0;
#ifdef DEBUG_DRV
    dump_ide_extension(ext);
    pr_dbg("probe IDE disk: base:%x irq:%d\n", channel->base, channel->irqno);
#endif
    
    return 0;
}

static iostatus_t ide_enter(driver_object_t *driver)
{
    iostatus_t status = IO_FAILED;
    
    device_object_t *devobj;
    device_extension_t *devext;

    volatile int id;
    char devname[DEVICE_NAME_LEN] = {0};

    /* 获取已经配置了的硬盘数量
	这种方法的设备检测需要磁盘安装顺序不能有错误，
	可以用轮训的方式来改变这种情况。 
	 */
	unsigned char disk_foud = *((unsigned char *)IDE_DISK_NR_ADDR);
	printk(KERN_INFO "ide_enter: found %d hard disks.\n", disk_foud);

	/* 有磁盘才初始化磁盘 */
	if (disk_foud > 0) {    
        for (id = 0; id < disk_foud; id++) {
            sprintf(devname, "%s%d", DEV_NAME, id);
            /* 初始化一些其它内容 */
            status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_DISK, &devobj);

            if (status != IO_SUCCESS) {
                printk(KERN_ERR "ide_enter: create device failed!\n");
                return status;
            }
            /* buffered io mode */
            devobj->flags = DO_BUFFERED_IO;

            devext = (device_extension_t *)devobj->device_extension;
            string_new(&devext->device_name, devname, DEVICE_NAME_LEN);
            devext->device_object = devobj;
    #ifdef DEBUG_DRV
            printk(KERN_DEBUG "ide_enter: device extension: device name=%s object=%x\n",
                devext->device_name.text, devext->device_object);
    #endif
            /* 填写设备扩展信息 */
            if (ide_probe(devext, id) < 0) {
                string_del(&devext->device_name); /* 删除驱动名 */
                io_delete_device(devobj);
                status = IO_FAILED;
                continue;
            }
        }
	}

    return status;
}

static iostatus_t ide_exit(driver_object_t *driver)
{
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    device_extension_t *ext;
    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        ext = devobj->device_extension;
        /* 释放分配的信息缓冲区 */
        if (ext->info) {
            mem_free(ext->info);
        }
        if (ext->drive == 0) {  /* 通道上第一个磁盘的时候才注销中断 */
            /* 注销中断 */
    		irq_unregister(ext->channel->irqno, ext->channel);
        }
        
        io_delete_device(devobj);   /* 删除每一个设备 */
    }

    string_del(&driver->name); /* 删除驱动名 */
    return IO_SUCCESS;
}

iostatus_t ide_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = ide_enter;
    driver->driver_exit = ide_exit;

    driver->dispatch_function[IOREQ_READ] = ide_read;
    driver->dispatch_function[IOREQ_WRITE] = ide_write;
    driver->dispatch_function[IOREQ_DEVCTL] = ide_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "ide_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    return status;
}

static __init void ide_driver_entry(void)
{
    if (driver_object_create(ide_driver_func) < 0) {
        printk(KERN_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(ide_driver_entry);
