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

#define DRV_NAME "virt-null"
#define DRV_VERSION "0.1"

#define DEV_NAME "null"

#define DEBUG_LOCAL 0

iostatus_t null_read(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "null_read: data:\n");
#endif
    int len = ioreq->parame.read.length;
    unsigned char *data = (unsigned char *) ioreq->user_buffer;
    while (len-- > 0) {
        *data = 0;
        data++;
    }

    ioreq->io_status.infomation = ioreq->parame.read.length;    /* 读取永远是0 */
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

iostatus_t null_write(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "null_write: data:\n");
    int len = ioreq->parame.write.length;
    unsigned char *data = (unsigned char *) ioreq->user_buffer;
    while (len-- > 0) {
        printk("%x ", *data);
        data++;
    }
#endif
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = ioreq->parame.write.length;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t null_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    /* 初始化一些其它内容 */
    status = io_create_device(driver, 0, DEV_NAME, DEVICE_TYPE_VIRTUAL_CHAR, &devobj);
    if (status != IO_SUCCESS) {
        printk(KERN_ERR "null_enter: create device failed!\n");
        return status;
    }
    /* neighter io mode */
    devobj->flags = 0;
    return status;
}

static iostatus_t null_exit(driver_object_t *driver)
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

iostatus_t null_driver_vine(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = null_enter;
    driver->driver_exit = null_exit;

    driver->dispatch_function[IOREQ_READ] = null_read;
    driver->dispatch_function[IOREQ_WRITE] = null_write;

    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "null_driver_vine: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}
