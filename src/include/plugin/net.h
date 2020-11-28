
#ifndef _XBOOK_PLUGIN_NET_H
#define _XBOOK_PLUGIN_NET_H

#include <lwip/sockets.h>
#include <stdint.h>

void network_init(void);

/* 套接字参数传递 */
struct _sockarg {
    void *buf;                  /* 缓冲区 */
    int len;                    /* 缓冲区长度 */
    unsigned int flags;         /* 标志 */
    struct sockaddr *to_from;   /* 传输目的地或者传输源 */
    socklen_t tolen;                  /* 套接字结构长度 */
    socklen_t *fromlen;               /* 来源套接字结构长度 */
};

struct _sockfd_set {
    fd_set *readfds;
    fd_set *writefds;
    fd_set *errorfds;
};

int sys_socket(int domain, int type, int protocol);
int sys_bind(int sockfd, struct sockaddr *my_addr, int addrlen);
int sys_connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
int sys_listen(int sockfd, int backlog);
int sys_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int sys_send(int sockfd, const void *buf, int len, int flags);
int sys_recv(int sockfd, void *buf, int len, unsigned int flags);
int sys_sendto(int sockfd, struct _sockarg *arg);
int sys_recvfrom(int sockfd, struct _sockarg *arg);
int sys_shutdown(int sockfd, int how);
int sys_getpeername(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen);
int sys_getsockname(int sockfd, struct sockaddr *my_addr, socklen_t *addrlen);
int sys_getsockopt(int sockfd, unsigned int flags, void *optval, socklen_t *optlen);
int sys_setsockopt(int sockfd, unsigned int flags, const void *optval, socklen_t optlen);
int sys_ioctlsocket(int sockfd, int request, void *arg);
int sys_select(int maxfdp, struct _sockfd_set *fd_sets, struct timeval *timeout);
int sys_dns_setserver(uint8_t numdns, const char *str);

#include <xbook/list.h>
#include <types.h>
#include <xbook/driver.h>

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

#endif  /* _XBOOK_PLUGIN_NET_H */
