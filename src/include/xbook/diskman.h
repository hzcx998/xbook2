#ifndef _XBOOK_DISKMAN_H
#define _XBOOK_DISKMAN_H

#include <arch/atomic.h>
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

#define DISK_MAN_SOLT_NR 10

typedef struct {
    int (*open)(int);
    int (*close)(int);
    int (*read)(int , off_t , void *, size_t );
    int (*write)(int , off_t , void *, size_t );
    int (*ioctl)(int , unsigned int , unsigned long );
} disk_manager_t;

extern disk_manager_t diskman;

int disk_probe_device(device_type_t type);
void disk_info_print();
int disk_manager_init();
int disk_info_find(char *name);
int disk_info_find_with_path(char *pathname);

#endif   /* _XBOOK_DISKMAN_H */
