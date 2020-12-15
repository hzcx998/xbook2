#ifndef _XBOOK_SERVCALL_H
#define _XBOOK_SERVCALL_H

#include "msgpool.h"
#include "semaphore.h"
#include <stdint.h>

#define SERVPORT_NR 32
#define SERVMSG_SIZE (4096 - sizeof(size_t))
#define SERVMSG_NR 8

#define BAD_SERVPORT(port) ((port) >= SERVPORT_NR)

enum servport_flags {
    SERVPORT_USING = 0X01,
};
/* 每个任务只能绑定一个服务 */
typedef struct {
    int my_port;
    spinlock_t lock;
    msgpool_t *msgpool; 
    uint32_t flags;
} servport_t;

/* 消息结构 */
typedef struct {
    uint32_t port;
    uint32_t id;
    size_t size;
    uint8_t data[SERVMSG_SIZE];
} servmsg_t;

/**
 * port: 要绑定的端口号，如果为负，则绑定一个可用的最小端口号
 */
int servport_bind(int port);

/**
 * port: 要解绑的端口号，如果为负，则解绑当前任务绑定的
 */
int servport_unbind(int port);

int servport_request(uint32_t port, servmsg_t *msg);
int servport_receive(int port, servmsg_t *msg);
int servport_reply(int port, servmsg_t *msg);

void servcall_init();

#endif /* _XBOOK_SERVCALL_H */