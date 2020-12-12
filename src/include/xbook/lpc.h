#ifndef _XBOOK_LPC_H
#define _XBOOK_LPC_H

#include "list.h"
#include "spinlock.h"
#include "waitqueue.h"
#include "task.h"
#include "semaphore.h"
#include "msgpool.h"
#include <stdint.h>
#include <stddef.h>
#include <types.h>

#define LPC_PORT_NAME_LEN   32

#define LPC_MAX_CONNECT_NR   64

#define LPC_MAX_MESSAGE_LEN   256

#define LPC_MAX_MESSAGE_NR   32

/* port type */
#define LPC_PORT_TYPE_SERVER_CONNECTION         1  
#define LPC_PORT_TYPE_SERVER_COMMUNICATION      2  
#define LPC_PORT_TYPE_CLIENT_COMMUNICATION      3  
#define LPC_PORT_TYPE_MASK  0xf  

#define LPC_CONNECT_TIMEOUT   (1000 * 3)
#define LPC_CONNECT_SLEEP_MS   200

typedef enum {
    LPC_PORT_CREATED        = 0,
    LPC_PORT_LISTEN,        /* 服务端监听连接 */
    LPC_PORT_ACCEPT,        /* 处于接收连接中 */
    LPC_PORT_CONNECTED,     /* 已经连接上 */
    LPC_PORT_ACK,     /* 已经连接上 */
    LPC_PORT_DISCONNECTED,  
} lpc_port_state_t;


/* message has different type for different use. */
typedef struct {
    uint32_t id;
    uint32_t type;
    uint32_t size;
    uint8_t data[LPC_MAX_MESSAGE_LEN]; 
} lpc_message_t;

/* local process communication */
typedef struct lpc_port {
    struct lpc_port *port;      /* 连接端口指向自己，服务端口指向连接端口 */
    list_t list;        /* on lpc port list */
    uint32_t id;   
    task_t *task;       /* 通信端口指向对方端口的任务，连接端口随着状态改变而改变 */
    task_t *server;
    task_t *client;
    semaphore_t sema;   /* 连接端口的信号量 */
    lpc_port_state_t state;
    spinlock_t lock;
    msgpool_t *msgpool;
    lpc_message_t *msg;
    /* 0-3：端口类型 */     
    uint32_t flags;        
    char *name;
    /* 消息池，通信端口才有消息池 */
    uint32_t max_connects;
    uint32_t max_msgsz;
} lpc_port_t;

lpc_port_t *lpc_create_port(char *name, uint32_t max_connects, uint32_t max_msgsz, uint32_t flags);
int lpc_destroy_port(lpc_port_t *port);
lpc_port_t *lpc_accept_port(lpc_port_t *port, bool isaccept, void *addr);
lpc_port_t *lpc_connect_port(char *name, uint32_t *max_msgsz, void *addr);

int lpc_request_port(lpc_port_t *port, lpc_message_t *msg);
int lpc_request_wait_reply_port(lpc_port_t *port, lpc_message_t *msgin, lpc_message_t *msgout);

int lpc_reply_port(lpc_port_t *port, lpc_message_t *msg);
int lpc_reply_wait_receive_port(lpc_port_t *port, lpc_message_t *msgin, lpc_message_t *msgout);
int lpc_reply_wait_receive_port_timeout(lpc_port_t *port, lpc_message_t *msgin, lpc_message_t *msgout);
int lpc_reply_wait_reply_port(lpc_port_t *port, lpc_message_t *msg, lpc_message_t *msgout);

int lpc_send_port(lpc_port_t *port, void *msg, size_t size);
int lpc_recv_port(lpc_port_t *port, void *msg, size_t *size);
int lpc_send_and_reply_port(lpc_port_t *port, void *msg, size_t size);
int lpc_recv_and_reply_port(lpc_port_t *port, void *msg, size_t *size);

void lpc_init();

#endif /* _XBOOK_LPC_H */