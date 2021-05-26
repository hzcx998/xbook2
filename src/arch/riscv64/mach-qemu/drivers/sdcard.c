
#include <xbook/debug.h>
#include <arch/memory.h>
#include <string.h>
#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <xbook/hardirq.h>
#include <xbook/schedule.h>
#include <xbook/task.h>
#include <xbook/clock.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>

// sdcard
#include <sdcard.h>
#include <dmac.h>

#define DRV_NAME "sdcard"
#define DRV_VERSION "0.1"

#define DEV_NAME "sda"
#define DRV_PREFIX  "[sdcard] "

/* assume 128MB */
#define SDCARD_SECTORS     (262144)

#define SDCARD_IRQ 27

//#define DEBUG_DRV

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    unsigned long sectors;          /* 磁盘扇区数 */
    unsigned long rwoffset;         // 读写偏移位置
    uint8_t *memio_addr;            /* 磁盘内存映射地址 */

} device_extension_t;


static void sdcard_rw(device_extension_t *extension, 
        unsigned long lba, void *data, int write)
{
    #ifdef DEBUG_DRV
    dbgprintln(DRV_PREFIX"sdcard_rw: lba=%lx data=%p, rw=%d [START]", lba, data, write);
    #endif
    if (write) {
        sdcard_write_sector(data, lba);
    } else {
        sdcard_read_sector(data, lba);
    }
    #ifdef DEBUG_DRV
    dbgprintln(DRV_PREFIX"sdcard_rw: lba=%lx data=%p, rw=%d [END]]", lba, data, write);
    #endif
}


static iostatus_t sdcard_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    unsigned long off;    
    if (ioreq->parame.read.offset == DISKOFF_MAX) {
        off = extension->rwoffset;
    } else {
        off = ioreq->parame.read.offset;
    }
    unsigned long length = ioreq->parame.read.length;
    size_t sectors = DIV_ROUND_UP(length, PAGE_SIZE);
    char *data = (char *)ioreq->system_buffer;
    /* 判断越界 */
    if (off + sectors  >= extension->sectors) {
		status = IO_FAILED;
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "sdcard_read: read disk offset=%d counts=%d failed!\n",
            off, sectors);
#endif

	} else {
		/* 进行磁盘读取 */
		int i;
        for (i = 0; i < sectors; i++) {
            sdcard_rw(extension, off + i, data + i * SECTOR_SIZE, 0);
        }

        ioreq->io_status.infomation = length;
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "sdcard_read: read disk offset=%d counts=%d ok.\n",
            off, (length / SECTOR_SIZE));
#endif

	}
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "sdcard_read: io status:%d\n", status);
#endif
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t sdcard_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    unsigned long off;    
    if (ioreq->parame.write.offset == DISKOFF_MAX) {
        off = extension->rwoffset;
    } else {
        off = ioreq->parame.write.offset;
    }
    unsigned long length = ioreq->parame.write.length;
    size_t sectors = DIV_ROUND_UP(length, PAGE_SIZE);
    char *data = (char *)ioreq->system_buffer;
    /* 判断越界 */
    if (off + sectors  >= extension->sectors) {
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "sdcard_write: write disk offset=%d counts=%d failed!\n",
            off, sectors);
#endif
		status = IO_FAILED;
	} else {

		/* 进行磁盘写入 */
		int i;
        for (i = 0; i < sectors; i++) {
            sdcard_rw(extension, off + i, data + i * SECTOR_SIZE, 1);
        }
        
        ioreq->io_status.infomation = length;
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "sdcard_write: write disk offset=%d counts=%d ok.\n",
            off, (length / SECTOR_SIZE));
#endif
	}
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "sdcard_write: io status:%d\n", status);
#endif
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t sdcard_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    unsigned long arg = ioreq->parame.devctl.arg;
    unsigned long off;
    iostatus_t status = IO_SUCCESS;
    switch (ioreq->parame.devctl.code)
    {    
    case DISKIO_GETSIZE:
        *((unsigned int *) arg) = extension->sectors; 
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "sdcard_devctl: get disk sectors=%d\n", extension->sectors);
#endif
        break;
    case DISKIO_SETOFF:
        off = *((unsigned long *) arg);
        if (off > extension->sectors - 1)
            off = extension->sectors - 1;
        extension->rwoffset = off;
        break;
    case DISKIO_GETOFF:
        *((unsigned long *) arg) = extension->rwoffset;
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static int sdcard_intr(irqno_t irqno, void *data)
{
    device_extension_t *extension = (device_extension_t *) data; 
    
    #ifdef DEBUG_DRV
    dbgprintln(DRV_PREFIX"sdcard_intr: interrupt occur!");
    #endif
    dmac_intr(DMAC_CHANNEL0);
    #ifdef DEBUG_DRV
    dbgprintln(DRV_PREFIX"sdcard_intr: interrupt finished!");
    #endif
    
    return IRQ_HANDLED;
}

static int sdcard_do_init(device_extension_t *extension)
{
    sdcard_init();
    // Register interrupt
    if (irq_register(SDCARD_IRQ, sdcard_intr, 0,
        "sdcard", DEV_NAME, extension) < 0) 
    {
        errprintln(DRV_PREFIX"irq %d register failed!", SDCARD_IRQ);
        return -1; 
    }
    dbgprintln(DRV_PREFIX"disk init done.");
    return 0;
}

static iostatus_t sdcard_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *extension;

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_VIRTUAL_DISK, &devobj);
    if (status != IO_SUCCESS) {
        keprint(PRINT_ERR "sdcard_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = DO_BUFFERED_IO;
    extension = (device_extension_t *)devobj->device_extension;
    extension->device_object = devobj;
    extension->sectors = SDCARD_SECTORS;
    extension->rwoffset = 0;

    if (sdcard_do_init(extension) < 0) {
        io_delete_device(devobj);
        status = IO_FAILED; 
    }
    return status;
}


static iostatus_t sdcard_exit(driver_object_t *driver)
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

static iostatus_t sdcard_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = sdcard_enter;
    driver->driver_exit = sdcard_exit;

    driver->dispatch_function[IOREQ_READ] = sdcard_read;
    driver->dispatch_function[IOREQ_WRITE] = sdcard_write;
    driver->dispatch_function[IOREQ_DEVCTL] = sdcard_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "sdcard_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void sdcard_driver_entry(void)
{
    if (driver_object_create(sdcard_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(sdcard_driver_entry);
