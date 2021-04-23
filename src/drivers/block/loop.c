#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#define DRV_NAME "loop-block"
#define DRV_VERSION "0.1"

#define DEV_NAME "loop"

// #define DEBUG_LOOP_DRV

/* 默认的循环设备数量 */
#define LOOP_DEV_NR 4  
#define LOOP_IMAGE_PATH_LEN 260  

#define LOOP_FLAG_UP    0x01    /* 设备处于使用中，绑定了镜像文件 */

#define IS_LOOP_UP(ext) ((ext)->flags & LOOP_FLAG_UP)

typedef struct _device_extension {
    char image_file[LOOP_IMAGE_PATH_LEN];  /* 循环设备绑定的镜像文件 */
    int flags;                  /* 标志 */
    unsigned long rwoffset;     /* 读写偏移位置 */
    unsigned long sectors;      /* 磁盘扇区数 */
    int gfd;                    /* 全局文件描述符 */
} device_extension_t;

static iostatus_t loop_read(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    device_extension_t *extension = device->device_extension;
    if (!IS_LOOP_UP(extension) || extension->gfd < 0) {
        errprint("loop_write: device %s not ready!\n", device->name.text);
        status = IO_FAILED;
        goto final;
    }
    unsigned long off;    
    if (ioreq->parame.read.offset == DISKOFF_MAX) {
        off = extension->rwoffset;
    } else {
        off = ioreq->parame.read.offset;
    }
    unsigned long length = ioreq->parame.read.length;
    /* 判断越界 */
    if (off + (length / SECTOR_SIZE)  >= extension->sectors) {
		status = IO_FAILED;
#ifdef DEBUG_LOOP_DRV
        keprint(PRINT_DEBUG "loop_read: read disk offset=%d counts=%d out of range!\n",
            off, (length / SECTOR_SIZE));
#endif
	} else {
		/* 进行磁盘读取 */
        kfile_lseek(extension->gfd, off * SECTOR_SIZE, SEEK_SET);
        int read_bytes = kfile_read(extension->gfd, ioreq->user_buffer, length);
        if (read_bytes <= 0) {
            errprint("loop_read: read loop file %s failed!\n", extension->image_file);
            status = IO_FAILED;
            goto final;
        }
        ioreq->io_status.infomation = read_bytes;
#ifdef DEBUG_LOOP_DRV
        keprint(PRINT_DEBUG "loop_read: read disk offset=%d counts=%d ok.\n",
            off, (length / SECTOR_SIZE));
#endif

	}
final:
#ifdef DEBUG_LOOP_DRV
    keprint(PRINT_DEBUG "loop_read: io status:%d, info:%x\n", status, ioreq->io_status.infomation);
#endif
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t loop_write(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    device_extension_t *extension = device->device_extension;
    if (!IS_LOOP_UP(extension) || extension->gfd < 0) {
        errprint("loop_write: device %s not ready!\n", device->name.text);
        status = IO_FAILED;
        goto final;
    }
    unsigned long off;    
    if (ioreq->parame.write.offset == DISKOFF_MAX) {
        off = extension->rwoffset;
    } else {
        off = ioreq->parame.write.offset;
    }
    unsigned long length = ioreq->parame.write.length;
    /* 判断越界 */
    if (off + (length / SECTOR_SIZE)  >= extension->sectors) {
#ifdef DEBUG_LOOP_DRV
        keprint(PRINT_DEBUG "loop_write: write disk offset=%d counts=%d out of range!\n",
            off, (length / SECTOR_SIZE));
#endif
		status = IO_FAILED;
	} else {
		/* 进行磁盘写入 */
        kfile_lseek(extension->gfd, off * SECTOR_SIZE, SEEK_SET);
        int write_bytes = kfile_write(extension->gfd, ioreq->user_buffer, length);
        if (write_bytes <= 0) {
            errprint("loop_write: write loop file %s failed!\n", extension->image_file);
            status = IO_FAILED;
            goto final;
        }
        ioreq->io_status.infomation = write_bytes;
#ifdef DEBUG_LOOP_DRV
        keprint(PRINT_DEBUG "loop_write: write disk offset=%d counts=%d ok.\n",
            off, (length / SECTOR_SIZE));
#endif
	}
final:
#ifdef DEBUG_LOOP_DRV
    keprint(PRINT_DEBUG "loop_write: io status:%d, info:%x\n", status, ioreq->io_status.infomation);
#endif
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static void loop_extension_init(device_extension_t *extension)
{
    /* 记录文件信息并启用该设备 */
    extension->gfd = -1;
    extension->sectors = 0;
    extension->flags = 0;
    extension->rwoffset = 0;
    memset(extension->image_file, 0, LOOP_IMAGE_PATH_LEN);
}

static int loop_setup(device_object_t *device, char *pathname)
{
    device_extension_t *extension = device->device_extension;
    if (IS_LOOP_UP(extension)) {
        errprint("loop: setup=> device %s is upping, please setdown then try it latter!\n", 
            device->name.text);
        return -1;
    }
    /* 检测文件信息 */
    int fd = kfile_open(pathname, O_RDWR);
    if (fd < 0) {
        errprint("loop: setup=> file %s not exist or no permission to access!\n", pathname);
        return -1;
    }
    /* 获取文件大小 */
    kfile_lseek(fd, 0, SEEK_END);
    int filesz = kfile_ftell(fd);
    kfile_lseek(fd, 0, SEEK_SET);
    if (filesz < SECTOR_SIZE) {
        errprint("loop: setup=> file %s size too small!\n", pathname);
        kfile_close(fd);
        return -1;
    }
    /* 记录文件信息并启用该设备 */
    extension->gfd = fd;
    extension->sectors = filesz / SECTOR_SIZE;
    extension->flags |= LOOP_FLAG_UP;
    strncpy(extension->image_file, pathname, LOOP_IMAGE_PATH_LEN);
    extension->image_file[LOOP_IMAGE_PATH_LEN - 1] = '\0';

    #ifdef DEBUG_LOOP_DRV
    infoprint("loop: setup=> gfd=%d, sectors=%d, image file=%s\n", 
        fd, extension->sectors, extension->image_file);
    #endif
    return 0;
}

static int loop_setdown(device_object_t *device)
{
    device_extension_t *extension = device->device_extension;
    if (!IS_LOOP_UP(extension)) {
        errprint("loop: setdown=> device %s not up, please setup it first!\n", 
            device->name.text);
        return -1;
    }
    if (extension->gfd < 0 || !extension->sectors) {
        errprint("loop: setdown=> device %s extension info error!\n", 
            device->name.text);
        return -1;
    }
    kfile_close(extension->gfd);    /* 关闭文件 */
    loop_extension_init(extension);
    dbgprint("loop: setdown=> device %s extension info ok!\n", 
        device->name.text);
    return 0;
}

static iostatus_t loop_open(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    
    noteprint("\n\n##loop device %s open!\n\n", device->name.text);

    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t loop_close(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    
    noteprint("\n\n##loop device %s close!\n\n", device->name.text);
    
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t loop_devctl(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    device_extension_t *extension = device->device_extension;
    unsigned long arg = ioreq->parame.devctl.arg;
    unsigned long off;
    switch (ioreq->parame.devctl.code)
    {    
    case DISKIO_GETSIZE:
        if (!IS_LOOP_UP(extension)) {
            *((unsigned int *) arg) = 0;
            status = IO_FAILED;
            goto final;
        }
        *((unsigned int *) arg) = extension->sectors; 
#ifdef DEBUG_LOOP_DRV
        keprint(PRINT_DEBUG "loop_devctl: get disk sectors=%d\n", extension->sectors);
#endif
        break;
    case DISKIO_SETOFF:
        if (!IS_LOOP_UP(extension)) {
            status = IO_FAILED;
            goto final;
        }
        off = *((unsigned long *) arg);
        if (off > extension->sectors - 1)
            off = extension->sectors - 1;
        extension->rwoffset = off;
        break;
    case DISKIO_GETOFF:
        if (!IS_LOOP_UP(extension)) {
            *((unsigned long *) arg) = 0;
            status = IO_FAILED;
            goto final;
        }
        *((unsigned long *) arg) = extension->rwoffset;
        break;
    case DISKIO_SETUP:
        /* 设置启动信息 */
        if (loop_setup(device, (char *)arg) < 0) {
            status = IO_FAILED;
            goto final;
        }
        break;
    case DISKIO_SETDOWN:
        /* 设置停止信息 */
        if (loop_setdown(device) < 0) {
            status = IO_FAILED;
            goto final;
        }
        break;
    default:
        status = IO_FAILED;
        break;
    }
final:
    ioreq->io_status.status = status;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t loop_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *extension;
    char devname[DEVICE_NAME_LEN] = {0, };
    
    int i;
    for (i = 0; i < LOOP_DEV_NR; i++) {
        memset(devname, 0, DEVICE_NAME_LEN);
        sprintf(devname, "%s%d", DEV_NAME, i);

        status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_VIRTUAL_DISK, &devobj);

        if (status != IO_SUCCESS) {
            keprint(PRINT_ERR "loop_enter: create device failed!\n");
            return status;
        }
        /* neighter io mode */
        devobj->flags = 0;
        extension = (device_extension_t *)devobj->device_extension;
        loop_extension_init(extension);
    }
    return status;
}

static iostatus_t loop_exit(driver_object_t *driver)
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

iostatus_t loop_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = loop_enter;
    driver->driver_exit = loop_exit;

    driver->dispatch_function[IOREQ_OPEN] = loop_open;
    driver->dispatch_function[IOREQ_CLOSE] = loop_close;
    driver->dispatch_function[IOREQ_READ] = loop_read;
    driver->dispatch_function[IOREQ_WRITE] = loop_write;
    driver->dispatch_function[IOREQ_DEVCTL] = loop_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_LOOP_DRV
    keprint(PRINT_DEBUG "loop_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void loop_driver_entry(void)
{
    if (driver_object_create(loop_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(loop_driver_entry);
