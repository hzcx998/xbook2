#include <xbook/debug.h>
#include <xbook/disk.h>
#include <xbook/driver.h>
#include <xbook/bitops.h>
#include <string.h>

#include <sys/ioctl.h>

#define DRV_NAME    "iso9660-cdrom"
#define DRV_VERSION "0.1"

#define DEV_NAME "cdrom"

typedef struct _device_extension
{
    device_object_t *device_object;
    disk_t disk;
    unsigned long rwoffset;
} device_extension_t;

iostatus_t cdrom_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;

    unsigned long off;
    unsigned long len;

    if (ioreq->parame.read.offset == DISKOFF_MAX)
    {
        off = extension->rwoffset;
    }
    else
    {
        off = ioreq->parame.read.offset;
    }

    len = ioreq->parame.read.length;

    disk_read_sector(&extension->disk, off, len / extension->disk.sector_size, ioreq->system_buffer);

    ioreq->io_status.infomation = len;
    ioreq->io_status.status = status;

    io_complete_request(ioreq);
    return status;
}

iostatus_t cdrom_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    unsigned long off;

    unsigned long arg = ioreq->parame.devctl.arg;

    switch (ioreq->parame.devctl.code)
    {
    case DISKIO_GETSECSIZE:
        *((unsigned int *)arg) = extension->disk.sector_size;
        break;
    case DISKIO_SETOFF:
        off = *((unsigned long *) arg);
        extension->rwoffset = off;
        break;
    case DISKIO_GETOFF:
        *((unsigned long *) arg) = extension->rwoffset;
        break;
    default:
        status = IO_FAILED;
        break;
    }

    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t cdrom_enter(driver_object_t *driver)
{
    iostatus_t status;

    device_object_t *devobj;
    device_extension_t *extension;

    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_DISK, &devobj);

    if (status != IO_SUCCESS)
    {
        keprint(PRINT_ERR "cdrom_enter: create device failed!\n");
        return status;
    }

    extension = (device_extension_t *)devobj->device_extension;
    extension->disk.name = DEV_NAME;
    extension->rwoffset = 0;

    if (disk_match(&extension->disk))
    {
        status = IO_FAILED;
    }

    /* neither io mode */
    devobj->flags = DO_BUFFERED_IO;

    return status;
}

static iostatus_t cdrom_exit(driver_object_t *driver)
{
    device_object_t *devobj, *next;

    list_for_each_owner_safe (devobj, next, &driver->device_list, list)
    {
        io_delete_device(devobj);
    }

    string_del(&driver->name);

    return IO_SUCCESS;
}

iostatus_t cdrom_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;

    driver->driver_enter = cdrom_enter;
    driver->driver_exit = cdrom_exit;

    driver->dispatch_function[IOREQ_READ] = cdrom_read;
    driver->dispatch_function[IOREQ_DEVCTL] = cdrom_devctl;

    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);

    return status;
}

static __init void cdrom_driver_entry(void)
{
    if (driver_object_create(cdrom_driver_func) < 0)
    {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(cdrom_driver_entry);
