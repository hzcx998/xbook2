#ifndef _XBOOK_PORT_COMM_H
#define _XBOOK_PORT_COMM_H

#include "msgpool.h"
#include "semaphore.h"
#include <arch/atomic.h>
#include <stdint.h>

#define PORT_COMM_NR 32
#define PORT_COMM_UNNAMED_START (PORT_COMM_NR / 4)

#define PORT_MSG_NR 8

#define PORT_COMM_RETRY_GET_CNT  10

#define BAD_PORT_COMM(port) ((port) >= PORT_COMM_NR)

/* 知名端口号 */
enum {
    PORT_COMM_TEST = 0,
    PORT_COMM_NET,
    PORT_COMM_GRAPH,
    PORT_COMM_LAST = 8
};


enum port_comm_flags {
    PORT_COMM_USING = 0X01,
    PORT_COMM_GROUP = 0X02, /* 端口可以和一组进程通信 */
};

/* 端口绑定标志 */
enum {
    PORT_BIND_GROUP = 0x01, /* 端口绑定时为组端口 */
    PORT_BIND_ONCE  = 0x02,     /* 端口绑定时只绑定一次，多次绑定还是返回成功 */
};

/* 每个任务只能绑定一个服务 */
typedef struct {
    int my_port;
    spinlock_t lock;
    msgpool_t *msgpool; 
    uint32_t flags;
    atomic_t reference; /* 绑定的服务端口数量 */
} port_comm_t;

typedef struct {
    uint32_t port;
    uint32_t id;
    uint32_t size;
} port_msg_header_t;

#define PORT_MSG_HEADER_SIZE (sizeof(port_msg_header_t))
#define PORT_MSG_SIZE (4096 - PORT_MSG_HEADER_SIZE)

/* 消息结构 */
typedef struct {
    port_msg_header_t header;
    uint8_t data[PORT_MSG_SIZE];
} port_msg_t;

/**
 * port: 要绑定的端口号，如果为负，则绑定一个可用的最小端口号
 */
int sys_port_comm_bind(int port, int flags);

/**
 * port: 要解绑的端口号，如果为负，则解绑当前任务绑定的
 */
int sys_port_comm_unbind(int port);

int sys_port_comm_request(uint32_t port, port_msg_t *msg);
int sys_port_comm_receive(int port, port_msg_t *msg);
int sys_port_comm_reply(int port, port_msg_t *msg);

void port_comm_init();

void port_msg_reset(port_msg_t *msg);
void port_msg_copy_header(port_msg_t *src, port_msg_t *dest);

#endif /* _XBOOK_PORT_COMM_H */