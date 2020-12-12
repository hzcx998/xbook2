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
static uint32_t lpc_msg_next_id = 0;


uint32_t lpc_generate_msg_id()
{
    return lpc_msg_next_id++;
}
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

    if (!max_connects || max_connects > LPC_MAX_CONNECT_NR)
        max_connects = LPC_MAX_CONNECT_NR;
    port->max_connects = max_connects;

    if (!max_msgsz || max_msgsz > LPC_MAX_MESSAGE_LEN)
        max_msgsz = LPC_MAX_MESSAGE_LEN;
    port->max_msgsz = max_msgsz;

    if (port_type == LPC_PORT_TYPE_SERVER_CONNECTION) {
        port->name = mem_alloc(LPC_PORT_NAME_LEN);
        if (!port->name) {
            errprint("lpc create: mem alloc for new port name %s failed!" endl, name);
            mem_free(port);
            return NULL;
        }
        memset(port->name, 0, LPC_PORT_NAME_LEN);
        memcpy(port->name, name, min(strlen(name), LPC_PORT_NAME_LEN - 1));
        port->msgpool = NULL;
        semaphore_init(&port->sema, 1);
    } else {
        port->name = NULL;
        port->msgpool = msgpool_create(port->max_msgsz + sizeof(lpc_message_t), LPC_MAX_MESSAGE_NR);
        if (!port->msgpool) {
            errprint("lpc create: create messagepool for commmunication port name %s failed!" endl, name);
            mem_free(port);
            return NULL;
        }
        /* 通信端口信号量 */
        semaphore_init(&port->sema, 1);
    }
    
    spinlock_init(&port->lock);
    port->id = lpc_port_next_id++;
    port->state = LPC_PORT_CREATED;
    port->flags = flags;
    port->port = port;
    port->task = task_current;
    port->server = NULL;
    port->client = NULL;
    port->msg = NULL;
    
    unsigned long iflags;
    spin_lock_irqsave(&lpc_port_list_lock, iflags);
    list_add_tail(&port->list, &lpc_port_list_head);
    spin_unlock_irqrestore(&lpc_port_list_lock, iflags);

    return port;
}

int lpc_destroy_port(lpc_port_t *port)
{
    if (!port)
        return -1;
    unsigned long iflags;
    spin_lock_irqsave(&port->lock, iflags);
    int type = port->flags & LPC_PORT_TYPE_MASK;
    port->state = LPC_PORT_DISCONNECTED;
    spin_unlock_irqrestore(&port->lock, iflags);
    if (type == LPC_PORT_TYPE_SERVER_CONNECTION) {
        // 释放信号量
        while (atomic_get(&port->sema.counter) > 0) {
            semaphore_up(&port->sema);
        }
        mem_free(port->name);
    }
    spin_lock_irqsave(&lpc_port_list_lock, iflags);
    list_del_init(&port->list); 
    spin_unlock_irqrestore(&lpc_port_list_lock, iflags);
    
    /* TODO: 服务端口不立即释放，需要等待所以连接客户端都响应完毕后才会释放。 */
    mem_free(port);
    return 0;
}

static void lpc_connect_exit_hook(void *arg)
{
    lpc_port_t *port = (lpc_port_t *) arg;
    semaphore_up(&port->sema);
}

lpc_port_t *lpc_connect_port(char *name, uint32_t *max_msgsz, void *addr)
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

    if (port->state == LPC_PORT_DISCONNECTED) {
        errprint("lpc connect: server %d disconnected!\n", port->id);
        return NULL;
    }

    task_t *cur = task_current;
    lpc_port_t *comm_port = NULL;
    while (1) {
        semaphore_down(&port->sema);
        if (port->state == LPC_PORT_LISTEN) {
            /* 创建客户端通信端口 */
            comm_port = lpc_create_port("unnamed", 0, port->max_msgsz, LPC_PORT_TYPE_CLIENT_COMMUNICATION);
            if (comm_port == NULL) {
                errprint("lpc connect: create communication port failed!\n");
                semaphore_up(&port->sema);
                break;
            }
            /* 创建客户端通信端口，唤醒服务线程，并等待其接收请求 */
            dbgprint("lpc connect: server port is LISTEN.\n");
            unsigned long iflags;
            spin_lock_irqsave(&port->lock, iflags);
            port->state = LPC_PORT_ACCEPT;
            port->port = comm_port;
            dbgprint("lpc connect: client sleep on connect list.\n");
            ASSERT(port->task);
            task_unblock(port->task);
            port->task = cur;
            spin_unlock_irqrestore(&port->lock, iflags);
            /* 如果在阻塞期间被中断导致退出，那么需要设置钩子函数释放信号量 */
            cur->exit_hook = lpc_connect_exit_hook;
            cur->exit_hook_arg = port;
            task_block(TASK_BLOCKED);
            cur->exit_hook = NULL;
            cur->exit_hook_arg = NULL;
            dbgprint("lpc connect: client was wakeup\n");
            if (max_msgsz)
                *max_msgsz = comm_port->max_msgsz;
            comm_port->msg = mem_alloc(sizeof(lpc_message_t));
            if (!comm_port->msg) {
                lpc_destroy_port(comm_port);
                errprint("lpc connect: server not accept!\n");
                comm_port = NULL;
            } else if (comm_port->state != LPC_PORT_CONNECTED) {
                mem_free(comm_port->msg);
                lpc_destroy_port(comm_port);
                errprint("lpc connect: server not accept!\n");
                comm_port = NULL;
            }
            /* 服务端会释放信号量 */
            break;
        } else {
            semaphore_up(&port->sema);
            /* 端口在销毁中，不能进行连接 */
            if (port->state == LPC_PORT_DISCONNECTED) {
                errprint("lpc connect: server %d disconnected!\n", port->id);
                break;
            }
        }
    }
    return comm_port;
}

static void lpc_accept_exit_hook(void *arg)
{
    lpc_port_t *port = (lpc_port_t *) arg;
    task_unblock(port->task);
    port->task = NULL;
    if (port->port) {   /* 执行的是服务端的通信端口 */
        lpc_destroy_port(port->port);
        port->port = NULL;
    }
}

lpc_port_t *lpc_accept_port(lpc_port_t *port, bool isaccept, void *addr)
{
    if (!port)
        return NULL;
    if (port->state != LPC_PORT_CREATED) {
        errprint("lpc accept: port %d must be WAKEUP!\n", port->id);
        return NULL;
    }
    task_t *cur = task_current;
    unsigned long iflags;
    while (1) {
        spin_lock_irqsave(&port->lock, iflags);
        port->state = LPC_PORT_LISTEN;
        port->task = cur;
        port->port = port;
        spin_unlock_irqrestore(&port->lock, iflags);
        task_block(TASK_BLOCKED);
        /* TODO: 检查是正常唤醒还是被强制打断 */
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
            port->port = NULL;
        } else {
            comm_port->msg = mem_alloc(sizeof(lpc_message_t));
            if (!comm_port->msg) {
                lpc_destroy_port(comm_port);
                errprint("lpc accept: create communication port failed!\n");
                port->port = NULL;
                comm_port = NULL;
            } else {
                /* 端口指向客户端的通信端口 */
                lpc_port_t *client_port = port->port;
                /* 通信端口的绑定为客户端 */
                comm_port->port = client_port;
                comm_port->state = LPC_PORT_CONNECTED;
                client_port->port = comm_port;
                client_port->state = LPC_PORT_CONNECTED;
                spin_lock_irqsave(&port->lock, iflags);
                port->port = comm_port;  /* 绑定服务端的通信端口 */
                spin_unlock_irqrestore(&port->lock, iflags);  
            }
        }
    }
    dbgprint("lpc accept: wakeup client\n");
    port->state = LPC_PORT_CREATED; // 恢复初始状态
    /* 注册一个钩子函数，来保证异常退出时也能唤醒客户端 */
    cur->exit_hook = lpc_accept_exit_hook;
    cur->exit_hook_arg = port;
    ASSERT(port->task);
    TASK_NEED_STATE(port->task, TASK_BLOCKED);
    cur->exit_hook = NULL;
    cur->exit_hook_arg = NULL;
    task_unblock(port->task);
    port->task = NULL;
    dbgprint("lpc accept: wakeup done\n");
    semaphore_up(&port->sema);
    return comm_port;
}

void lpc_copy_message(lpc_message_t *dest, lpc_message_t *src)
{
    if (src->size > LPC_MAX_MESSAGE_LEN)
        src->size = LPC_MAX_MESSAGE_LEN;
    dest->id = src->id;
    dest->size = src->size;
    dest->type = src->type;
    memcpy(dest->data, src->data, src->size);
}

void lpc_reset_message(lpc_message_t *msg)
{
    memset(msg, 0, sizeof(lpc_message_t));
}

int lpc_receive_port(lpc_port_t *port, lpc_message_t *lpc_msg)
{
    if (!port)
        return -EINVAL;
    if (port->state != LPC_PORT_CONNECTED) {
        //errprint("lpc recv: port %d must be connected!\n", port->id);
        return -EPERM;
    }
    if ((port->flags & LPC_PORT_TYPE_MASK) == LPC_PORT_TYPE_SERVER_CONNECTION) {
        errprint("lpc recv: port %d must be communication port!\n", port->id);
        return -EPERM;
    }
    /* 进入监听状态，等待被唤醒 */
    semaphore_down(&port->sema);
    //infoprint("lpc recv: listen\n");
    port->server = task_current;
    port->state = LPC_PORT_LISTEN;
    //infoprint("lpc recv: no req, block\n");
    semaphore_up(&port->sema);
    task_block(TASK_BLOCKED);
    /* 唤醒后读取数据 */
    semaphore_down(&port->sema);
    //infoprint("lpc recv: wakeup\n");
    ASSERT(port->state == LPC_PORT_ACCEPT);
    // TODO: 复制参数到消息中
    lpc_copy_message(lpc_msg, port->msg);
    semaphore_up(&port->sema);
    
    /* 返回 */
    return 0;
}

/*
消息：长消息，短消息。
短消息就和消息一起发送过去了。
长消息需要用共享内存映射。

如果提前建立了共享内存，那么收到长消息，就可以直接通过长内存地址读取数据。

lpc_msg->id = 1;
lpc_msg->size = 10;
lpc_msg->data 
根据消息大小，选择消息的类型。
*/

int lpc_request_port(lpc_port_t *port, lpc_message_t *lpc_msg)
{
    if (!port || !lpc_msg)
        return -EINVAL;
    if (port->state != LPC_PORT_CONNECTED) {
        // errprint("lpc requset: port %d must be connected!\n", port->id);
        return -EPERM;
    }
    if ((port->flags & LPC_PORT_TYPE_MASK) == LPC_PORT_TYPE_SERVER_CONNECTION) {
        errprint("lpc requset: port %d must be communication port!\n", port->id);
        return -EPERM;
    }
    lpc_port_t *other_port = port->port;
    /* make sure another comm port be ture */
    ASSERT(other_port);

    uint32_t msgid = lpc_generate_msg_id();
    lpc_msg->id = msgid;
    while (1) {    
        semaphore_down(&other_port->sema);
        /* 是监听才进行请求 */
        if (other_port->state == LPC_PORT_LISTEN) {
            //infoprint("lpc request: start\n");
            lpc_copy_message(other_port->msg, lpc_msg);
            /* 改变状态，并唤醒对方 */
            ASSERT(other_port->state == LPC_PORT_LISTEN);
            other_port->state = LPC_PORT_ACCEPT; /* 接收数据 */
            other_port->client = task_current;
            TASK_NEED_STATE(other_port->server, TASK_BLOCKED);
            task_unblock(other_port->server);
            //infoprint("lpc request: wakeup server\n");

            /* 如果没有应答，那么就需要阻塞等待被唤醒 */
            if (other_port->state != LPC_PORT_ACK) {
                /*
                infoprint("lpc request: wait for ack\n");
                infoprint("lpc request: port %d task state %d\n", 
                        other_port->state, other_port->server->state);*/
                semaphore_up(&other_port->sema);
                task_block(TASK_BLOCKED);
                semaphore_down(&other_port->sema);    
            }
            lpc_copy_message(lpc_msg, other_port->msg);
            
            //infoprint("lpc request: after wait\n");
            ASSERT(other_port->state == LPC_PORT_ACK);
            other_port->state = LPC_PORT_CONNECTED; /* 恢复连接状态 */
            
            /* TODO: 调用者如果阻塞了，就需要唤醒 */
            if (other_port->server->state == TASK_BLOCKED) {
                //infoprint("lpc request: wakeup server\n");
                task_unblock(other_port->server);
            }
            other_port->server = NULL;
            semaphore_up(&other_port->sema);
            break;
        }
        semaphore_up(&other_port->sema);
    }
    if (lpc_msg->id != msgid) { // invalid msg
        return -1;
    }
    return 0;
}

int lpc_reply_port(lpc_port_t *port, lpc_message_t *lpc_msg)
{
    if (!port)
        return -EINVAL;
    if (port->state != LPC_PORT_ACCEPT) {
        errprint("lpc requset: port %d must be accept!\n", port->id);
        return -EPERM;
    }
    if ((port->flags & LPC_PORT_TYPE_MASK) == LPC_PORT_TYPE_SERVER_CONNECTION) {
        errprint("lpc requset: port %d must be communication port!\n", port->id);
        return -EPERM;
    }

    semaphore_down(&port->sema);
    port->state = LPC_PORT_ACK;

    /* TODO: 发送应答消息 */
    lpc_copy_message(port->msg, lpc_msg);
            
    if (port->client) {
        if (port->client->state == TASK_BLOCKED) {
            task_unblock(port->client);
        }
        port->client = NULL;
    }

    semaphore_up(&port->sema);
    if (port->state == LPC_PORT_ACK) {
        task_block(TASK_BLOCKED);
    }
    /* 确保状态为连接 */
    ASSERT(port->state == LPC_PORT_CONNECTED);

    return 0;
}

#define BOTH 0 

void lpc_server(void *arg)
{
    kprint("lpc server start.\n");
    lpc_port_t *port = lpc_create_port("lpc_comm0", 10, 5, LPC_PORT_TYPE_SERVER_CONNECTION);
    if (port == NULL)
        panic("create port failed!\n");    
    int isaccept = 1; 
    lpc_port_t *server_comm = lpc_accept_port(port, isaccept, NULL);
    if (server_comm)
        infoprint("server: accept %d success!\n", server_comm->id);   
    ASSERT(server_comm);
    infoprint("server: accept state %d\n", server_comm->state);   
    
    uint8_t buf[32];
    size_t buflen;
    lpc_message_t msg;
    while (1) {
        #if BOTH
        if (!lpc_receive_port_and_reply_port(server_comm, buf, &buflen)) {
            infoprint("server: recv %d ok!\n", buflen);
        }
        #else
        lpc_reset_message(&msg);
        if (!lpc_receive_port(server_comm, &msg))
            infoprint("server: recv %d success!\n", server_comm->id);   

        /* 处理数据 */
        if (msg.size > 0) {
            dbgprint("server: recv %s\n", msg.data);
        }
        
        char *str = "hello, client!\n";
        memset(msg.data, 0, LPC_MAX_MESSAGE_LEN);
        strcpy(msg.data, str);
        msg.size = strlen(str);
        if (!lpc_reply_port(server_comm, &msg))
            infoprint("server: reply %d success!\n", server_comm->id);   

        #endif
    }
}

void lpc_client_a(void *arg)
{
    kprint("lpc client a start.\n");
    uint32_t max_msgsz;
    lpc_port_t *port = lpc_connect_port("lpc_comm0", &max_msgsz, NULL);
    if (port) {
        infoprint("client A: connect %d success!\n", port->id);
        
        // lpc_destroy_port(port);
    }
    ASSERT(port);
    infoprint("client A: connect %d success with %d!\n", port->id, max_msgsz);

    uint8_t buf[32]; 
    lpc_message_t msg;
    
    while (1) {
        #if BOTH
        if (!lpc_send_and_reply_port(port, buf, 32)) {
            infoprint("client A: send port ok!\n");
        }
        #else
        lpc_reset_message(&msg);
        char *str = "hello, lpc!\n";
        msg.size = strlen(str);
        strcpy(msg.data, str);
        if (!lpc_request_port(port, &msg))
            infoprint("client A: request %d done!\n", port->id);
        if (msg.size > 0) {
            dbgprint("client: echo %s\n", msg.data);
        }
        #endif
    }
}

void lpc_client_b(void *arg)
{
    kprint("lpc client b start.\n");
    lpc_port_t *port = lpc_connect_port("lpc_comm0", NULL, NULL);
    if (port) {
        infoprint("client B: connect %d success!\n", port->id);
        lpc_destroy_port(port);
    }
    
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