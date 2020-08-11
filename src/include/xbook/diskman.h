#ifndef _XBOOK_DISKMAN_H
#define _XBOOK_DISKMAN_H

#include <arch/atomic.h>
#include <sys/res.h>
#include "driver.h"

/* 磁盘驱动器 */
typedef struct {
    list_t list;        /* 信息链表 */
    int solt;           /* 插槽位置 */
    int handle;         /* 资源句柄 */
    devent_t devent;    /* 设备项 */
    atomic_t ref;       /* 引用计数 */
    char virname[DEVICE_NAME_LEN];        /* 虚拟磁盘名字 */
} disk_info_t;

#define DISK_SOLT_NR 10

typedef struct {
    int (*open)(int);
    int (*close)(int);
    int (*read)(int , off_t , void *, size_t );
    int (*write)(int , off_t , void *, size_t );
    int (*ioctl)(int , unsigned int , unsigned long );
} disk_drvier_t;

extern disk_drvier_t drv_disk;

int disk_probe_device(device_type_t type);
void disk_info_print();
int init_disk_driver();
int disk_res_find(char *name);
disk_info_t *disk_res_find_info(char *name);

#endif   /* _XBOOK_DISKMAN_H */
