#include <xbook/unit.h>
#include <xbook/debug.h>
#include <xbook/kernel.h>
#include <xbook/device.h>
#include <xbook/const.h>
#include <xbook/math.h>
#include <xbook/softirq.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>

#include <sys/ioctl.h>

/* 配置开始 */
//#define _DEBUG_IDE_INFO
//#define _DEBUG_IDE
//#define _DEBUG_TEST

/* 配置结束 */

#define DRV_NAME "ide disk"
#define DRV_VERSION "0.1"

/* IDE设备最多的磁盘数量 */
#define MAX_IDE_DISK_NR			4

/* IDE设备1个块的大小 */
#define IDE_BLOCK_SIZE			(SECTOR_SIZE * 2)

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
   	unsigned short base;  // I/O Base.
	char irqno;		 	// 本通道所用的中断号
   	struct ide_driver *devices;	// 通道上面的设备
	char who;		/* 通道上主磁盘在活动还是从磁盘在活动 */
	char what;		/* 执行的是什么操作 */
} channels[2];

/* IDE块设备结构体 */
static struct ide_driver {
    device_t *dev;
	struct ide_channel *channel;		/* 所在的通道 */
	struct ide_identy *info;	/* 磁盘信息 */
	unsigned char reserved;	// disk exist
	unsigned char drive;	// 0 (Master Drive) or 1 (Slave Drive).
	unsigned char type;		// 0: ATA, 1:ATAPI.
	unsigned char signature;// Drive Signature
	unsigned int capabilities;// Features.
	unsigned int command_sets; // Command Sets Supported.
	unsigned int size;		// Size in Sectors.

	/* 状态信息 */
	unsigned int read_sectors;	// 读取了多少扇区
	unsigned int write_sectors;	// 写入了多少扇区
}devices[MAX_IDE_DISK_NR];		/* 最多有4块磁盘 */

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

/* 扫描到的磁盘数 */
static unsigned char disk_foud;

/**
 * ide_print_error - 打印错误
 * @dev: 设备
 * @err: 错误码
 * 
 * 根据错误码打印对应的错误
 */
static unsigned char ide_print_error(struct ide_driver *dev, unsigned char err)
{
   	if (err == 0)
      	return err;
	
   	printk("IDE:");
   	if (err == 1) {printk("- Device Fault\n     ");}
   	else if (err == 2) {	/* 其它错误 */
		unsigned char state = in8(ATA_REG_ERROR(dev->channel));
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
		(const char *[]){"Primary", "Secondary"}[dev->channel - channels], // Use the channel as an index into the array
		(const char *[]){"Master", "Slave"}[dev->drive] // Same as above, using the drive
		);

   return err;
}
#ifdef _DEBUG_IDE
static void dump_ide_channel(struct ide_channel *channel)
{
	printk("[Ide Channel]\n");

	printk("self:%x base:%x irq:%d\n", channel, channel->base, channel->irqno);
}

static void dump_ide_device(struct ide_driver *dev)
{
	printk("[Ide Device]\n");

	printk("self:%x channel:%x drive:%d type:%s \n",
	 	dev, dev->channel, dev->drive,
		dev->type == IDE_ATA ? "ATA" : "ATAPI");

	printk("capabilities:%x command_sets:%x signature:%x\n",
		dev->capabilities, dev->command_sets, dev->signature);
	
	if (dev->info->cmdSet1 & 0x0400) {
		printk("Total Sector(LBA 48):");
	} else {
		printk("Total Sector(LBA 28):");
	}
	printk("%d\n", dev->size);

	#ifdef _DEBUG_IDE_INFO
	
	printk("Serial Number:");
	
	int i;
	for (i = 0; i < 10; i++) {
		printk("%c%c", (dev->info->Serial_Number[i] >> 8) & 0xff,
			dev->info->Serial_Number[i] & 0xff);
	}

	printk("\n" "Fireware Version:");
	for (i = 0; i < 4; i++) {
		printk("%c%c", (dev->info->Firmware_Version[i] >> 8) & 0xff,
			dev->info->Firmware_Version[i] & 0xff);
	}

	printk("\n" "Model Number:");
	for (i = 0; i < 20; i++) {
		printk("%c%c", (dev->info->Model_Number[i] >> 8) & 0xff,
			dev->info->Model_Number[i] & 0xff);
	}
	
	printk("\n" "LBA supported:%s ",(dev->info->Capabilities0 & 0x0200) ? "Yes" : "No");
	printk("LBA48 supported:%s\n",((dev->info->cmdSet1 & 0x0400) ? "Yes" : "No"));
	
	#endif
}
#endif  /* _DEBUG_IDE */

/**
 * read_from_sector - 硬盘读入count个扇区的数据到buf
 * @dev: 设备
 * @buf: 缓冲区
 * @count: 扇区数 
 */
static void read_from_sector(struct ide_driver* dev, void* buf, unsigned int count) {
	unsigned int size_in_byte;
	if (count == 0) {
	/* 因为sec_cnt是8位变量,由主调函数将其赋值时,若为256则会将最高位的1丢掉变为0 */
		size_in_byte = 256 * SECTOR_SIZE;
	} else { 
		size_in_byte = count * SECTOR_SIZE; 
	}
	io_read(ATA_REG_DATA(dev->channel), buf, size_in_byte);
}

/**
 * write_to_sector - 将buf中count扇区的数据写入硬盘
 * @dev: 设备
 * @buf: 缓冲区
 * @count: 扇区数 
 */
static void write_to_sector(struct ide_driver* dev, void* buf, unsigned int count)
{
   unsigned int size_in_byte;
	if (count == 0) {
	/* 因为sec_cnt是8位变量,由主调函数将其赋值时,若为256则会将最高位的1丢掉变为0 */
		size_in_byte = 256 * 512;
	} else { 
		size_in_byte = count * 512; 
	}
   	io_write(ATA_REG_DATA(dev->channel), buf, size_in_byte);
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
static unsigned char busy_wait(struct ide_driver* dev) {
	struct ide_channel* channel = dev->channel;
	/* 等待1秒
	1000 * 1000 纳秒
	*/
   	unsigned int time_limit = 10 * 1000 * 1000;
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
int ide_polling(struct ide_channel* channel, unsigned int advanced_check)
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
    i = 10000;
    while ((in8(ATA_REG_STATUS(channel)) & ATA_STATUS_BUSY) && i--); // Wait for BSY to be zero.
	
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
void select_addr_mode(struct ide_driver *dev,
	unsigned int lba,
	unsigned char *mode,
	unsigned char *head,
	unsigned char *io)
{
	unsigned short cyl;
   	unsigned char sect;
	if (lba >= 0x10000000 && dev->capabilities & 0x200) { // Sure Drive should support LBA in this case, or you are
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
	} else if (dev->capabilities & 0x200)  { // Drive supports LBA?
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
static void select_disk(struct ide_driver* dev,
	unsigned char mode, 
	unsigned char head)
{
   	out8(ATA_REG_DEVICE(dev->channel),
	   ATA_MKDEV_REG(mode == 0 ? 0 : 1, dev->drive, head));

	dev->channel->who = dev->drive;	/* 通道上当前的磁盘 */
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
static void select_sector(struct ide_driver* dev,
	unsigned char mode,
	unsigned char *lbaIO,
	sector_t count)
{
  	struct ide_channel* channel = dev->channel;

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
static void rest_driver(struct ide_driver *dev)
{

	driver_soft_rest(dev->channel);
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
static int pio_data_transfer(struct ide_driver *dev,
	unsigned char rw,
	unsigned char mode,
	unsigned char *buf,
	unsigned short count)
{
	short i;
	unsigned char error;
	if (rw == IDE_READ) {	
		#ifdef _DEBUG_IDE
			printk("PIO read->");
		#endif
		for (i = 0; i < count; i++) {
			/* 醒来后开始执行下面代码*/
			if ((error = busy_wait(dev))) {     //  若失败
				/* 重置磁盘驱动并返回 */
				rest_driver(dev);
				return error;
			}
			read_from_sector(dev, buf, 1);
			buf += SECTOR_SIZE;
		}
	} else {
		#ifdef _DEBUG_IDE
			printk("PIO write->");
		#endif
		for (i = 0; i < count; i++) {
			/* 等待硬盘控制器请求数据 */
			if ((error = busy_wait(dev))) {     //  若失败
				/* 重置磁盘驱动并返回 */
				rest_driver(dev);
				return error;
			}
			/* 把数据写入端口，完成1个扇区后会产生一次中断 */
			write_to_sector(dev, buf, 1);
			buf += SECTOR_SIZE;
            //printk("write success! ");
		}
		/* 刷新写缓冲区 */
		out8(ATA_REG_CMD(dev->channel), mode > 1 ?
			ATA_CMD_CACHE_FLUSH_EXT : ATA_CMD_CACHE_FLUSH);
		ide_polling(dev->channel, 0);
	}
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
static int ata_type_transfer(struct ide_driver *dev,
	unsigned char rw,
	unsigned int lba,
	unsigned int count,
	void *buf)
{
	unsigned char mode;	/* 0: CHS, 1:LBA28, 2: LBA48 */
	unsigned char dma; /* 0: No DMA, 1: DMA */
	unsigned char cmd;	
	unsigned char *_buf = (unsigned char *)buf;

	unsigned char lbaIO[6];	/* 由于最大是48位，所以这里数组的长度为6 */

	struct ide_channel *channel = dev->channel;

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
		select_addr_mode(dev, lba + done, &mode, &head, lbaIO);

		/* 等待驱动不繁忙 */
		// (III) Wait if the drive is busy;
		while (in8(ATA_REG_STATUS(channel)) & ATA_STATUS_BUSY) cpu_lazy();// Wait if busy.
		/* 从控制器中选择设备 */
		select_disk(dev, mode, head);

		/* 填写参数，扇区和扇区数 */
		select_sector(dev, mode, lbaIO, count);

		/* 等待磁盘控制器处于准备状态 */
		while (!(in8(ATA_REG_STATUS(channel)) & ATA_STATUS_READY)) cpu_lazy();

		/* 选择并发送命令 */
		select_cmd(rw, mode, dma, &cmd);

		#ifdef _DEBUG_IDE	
			printk("lba mode %d num %d io %d %d %d %d %d %d->",
				mode, lba, lbaIO[0], lbaIO[1], lbaIO[2], lbaIO[3], lbaIO[4], lbaIO[5]);
			printk("rw %d dma %d cmd %x head %d\n",
				rw, dma, cmd, head);
		#endif
		/* 等待磁盘控制器处于准备状态 */
		while (!(in8(ATA_REG_STATUS(channel)) & ATA_STATUS_READY)) cpu_lazy();

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
			if ((err = pio_data_transfer(dev, rw, mode, _buf, todo))) {
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
static int ide_read_sector(struct ide_driver *dev,
	unsigned int lba,
	void *buf,
	unsigned int count)
{
	unsigned char error;
	/* 检查设备是否正确 */
	if (dev < devices || dev >= &devices[3] || dev->reserved == 0) {
		printk("device error!\n");
        return -1;
	} else if (lba + count > dev->size && dev->type == IDE_ATA) {
        printk("out of range!\n");
		return -1;
	} else {

		/* 进行磁盘访问 */

		/*如果类型是ATA*/
        error = ata_type_transfer(dev, IDE_READ, lba, count, buf);
		/*如果类型是ATAPI*/
		
		/* 打印驱动错误信息 */
		if(ide_print_error(dev, error)) {
			printk("ide read error!\n");
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
static int ide_write_sector(struct ide_driver *dev,
	unsigned int lba,
	void *buf,
	unsigned int count)
{
	unsigned char error;
	/* 检查设备是否正确 */
	if (dev < devices || dev >= &devices[3] || dev->reserved == 0) {
		return -1;
	} else if (lba + count > dev->size && dev->type == IDE_ATA) {
		return -1;
	} else {
		/* 进行磁盘访问，如果出错，就 */
		
		/*如果类型是ATA*/
			error = ata_type_transfer(dev, IDE_WRITE, lba, count, buf);
		/*如果类型是ATAPI*/

		/* 打印驱动错误信息 */
		if(ide_print_error(dev, error)) {
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
static int ide_clean_disk(struct ide_driver *dev, sector_t count)
{
	if (count == 0)
		count = dev->size;

	sector_t todo, done = 0;

	/* 每次写入10个扇区 */
	char *buffer = kmalloc(SECTOR_SIZE *10);
	if (!buffer) {
		printk("kmalloc for ide buf failed!\n");
		return -1;
	}

	memset(buffer, 0, SECTOR_SIZE *10);

	printk("IDE clean: count%d\n", count);
	while (done < count) {
		/* 获取要去操作的扇区数这里用10作为分界 */
		if ((done + 10) <= count) {
			todo = 10;
		} else {
			todo = count - done;
		}
		//printk("ide clean: done %d todo %d\n", done, todo);
		ide_write_sector(dev, done, buffer, todo);
		done += 10;
	}
	return 0;
}

/**
 * 编写设备操作接口
 */

/**
 * ide_ioctl - 硬盘的IO控制
 * @device: 设备项
 * @cmd: 命令
 * @arg1: 参数1
 * @arg2: 参数2
 * 
 * 成功返回0，失败返回-1
 */
static int ide_ioctl(device_t *device, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	struct ide_driver *dev = (struct ide_driver *)dev_get_local(device);
	
	switch (cmd)
	{
	case IDEIO_RINSE:	/* 执行清空磁盘命令 */
		if (ide_clean_disk(dev, arg)) {
			retval = -1;
		}
		break;
    case IDEIO_GETCNTS:	/* 获取扇区数 */
        *((sector_t *)arg) = dev->size;
		break;
	default:
		/* 失败 */
		retval = -1;
		break;
	}

	return retval;
}

/**
 * ide_read - 读取数据
 * @device: 设备
 * @lba: 逻辑扇区地址
 * @buffer: 缓冲区
 * @count: 扇区数
 * 
 * @return: 成功返回0，失败返回-1
 */
static int ide_read(device_t *device, off_t off, void *buffer, size_t count)
{
	/* 获取IDE设备 */
	struct ide_driver *dev = (struct ide_driver *)dev_get_local(device);
	return ide_read_sector(dev, off, buffer, count);
}

/**
 * ide_write - 写入数据
 * @device: 设备
 * @lba: 逻辑扇区地址
 * @buffer: 缓冲区
 * @count: 扇区数
 * 
 * @return: 成功返回0，失败返回-1
 */
static int ide_write(device_t *device, off_t off, void *buffer, size_t count)
{
	/* 获取IDE设备 */
	struct ide_driver *dev = (struct ide_driver *)dev_get_local(device);
	
	return ide_write_sector(dev, off, buffer, count);
}

/**
 * 块设备操作的结构体
 */
static device_ops_t ops = {
	.ioctl  = ide_ioctl,
	.read   = ide_read,
	.write  = ide_write,
};

static driver_info_t drvinfo = {
    .name = DRV_NAME,
    .version = DRV_VERSION,
    .owner = "jason hu",
};

/**
 * ide_add_device - 创建块设备
 * @dev: 要创建的设备
 * @major: 主设备号
 * @diskIdx: 磁盘的索引
 */
static int ide_add_device(struct ide_driver *dev, int major, int idx)
{
    /* register device */
    dev->dev = dev_alloc(MKDEV(IDE_MAJOR, idx));
    if (dev->dev == NULL) {
        printk(KERN_ERR "alloc dev for ide failed!\n");
        return -1;
    }

    dev->dev->ops = &ops;
    dev->dev->drvinfo = &drvinfo;

    dev_make_name(devname, "hd", idx);
    if (register_device(dev->dev, devname, DEVTP_BLOCK, dev)) {
        printk(KERN_ERR "register dev for ide failed!\n");
        dev_free(dev->dev);
        return -1;
    }
    return 0;
}

/**
 * ide_del_device - 删除一个块设备
 * @dev: 要删除的块设备
 */
static void ide_del_device(struct ide_driver *dev)
{
	if (dev->dev) {
        unregister_device(dev->dev);
        dev_free(dev->dev);
    }
}

/* 硬盘的任务协助 */
static task_assist_t ide_assist;

/**
 * ide_assist_handler - 任务协助处理函数
 * @data: 传递的数据
 */
static void ide_assist_handler(unsigned long data)
{
	struct ide_channel *channel = (struct ide_channel *)data;

	struct ide_driver *dev = channel->devices + channel->who;

	/* 获取状态，做出错判断 */
	if (in8(ATA_REG_STATUS(channel)) & ATA_STATUS_ERR) {
		/* 尝试重置驱动 */
		rest_driver(dev);
	}

	/* 可以在这里面做一些统计信息 */
	if (channel->what == IDE_READ) {
		dev->read_sectors++;
	} else if (channel->what == IDE_WRITE) {
		dev->write_sectors++;
	}
}

/**
 * ide_handler - IDE硬盘中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int ide_handler(unsigned long irq, unsigned long data)
{
    //ide_assist.data = data;
	/* 调度协助 */
	//task_assist_schedule(&ide_assist);
    
    struct ide_channel *channel = (struct ide_channel *)data;
	struct ide_driver *dev = channel->devices + channel->who;

    
	/* 获取状态，做出错判断 */
	if (in8(ATA_REG_STATUS(channel)) & ATA_STATUS_ERR) {
		/* 尝试重置驱动 */
		rest_driver(dev);
	}

	/* 可以在这里面做一些统计信息 */
	if (channel->what == IDE_READ) {
		dev->read_sectors++;
	} else if (channel->what == IDE_WRITE) {
		dev->write_sectors++;
	}
    return 0;
}

/**
 * ide_probe - 探测设备
 * @disks: 找到的磁盘数
 * 
 * 根据磁盘数初始化对应的磁盘的信息
 */
static void ide_probe(char disks)
{
	/* 初始化，并获取信息 */
	int channel_cnt = DIV_ROUND_UP(disks, 2);	   // 一个ide通道上有两个硬盘,根据硬盘数量反推有几个ide通道
	struct ide_channel* channel;
	int channelno = 0, devno = 0; 
	char err;

	int left_disks = disks;

   	/* 处理每个通道上的硬盘 */
   	while (channelno < channel_cnt) {
      	channel = &channels[channelno];
		/* 为每个ide通道初始化端口基址及中断向量 */
		switch (channelno) {
		case 0:
			channel->base	 = ATA_PRIMARY_PORT;	   // ide0通道的起始端口号是0x1f0
			channel->irqno	 = IRQ14;	   // 从片8259a上倒数第二的中断引脚,温盘,也就是ide0通道的的中断向量号
			break;
		case 1:
			channel->base	 = ATA_SECONDARY_PORT;	   // ide1通道的起始端口号是0x170
			channel->irqno	 = IRQ15;	   // 从8259A上的最后一个中断引脚,我们用来响应ide1通道上的硬盘中断
			break;
		}
		channel->who = 0;	// 初始化为0
		channel->what = 0;
		//SynclockInit(&channel->lock);	

		/* 初始化任务协助 */
		task_assist_init(&ide_assist, ide_assist_handler, 0);
        
		/* 注册中断 */
		register_irq(channel->irqno, ide_handler, IRQF_DISABLED, "ATA", "ide", (unsigned long)channel);
		#ifdef _DEBUG_IDE_INFO	
		dump_ide_channel(channel);
		#endif
        
		/* 分别获取两个硬盘的参数及分区信息 */
		while (devno < 2 && left_disks) {
			/* 选择一个设备 */
			if (channelno == ATA_SECONDARY) {
				channel->devices = &devices[2];
			} else {
				channel->devices = &devices[0];
			}
			
			/* 填写设备信息 */
			struct ide_driver* dev = &channel->devices[devno];
			dev->channel = channel;
			dev->drive = devno;
			dev->type = IDE_ATA;

			dev->info = kmalloc(SECTOR_SIZE);
			if (dev->info == NULL) {
				printk("kmalloc for ide device info failed!\n");
				continue;
			}
            
            /* 重置驱动器 */
            rest_driver(dev);

            /* 获取磁盘的磁盘信息 */
			select_disk(dev, 0, 0);
			
			//等待硬盘准备好
			while (!(in8(ATA_REG_STATUS(channel)) & ATA_STATUS_READY)) cpu_lazy();

			send_cmd(channel, ATA_CMD_IDENTIFY);
			
			err = ide_polling(channel, 1);
			if (err) {
				ide_print_error(dev, err);
				continue;
			}
			
			/* 读取到缓冲区中 */
			read_from_sector(dev, dev->info, 1);

			/* 根据信息设定一些基本数据 */
			dev->command_sets = (int)((dev->info->cmdSet1 << 16) + dev->info->cmdSet0);

			/* 根据模式设置读取扇区大小 */
			if (dev->command_sets & (1 << 26)) {
				dev->size = ((int)dev->info->lba48Sectors[1] << 16) + \
					(int)dev->info->lba48Sectors[0];

            } else {
				dev->size = ((int)dev->info->lba28Sectors[1] << 16) + 
					(int)dev->info->lba28Sectors[0];

            }

			dev->capabilities = dev->info->Capabilities0;
			dev->signature = dev->info->General_Config;
			dev->reserved = 1;	/* 设备存在 */
			#ifdef _DEBUG_IDE_INFO
			dump_ide_device(dev);
			#endif
			devno++; 
			left_disks--;
		}
		devno = 0;			  	   // 将硬盘驱动器号置0,为下一个channel的两个硬盘初始化。
		channelno++;				   // 下一个channel
	}
}


/**
 * ide_init - 初始化IDE硬盘驱动
 */
static int ide_init()
{
	/* 获取已经配置了的硬盘数量
	这种方法的设备检测需要磁盘安装顺序不能有错误，
	可以用轮训的方式来改变这种情况。 
	 */
	disk_foud = *((unsigned char *)IDE_DISK_NR_ADDR);
	printk(KERN_INFO "found %d hard disks.\n", disk_foud);

	/* 有磁盘才初始化磁盘 */
	if (disk_foud > 0) {
        /* 驱动本身的初始化 */
		ide_probe(disk_foud);
    
		int status;
		/* 块设备的初始化 */
		int i;
		for (i = 0; i < disk_foud; i++) {
			status = ide_add_device(&devices[i], IDE_MAJOR, i);
			if (status < 0) {
				return status;
			}
		}	
	}

#if 0   /* for test */
	//ide_clean_disk(&devices[0], 0);

    dev_open(DEV_HD0, 0);

	logger("test start!\n");
	char *buf = kmalloc(SECTOR_SIZE*10);
	if (!buf)
		panic("error!");

    int read = dev_read(DEV_HD0, 0, buf, 10);
	if (read == -1)
		printk("block read error!\n");

	printk("%x-%x-%x-%x\n",
	 buf[0], buf[511], buf[512],buf[512*2-1]);

	memset(buf, 0x5a, SECTOR_SIZE*10);

	int write = dev_write(DEV_HD0, 0, buf, 10);
	if (write == -1)
		printk("block write error!\n");
	
    memset(buf, 0, SECTOR_SIZE*10);

    ide_read_sector(&devices[0],0,buf, 5);

	printk("%x-%x-%x-%x-%x-%x\n",
	 buf[0], buf[511], buf[512+511],buf[512*2], buf[4096],buf[10*512-1]);
	/*
	ide_read_sector(&devices[0],2,5,buf);

	printk("%x-%x-%x-%x-%x-%x\n",
	 buf[0], buf[511], buf[512+511],buf[512*2], buf[4096],buf[10*512-1]);
	*/
	memset(buf,0xfa,SECTOR_SIZE * 5);
	
	ide_write_sector(&devices[0],10,buf,5);
	
	memset(buf,1,SECTOR_SIZE * 5);
	
	ide_read_sector(&devices[0],10,buf,5);

	printk("%x-%x-%x-%x-%x-%x\n",
	 buf[0], buf[511], buf[512+511],buf[512*2], buf[4096],buf[10*512-1]);

	printk("read %d write %d\n", devices[0].read_sectors, devices[0].write_sectors);	
#endif
    return 0;
}

/**
 * ide_exit - 退出驱动
 */
static void ide_exit()
{
	/* 删除内核设备信息 */
	int i, j;
	for (i = 0; i < disk_foud; i++) {
		ide_del_device(&devices[i]);
	}
	
	/* 删除设备信息 */
	int channel_cnt = DIV_ROUND_UP(disk_foud, 2);	   // 一个ide通道上有两个硬盘,根据硬盘数量反推有几个ide通道
	int disks = disk_foud;
	struct ide_channel *channel;
	for (i = 0; i < channel_cnt; i++) {
		channel = &channels[i];
		/* 注销中断 */
		unregister_irq(channel->irqno, channel);
		j = 0;
		/* 释放分配的资源 */
		while (j < 2 && disks > 0) {
			kfree(channel->devices[j].info);
			disks--;
		}
	}
}

EXPORT_UNIT(ide_unit, "ide", ide_init, ide_exit);