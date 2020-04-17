#include <xbook/unit.h>
#include <xbook/debug.h>
#include <xbook/device.h>
#include <xbook/bitops.h>
#include <xbook/vsprintf.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <xbook/mdl.h>
#include <arch/io.h>
#include <arch/interrupt.h>

#define DRV_NAME "uart-serial"
#define DRV_VERSION "0.1"

#define DEV_NAME "com"

/* 传输方法：0->user, 1->buffered, 2->direct  */
#define TRANS_METHOD 0

typedef struct _device_extension {
    string_t device_name;           /* 设备名字 */
    device_object_t *device_object; /* 设备对象 */
} device_extension_t;

iostatus_t serial_open(device_object_t *device, io_request_t *ioreq)
{
    printk(KERN_DEBUG "serial_open: enter!\n");
    
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

iostatus_t serial_close(device_object_t *device, io_request_t *ioreq)
{
    printk(KERN_DEBUG "serial_close: enter!\n");
    
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

iostatus_t serial_read(device_object_t *device, io_request_t *ioreq)
{
    printk(KERN_DEBUG "serial_read: enter!\n");
    
    unsigned long len = ioreq->parame.read.length;

#if TRANS_METHOD == 0
    memset(ioreq->user_buffer, 0xaa, len);
#elif TRANS_METHOD == 1
    memset(ioreq->system_buffer, 0xaa, len);
#elif TRANS_METHOD == 2
    memset(MDL_GET_MAPPED_VADDR(ioreq->mdl_address), 0xaa, len);    
#endif
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;

    /* 调用完成请求 */
    io_complete_request(ioreq);

    return IO_SUCCESS;
}

iostatus_t serial_write(device_object_t *device, io_request_t *ioreq)
{
    printk(KERN_DEBUG "serial_write: enter!\n");

    unsigned long len = ioreq->parame.read.length;

#if TRANS_METHOD == 0
    dump_buffer(ioreq->user_buffer, 32, 1);
#elif TRANS_METHOD == 1
    dump_buffer(ioreq->system_buffer, 32, 1);
#elif TRANS_METHOD == 2
    dump_buffer(MDL_GET_MAPPED_VADDR(ioreq->mdl_address), 32, 1);
#endif
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

iostatus_t serial_devctl(device_object_t *device, io_request_t *ioreq)
{
    printk(KERN_DEBUG "serial_devctl: enter!\n");
    unsigned int ctlcode = ioreq->parame.devctl.code;

    iostatus_t status;

    switch (ctlcode)
    {
    case DEVCTL_CODE_TEST:
        printk(KERN_DEBUG "serial_devctl: code=%x arg=%x\n", ctlcode, ioreq->parame.devctl.arg);
        status = IO_SUCCESS;
        break;
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t serial_enter(driver_object_t *driver)
{
    printk(KERN_DEBUG "driver_enter: enter.\n");
    iostatus_t status;
    
    device_object_t *devobj;
    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_SERIAL_PORT, &devobj);

    if (status != IO_SUCCESS) {
        printk(KERN_DEBUG "serial_driver_vine: create device failed!\n");
        return status;
    }

#if TRANS_METHOD == 0
    devobj->flags = 0;
#elif TRANS_METHOD == 1
    devobj->flags = DO_BUFFERED_IO;
#elif TRANS_METHOD == 2
    devobj->flags = DO_DIRECT_IO;
#endif
    
    dump_device_object(devobj);

    device_extension_t *devext = (device_extension_t *)devobj->device_extension;
    string_new(&devext->device_name, DEV_NAME, DEVICE_NAME_LEN);
    devext->device_object = devobj;

    printk(KERN_DEBUG "serial_driver_vine: device extension: device name=%s object=%x\n",
        devext->device_name.text, devext->device_object);
    
    return IO_SUCCESS;
}

static iostatus_t serial_exit(driver_object_t *driver)
{
    printk(KERN_NOTICE "serial_exit: enter.\n");
    
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        io_delete_device(devobj);   /* 删除每一个设备 */
    }

    string_del(&driver->name); /* 删除驱动名 */
    printk(KERN_NOTICE "serial_exit: leave.\n");
    
    return IO_SUCCESS;
}

iostatus_t serial_driver_vine(driver_object_t *driver)
{
    printk(KERN_DEBUG "driver_vine_serial: enter.\n");
    
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = serial_enter;
    driver->driver_exit = serial_exit;

    driver->dispatch_function[IOREQ_OPEN] = serial_open;
    driver->dispatch_function[IOREQ_CLOSE] = serial_close;
    driver->dispatch_function[IOREQ_READ] = serial_read;
    driver->dispatch_function[IOREQ_WRITE] = serial_write;
    driver->dispatch_function[IOREQ_DEVCTL] = serial_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);

    printk(KERN_DEBUG "serial_driver_vine: driver name=%s\n",
        driver->name.text);

    return status;
}
