
#ifndef _SERV_NETSERV_H
#define _SERV_NETSERV_H

#include <stdint.h>

#include <sys/list.h>
#include <sys/udev.h>
#include <types.h>

/* 磁盘驱动器 */
typedef struct {
    list_t list;        /* 信息链表 */
    int solt;           /* 插槽位置 */
    int handle;         /* 资源句柄 */
    devent_t devent;    /* 设备项 */
    char virname[DEVICE_NAME_LEN];        /* 虚拟网卡名字 */
} netcard_info_t;

/* 最多支持2个网卡 */
#define NETCARD_SOLT_NR    2

typedef struct {
    int (*open)(int);
    int (*close)(int);
    int (*read)(int , void *, size_t );
    int (*write)(int  , void *, size_t );
    int (*ioctl)(int , unsigned int , unsigned long );
} netcard_drvier_t;

extern netcard_drvier_t drv_netcard;

int netcard_probe_device(device_type_t type);
void netcard_info_print();
int netcard_manager_init();
int netcard_find_by_name(char *name);
void network_init(void);

#endif  /* _SERV_NETSERV_H */
