#ifndef _XBOOK_PORT_COMM_H
#define _XBOOK_PORT_COMM_H

#include "msgpool.h"
#include "semaphore.h"
#include <stdint.h>

#define PORT_COMM_NR 32
#define PORT_COMM_UNNAMED_START (PORT_COMM_NR / 4)

#define PORT_MSG_HEADER_SIZE (4 * sizeof(uint32_t))
#define PORT_MSG_SIZE (4096 - PORT_MSG_HEADER_SIZE)
#define PORT_MSG_NR 8

#define PORT_COMM_RETRY_GET_CNT  10

#define BAD_PORT_COMM(port) ((port) >= PORT_COMM_NR)

enum port_comm_flags {
    PORT_COMM_USING = 0X01,
};
/* 每个任务只能绑定一个服务 */
typedef struct {
    int my_port;
    spinlock_t lock;
    msgpool_t *msgpool; 
    uint32_t flags;
} port_comm_t;

/* 消息结构 */
typedef struct {
    uint32_t port;
    uint32_t id;
    uint32_t code;
    uint32_t size;
    uint8_t data[PORT_MSG_SIZE];
} port_msg_t;

/**
 * port: 要绑定的端口号，如果为负，则绑定一个可用的最小端口号
 */
int sys_port_comm_bind(int port);

/**
 * port: 要解绑的端口号，如果为负，则解绑当前任务绑定的
 */
int sys_port_comm_unbind(int port);

int sys_port_comm_request(uint32_t port, port_msg_t *msg);
int sys_port_comm_receive(int port, port_msg_t *msg);
int sys_port_comm_reply(int port, port_msg_t *msg);

void port_comm_init();

#endif /* _XBOOK_PORT_COMM_H */