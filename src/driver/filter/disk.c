#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <xbook/vsprintf.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <xbook/task.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>

#define DRV_NAME "filter-disk"
#define DRV_VERSION "0.1"

#define DEV_NAME "disk"

/* 设备数 */
#define DISK_DEVICE_NR       1

#define DEBUG_LOCAL 0

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
} device_extension_t;

iostatus_t disk_open(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    device_extension_t *extension = device->device_extension;
    
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

iostatus_t disk_close(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    device_extension_t *extension = device->device_extension;

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}


iostatus_t disk_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;


    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

iostatus_t disk_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

iostatus_t disk_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    ssize_t retval = 0;
    switch (ioreq->parame.devctl.code)
    {    
    default:
        
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t disk_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *extension;

    int i;
    char devname[DEVICE_NAME_LEN] = {0, };
    
    for (i = 0; i < DISK_DEVICE_NR; i++) {
        memset(devname, 0, DEVICE_NAME_LEN);
        sprintf(devname, "%s%d", DEV_NAME, i);
        /* 初始化一些其它内容 */
        status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_VIRTUAL_CHAR, &devobj);

        if (status != IO_SUCCESS) {
            printk(KERN_ERR "disk_enter: create device failed!\n");
            device_close(kbd);
            kfree(public);
            return status;
        }
        /* buffered io mode */
        devobj->flags = DO_BUFFERED_IO;
        extension = (device_extension_t *)devobj->device_extension;
        extension->device_object = devobj;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "disk_enter: device extension: device name=%s object=%x\n",
            devext->device_name.text, devext->device_object);
#endif
        
    }
    return status;
}

static iostatus_t disk_exit(driver_object_t *driver)
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

iostatus_t disk_driver_vine(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = disk_enter;
    driver->driver_exit = disk_exit;

    driver->dispatch_function[IOREQ_OPEN] = disk_open;
    driver->dispatch_function[IOREQ_CLOSE] = disk_close;
    driver->dispatch_function[IOREQ_READ] = disk_read;
    driver->dispatch_function[IOREQ_WRITE] = disk_write;
    driver->dispatch_function[IOREQ_DEVCTL] = disk_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "disk_driver_vine: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}
