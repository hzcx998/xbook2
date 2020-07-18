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

typedef struct _device_extension {
    string_t device_name;           /* 设备名字 */
    device_object_t *device_object; /* 设备对象 */
	unsigned int size;		// Size in Sectors.

	/* 状态信息 */
	unsigned int read_sectors;	// 读取了多少扇区
	unsigned int write_sectors;	// 写入了多少扇区

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
