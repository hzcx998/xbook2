#ifndef _XBOOK_NET_H
#define _XBOOK_NET_H

#include <types.h>
#include "mutexlock.h"
#include "msgpool.h"

/* 网络服务结构体，记录网络服务进程信息 */
typedef struct {
    mutexlock_t lock;   /* 访问成员的锁 */
    int active;         /* 服务是否被激活 */
    pid_t service_group;   /* 服务组，只能由该组来处理服务 */
    msgpool_t *msgpool; /* 消息池，用来处理请求 */
} net_service_t;

typedef struct {
    unsigned int unused;  
} service_packet_t;

/* 服务操作接口 */
int sys_bind_service(int service);
int sys_unbind_service(int service);
int sys_pick_service(int service, service_packet_t *serv_pack);
int sys_complete_service(int service, service_packet_t *serv_pack);

int sys_servcall(int servop, void *arg);

#endif  /* _XBOOK_NET_H */