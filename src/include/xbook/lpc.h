#ifndef _XBOOK_LPC_H
#define _XBOOK_LPC_H

#include "list.h"
#include "spinlock.h"
#include "waitqueue.h"
#include "task.h"
#include "semaphore.h"
#include <stdint.h>
#include <stddef.h>
#include <types.h>

#define LPC_PORT_NAME_LEN   32

#define LPC_MAX_CONNECT_NR   64

#define LPC_MAX_MESSAGE_DATA_LEN   256

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
    LPC_PORT_DISCONNECTED,  
} lpc_port_state_t;


/* local process communication */
typedef struct lpc_port {
    struct lpc_port *port;      /* 连接端口指向自己，服务端口指向连接端口 */
    list_t list;        /* on lpc port list */
    uint32_t id;   
    task_t *server_task;    /* 服务端口任务 */
    task_t *client_task;    /* 客户端任务 */
    semaphore_t connect_sema;   /* 连接端口的信号量 */
    lpc_port_state_t state;
    spinlock_t lock;
    /* 0-3：端口类型 */     
    uint32_t flags;        
    char *name;
    /* 消息池，通信端口才有消息池 */
    uint32_t max_connects;
    uint32_t max_msgsz;
} lpc_port_t;

/* message has different type for different use. */
typedef struct {
    list_t list;
    void *sender_port;

} lpc_message_t;

lpc_port_t *lpc_create_port(char *name, uint32_t max_connects, uint32_t max_msgsz, uint32_t flags);
int lpc_destroy_port(lpc_port_t *port);
lpc_port_t *lpc_accept_port(lpc_port_t *port, bool isaccept);
lpc_port_t *lpc_connect_port(char *name, uint32_t *max_msgsz);
void lpc_init();

#endif /* _XBOOK_LPC_H */