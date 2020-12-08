#include <xbook/lpc.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/clock.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>

static LIST_HEAD(lpc_port_list_head);
static DEFINE_SPIN_LOCK_UNLOCKED(lpc_port_list_lock);
static uint32_t lpc_port_next_id = 0;

lpc_port_t *lpc_find_port_by_name(char *name)
{
    lpc_port_t *port = NULL, *tmp;
    unsigned long iflags;
    spin_lock_irqsave(&lpc_port_list_lock, iflags);
    list_for_each_owner (tmp, &lpc_port_list_head, list) {
        if (tmp->name) {
            if (!strcmp(tmp->name, name)) {
                port = tmp;
                break;
            }
        }
    }
    spin_unlock_irqrestore(&lpc_port_list_lock, iflags);
    return port;
}

lpc_port_t *lpc_create_port(char *name, uint32_t max_connects, uint32_t max_msgsz, uint32_t flags)
{
    if (!name)
        return NULL;
    int port_type = flags & LPC_PORT_TYPE_MASK;
    if (port_type == LPC_PORT_TYPE_SERVER_CONNECTION) {
        if (lpc_find_port_by_name(name)) {
            errprint("lpc create: port name %s has existed!" endl, name);
            return NULL;
        }
    } 
    lpc_port_t *port = mem_alloc(sizeof(lpc_port_t));
    if (!port) {
        errprint("lpc create: mem alloc for new port failed!" endl);
        return NULL;
    }
    if (port_type == LPC_PORT_TYPE_SERVER_CONNECTION) {
        port->name = mem_alloc(LPC_PORT_NAME_LEN);
        if (!port->name) {
            errprint("lpc create: mem alloc for new port name %s failed!" endl, name);
            mem_free(port);
            return NULL;
        }
        memset(port->name, 0, LPC_PORT_NAME_LEN);
        memcpy(port->name, name, min(strlen(name), LPC_PORT_NAME_LEN - 1));
    } else {
        port->name = NULL;
    }
    
    if (!max_connects || max_connects > LPC_MAX_CONNECT_NR)
        max_connects = LPC_MAX_CONNECT_NR;
    port->max_connects = max_connects;

    if (!max_msgsz || max_msgsz > LPC_MAX_MESSAGE_DATA_LEN)
        max_msgsz = LPC_MAX_MESSAGE_DATA_LEN;

    spinlock_init(&port->lock);
    wait_queue_init(&port->pending_list);
    port->max_msgsz = max_msgsz;
    port->id = lpc_port_next_id++;
    port->state = LPC_PORT_CREATED;
    port->flags = flags;
    port->port = port;
    port->server_task = NULL;
    port->client_task = NULL;
    unsigned long iflags;
    spin_lock_irqsave(&lpc_port_list_lock, iflags);
    list_add_tail(&port->list, &lpc_port_list_head);
    spin_unlock_irqrestore(&lpc_port_list_lock, iflags);

    return port;
}

lpc_port_t *lpc_connect_port(char *name, uint32_t *max_msgsz)
{
    if (!name)
        return NULL;
    lpc_port_t *port = lpc_find_port_by_name(name);
    if (!port) {
        errprint("lpc connect: port %s not exist!\n", name);
        return NULL;
    }
    if ((port->flags & LPC_PORT_TYPE_MASK) != LPC_PORT_TYPE_SERVER_CONNECTION) {
        errprint("lpc connect: port %s not CONNECTION!\n", name);
        return NULL;
    }
    while (1) {
        if (port->state == LPC_PORT_LISTEN) {
            /* 创建客户端通信端口 */
            lpc_port_t *comm_port = lpc_create_port("unnamed", 0, port->max_msgsz, LPC_PORT_TYPE_CLIENT_COMMUNICATION);
            if (comm_port == NULL) {
                errprint("lpc connect: create communication port failed!\n");
                return NULL;
            }
            /* 创建客户端通信端口，唤醒服务线程，并等待其接收请求 */
            dbgprint("lpc connect: server port is LISTEN.\n");
            unsigned long iflags;
            spin_lock_irqsave(&port->lock, iflags);
            port->state = LPC_PORT_ACCEPT;
            port->port = comm_port;
            dbgprint("lpc connect: client sleep on connect list.\n");
            ASSERT(port->server_task);
            task_unblock(port->server_task);
            port->server_task = NULL;
            port->client_task = task_current;
            spin_unlock_irqrestore(&port->lock, iflags);
            /* 阻塞自己，服务端完成请求后才唤醒自己 */
            task_block(TASK_BLOCKED);
            /* 对通信端口的连接状态做判断 */
            if (comm_port->state != LPC_PORT_CONNECTED) {
                /* 销毁端口 */

                errprint("lpc connect: server not accept failed!\n");
                comm_port = NULL;
            }
            /* 服务端完成请求后 */
            return comm_port;
        } else {
            /* 当服务连接端口不是处于监听，就加到未决队列中 */
            wait_queue_sleepon(&port->pending_list);
        }
    }
    errprint("lpc connect: server not listen, timeout." endl);
    return NULL;
}

lpc_port_t *lpc_accept_port(lpc_port_t *port, bool isaccept)
{
    if (!port)
        return NULL;
    if (port->state != LPC_PORT_CREATED) {
        errprint("lpc accept: port %d must be WAKEUP!\n", port->id);
        return NULL;
    }
    unsigned long iflags;
    spin_lock_irqsave(&port->lock, iflags);
    port->state = LPC_PORT_LISTEN;
    spin_unlock_irqrestore(&port->lock, iflags);
    while (1) {
        /* 查看连接队列是否有服务端 */
        if (wait_queue_length(&port->pending_list) > 0)
            wait_queue_wakeup(&port->pending_list);
        // TODO: 用互斥锁或者信号量解决不连贯问题
        port->server_task = task_current;
        task_block(TASK_BLOCKED);
        if (port->state == LPC_PORT_ACCEPT) {
            break;
        }
    }
    lpc_port_t *comm_port = NULL;
    if (isaccept) {
        /* 创建一个新的服务端通信端口 */
        comm_port = lpc_create_port("unnamed", 0, port->max_msgsz, LPC_PORT_TYPE_SERVER_COMMUNICATION);
        if (comm_port == NULL) {
            errprint("lpc accept: create communication port failed!\n");
        } else {
            lpc_port_t *client_port = port->port;
            /* 通信端口的绑定为客户端 */
            comm_port->port = client_port;
            comm_port->state = LPC_PORT_CONNECTED;
            client_port->port = comm_port;
            client_port->state = LPC_PORT_CONNECTED;
        }
        unsigned long iflags;
        spin_lock_irqsave(&port->lock, iflags);
        port->port = port;  /* 连接端口恢复绑定自己 */
        spin_unlock_irqrestore(&port->lock, iflags);  
    }
    port->state = LPC_PORT_CREATED; // 恢复初始状态
    /* 唤醒客户端 */
    ASSERT(port->client_task);
    TASK_NEED_STATE(port->client_task, TASK_BLOCKED);
    task_unblock(port->client_task);
    port->client_task = NULL;
    return comm_port;
}

void lpc_server(void *arg)
{
    kprint("lpc server start.\n");

    lpc_port_t *port = lpc_create_port("lpc_comm0", 10, 32, LPC_PORT_TYPE_SERVER_CONNECTION);
    if (port == NULL)
        panic("create port failed!\n");     
    while (1) {
        lpc_port_t *server_comm = lpc_accept_port(port, 1);
        if (server_comm)
            infoprint("server: accept %d success!\n", server_comm->id);    
    }
}

void lpc_client_a(void *arg)
{
    kprint("lpc client a start.\n");
    lpc_port_t *port = lpc_connect_port("lpc_comm0", NULL);
    if (port)
        infoprint("client A: connect %d success!\n", port->id);
    while (1) {
        
    }
}

void lpc_client_b(void *arg)
{
    kprint("lpc client b start.\n");
    lpc_port_t *port = lpc_connect_port("lpc_comm0", NULL);
    if (port)
        infoprint("client B: connect %d success!\n", port->id);
    
    while (1) {
        
    }
}

void lpc_init()
{
    
    infoprint("lpc init done\n");

    kern_thread_start("lpc_server", 0, lpc_server, NULL);
    kern_thread_start("lpc_client_a", 0, lpc_client_a, NULL);
    kern_thread_start("lpc_client_b", 0, lpc_client_b, NULL);
    
}