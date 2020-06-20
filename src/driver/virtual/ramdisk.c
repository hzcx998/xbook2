#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <xbook/vsprintf.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/vmarea.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>

#define DRV_NAME "virtual-ramdisk"
#define DRV_VERSION "0.1"

#define DEV_NAME "ramdisk"

/* 10MB */
#define RAMDISK_SECTORS     20480

#define DEBUG_LOCAL 0

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    unsigned char *buffer;          /* 缓冲区 */
    unsigned long buflen;           /* 缓冲区大小 */
    unsigned long sectors;          /* 磁盘扇区数 */
} device_extension_t;

iostatus_t ramdisk_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    unsigned long off = ioreq->parame.read.offset;
    unsigned long length = ioreq->parame.read.length;
    /* 判断越界 */
    if (off + (length / SECTOR_SIZE)  >= extension->sectors) {
		status = IO_FAILED;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "ramdisk_read: read disk offset=%d counts=%d failed!\n",
            off, (length / SECTOR_SIZE));
#endif

	} else {
		/* 进行磁盘读取 */
		memcpy(ioreq->user_buffer, extension->buffer + off * SECTOR_SIZE, length);
        ioreq->io_status.infomation = length;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "ramdisk_read: read disk offset=%d counts=%d ok.\n",
            off, (length / SECTOR_SIZE));
#endif

	}
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "ramdisk_read: io status:%d\n", status);
#endif
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

iostatus_t ramdisk_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    unsigned long off = ioreq->parame.write.offset;
    unsigned long length = ioreq->parame.write.length;
    /* 判断越界 */
    if (off + (length / SECTOR_SIZE)  >= extension->sectors) {
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "ramdisk_write: write disk offset=%d counts=%d failed!\n",
            off, (length / SECTOR_SIZE));
#endif
		status = IO_FAILED;
	} else {

		/* 进行磁盘写入 */
		memcpy(extension->buffer + off * SECTOR_SIZE, ioreq->user_buffer, length);
        ioreq->io_status.infomation = length;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "ramdisk_write: write disk offset=%d counts=%d ok.\n",
            off, (length / SECTOR_SIZE));
#endif
	}
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "ramdisk_write: io status:%d\n", status);
#endif
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

iostatus_t ramdisk_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    unsigned long arg = ioreq->parame.devctl.arg;
    iostatus_t status = IO_SUCCESS;
    switch (ioreq->parame.devctl.code)
    {    
    case DISKIO_GETSIZE:
        *((unsigned int *) arg) = extension->sectors; 
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "ramdisk_devctl: get disk sectors=%d\n", extension->sectors);
#endif
        break;
    case DISKIO_CLEAR:
        /* 清空缓冲区 */
        memset(extension->buffer, 0, extension->buflen);
        ioreq->io_status.infomation = 0;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "ramdisk_devctl: clear disk sectors=%d\n", extension->sectors);
#endif        
        break;
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t ramdisk_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *extension;

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_VIRTUAL_DISK, &devobj);

    if (status != IO_SUCCESS) {
        printk(KERN_ERR "ramdisk_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = 0;
    extension = (device_extension_t *)devobj->device_extension;
    extension->device_object = devobj;
    extension->sectors = RAMDISK_SECTORS;
    extension->buflen = extension->sectors * SECTOR_SIZE;
    extension->buffer = vmalloc(extension->buflen);
    if (extension->buffer == NULL) {
        status = IO_FAILED;
    }
    memset(extension->buffer, 0, extension->buflen);
    
    return status;
}

static iostatus_t ramdisk_exit(driver_object_t *driver)
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

iostatus_t ramdisk_driver_vine(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = ramdisk_enter;
    driver->driver_exit = ramdisk_exit;

    driver->dispatch_function[IOREQ_READ] = ramdisk_read;
    driver->dispatch_function[IOREQ_WRITE] = ramdisk_write;
    driver->dispatch_function[IOREQ_DEVCTL] = ramdisk_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "ramdisk_driver_vine: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}
