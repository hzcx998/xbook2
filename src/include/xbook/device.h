#ifndef _XBOOK_DEVICE_H
#define _XBOOK_DEVICE_H

#include <xbook/list.h>
#include <arch/atomic.h>
#include <xbook/types.h>

typedef enum device_type {
    DEVTP_UNKNOWN = 0,
    DEVTP_BLOCK,
    DEVTP_CHAR,
    DEVTP_NET,
    MAX_DEVTP_NR
} device_type_t;

/* 这里学习linux的做法，高12位是主设备号，低20位是从设备号 */
#define MINORBITS 20
#define MINORMASK 0x000fffff

#define MKDEV(major,minor) (((major) << MINORBITS) | (minor))

#define MAJOR(dev) ((dev) >> MINORBITS)
#define MINOR(dev) ((dev) & MINORMASK)

/* 设备号 */
#define CONSOLE_MAJOR       1
#define COM_MAJOR           2
#define IDE_MAJOR           10
#define RAMDISK_MAJOR       11

#define NULL_MAJOR          0xfff   /* 最后一个设备号 */

/* 设备号合成 */
#define DEV_NULL            MKDEV(NULL_MAJOR, 0)       /* 控制台设备0 */

#define DEV_CON0            MKDEV(CONSOLE_MAJOR, 0)       /* 控制台设备0 */
#define DEV_CON1            MKDEV(CONSOLE_MAJOR, 1)       /* 控制台设备1 */
#define DEV_CON2            MKDEV(CONSOLE_MAJOR, 2)       /* 控制台设备2 */
#define DEV_CON3            MKDEV(CONSOLE_MAJOR, 3)       /* 控制台设备3 */

#define DEV_HDA             MKDEV(IDE_MAJOR, 0)  /* IDE硬盘：主通道0磁盘 */
#define DEV_HDB             MKDEV(IDE_MAJOR, 1)  /* IDE硬盘：主通道1磁盘 */
#define DEV_HDC             MKDEV(IDE_MAJOR, 2)  /* IDE硬盘：从通道0磁盘 */
#define DEV_HDD             MKDEV(IDE_MAJOR, 3)  /* IDE硬盘：从通道1磁盘 */

#define DEV_RDA             MKDEV(RAMDISK_MAJOR, 0)   /* RAMDISK设备 */
#define DEV_RDB             MKDEV(RAMDISK_MAJOR, 1)   /* RAMDISK设备 */

#define DEV_COM1            MKDEV(COM_MAJOR, 0)       /* COM设备0 */
#define DEV_COM2            MKDEV(COM_MAJOR, 1)       /* COM设备1 */
#define DEV_COM3            MKDEV(COM_MAJOR, 2)       /* COM设备2 */
#define DEV_COM4            MKDEV(COM_MAJOR, 3)       /* COM设备3 */

#define DEVICE_NAME_LEN 24

typedef struct driver_info {
    char *name;             /* 驱动名 */
    char *version;          /* 驱动版本 */
    char *owner;            /* 驱动所有者 */
} driver_info_t;

/* 设备的抽象
每个设备子系统都应该继承这个抽象，然后再添加设备自己有的属性
 */
typedef struct device {
    list_t list;                   /* 所有设备构成一个设备链表 */
    dev_t devno;                        /* 设备号 */
    char name[DEVICE_NAME_LEN];         /* 设备名 */
    driver_info_t *drvinfo;                   /* 驱动信息 */
    struct device_ops *ops;    /* 设备操作集 */
    device_type_t type;                          /* 设备类型 */
    void *local;                      /* 指向设备子系统（字符设备，块设备） */
    atomic_t references;                /* 设备的引用计数 */
} device_t;

typedef struct device_ops {
    int (*open)(struct device *, unsigned long);
    int (*close)(struct device *);
    int (*read)(struct device *, off_t, void *, size_t);
    int (*write)(struct device *, off_t, void *, size_t);
    int (*ioctl)(struct device *, unsigned int, unsigned long);
    int (*getc)(struct device *, unsigned long *);
    int (*putc)(struct device *, unsigned long);
} device_ops_t;

#define SIZEOF_DEVICE sizeof(device_t)

int search_device(char *name);
void dump_devices();
device_t *get_device_by_name(char *name);
device_t *get_device_by_id(dev_t devno);

void dump_device(device_t *device);

/* 空操作或者错误操作 */
int device_io_null();
int device_io_error();

/* 设备操作集接口 */
int dev_open(dev_t devno, flags_t flags);
int dev_close(dev_t devno);
int dev_ioctl(dev_t devno, unsigned int cmd, unsigned long arg);
int dev_write(dev_t devno, off_t off, void *buffer, size_t count);
int dev_read(dev_t devno, off_t off, void *buffer, size_t count);
int dev_getc(dev_t devno, unsigned long *ch);
int dev_putc(dev_t devno, unsigned long ch);

static inline void dev_init(device_t *dev, dev_t devno)
{
    INIT_LIST_HEAD(&dev->list);
    dev->devno = devno;
    memset(dev->name, 0, DEVICE_NAME_LEN);
    dev->ops = NULL;
    dev->drvinfo = NULL;
    dev->local = NULL;
    atomic_set(&dev->references, 0);
    dev->type = DEVTP_UNKNOWN;
}

/**
 * dev_alloc - 分配一个设备
 * 
 */
static inline device_t *dev_alloc(dev_t devno)
{
    device_t *dev = kmalloc(sizeof(device_t));
    if (dev == NULL)
        return NULL;
    dev_init(dev, devno);
    return dev;
}

static inline void dev_free(device_t *dev)
{
    kfree(dev);
}

int register_device(device_t *dev, char *name, device_type_t type, void *local);

int unregister_device(device_t *dev);

#define dev_get_local(dev)  (dev)->local


#endif   /* _XBOOK_DEVICE_H */
