#include <xbook/portcomm.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

static port_comm_t port_comm_table[PORT_COMM_NR];
DEFINE_SPIN_LOCK_UNLOCKED(port_comm_lock);
static uint32_t port_comm_msg_next_id = 0;

uint32_t port_comm_generate_msg_id()
{
    return port_comm_msg_next_id++;
}

port_comm_t *port_comm_alloc()
{
    unsigned long iflags;
    spin_lock_irqsave(&port_comm_lock, iflags);
    int i; for (i = PORT_COMM_UNNAMED_START; i < PORT_COMM_NR; i++) {
        port_comm_t *port_comm = &port_comm_table[i];
        if (!port_comm->flags) {
            port_comm->flags = PORT_COMM_USING;
            atomic_set(&port_comm->reference, 0);
            spinlock_init(&port_comm->lock);
            port_comm->msgpool = NULL;
            spin_unlock_irqrestore(&port_comm_lock, iflags);
            return port_comm;
        }
    }
    spin_unlock_irqrestore(&port_comm_lock, iflags);
    return NULL;
}

int port_comm_free(port_comm_t *port_comm)
{
    unsigned long iflags;
    spin_lock_irqsave(&port_comm_lock, iflags);
    int i; for (i = 0; i < PORT_COMM_NR; i++) {
        port_comm_t *port = &port_comm_table[i];
        if (port == port_comm && port->flags) {
            port->flags = 0;
            spin_unlock_irqrestore(&port_comm_lock, iflags);
            return i;
        }
    }
    spin_unlock_irqrestore(&port_comm_lock, iflags);
    return -1;
}

port_comm_t *port_comm_find(uint32_t port)
{
    unsigned long iflags;
    spin_lock_irqsave(&port_comm_lock, iflags);
    int i; for (i = 0; i < PORT_COMM_NR; i++) {
        port_comm_t *port_comm = &port_comm_table[i];
        if (port_comm->flags && i == port) {
            spin_unlock_irqrestore(&port_comm_lock, iflags);
            return port_comm;
        }
    }
    spin_unlock_irqrestore(&port_comm_lock, iflags);
    return NULL;
}

uint32_t port_comm_p2i(port_comm_t *port_comm)
{
    uint32_t index = port_comm - port_comm_table; 
    assert(index < PORT_COMM_NR);
    return index;
}

port_comm_t *port_comm_i2p(uint32_t port)
{
    assert(port < PORT_COMM_NR);
    port_comm_t *port_comm = port_comm_table + port; 
    return port_comm;
}

/**
 * 重新分配一个指定了port号的端口，分配时，进行初始化
 */
port_comm_t *port_comm_realloc(uint32_t port)
{
    unsigned long iflags;
    spin_lock_irqsave(&port_comm_lock, iflags);
    port_comm_t *port_comm = port_comm_i2p(port);
    port_comm->flags = PORT_COMM_USING;
    spinlock_init(&port_comm->lock);
    atomic_set(&port_comm->reference, 0);
    port_comm->msgpool = NULL;        
    spin_unlock_irqrestore(&port_comm_lock, iflags);
    return port_comm;
}

/**
 * port_comm_vertify - 验证端口并返回端口指针
 * 
 * 如果任务没有绑定端口，则返回错误
 * 如果port为正，那么就查找端口的地址并判断是否为当前任务的端口，成功则返回端口地址。
 * 不然则返回错误。
 * 如果port为负，那么就看当前任务是否绑定了端口，如果是则返回端口地址，不然则返回错误
 */ 
int port_comm_vertify(int port, port_comm_t **out_port_comm, task_t *task)
{
    if (!task->port_comm) {
        //errprint("port unbind: not port bound on task %d.\n", task->pid);
        return -EPERM;
    }
    port_comm_t *port_comm;
    if (port >= 0) {
        port_comm = port_comm_find(port);
        if (!port_comm) {
            errprint("port unbind: port %d not bounded.\n", port);
            return -EPERM;
        }

        if (atomic_get(&port_comm->reference) < 1) {
            errprint("port unbind: port %d comm reference error!\n", port);
            return -EPERM;
        }
        
        /* 不是组，就检测是否端口和当前任务是否是一对一绑定 */
        if (!(port_comm->flags & PORT_COMM_GROUP)) {
            if (port_comm != task->port_comm) {
                errprint("port unbind: port %d not bond on task %d.\n", port, task->pid);
                return -EPERM;                
            }
        }
    } else {    /* 端口为负，则验证当前任务绑定的端口 */
        if (!(task->port_comm && task->port_comm->flags)) {
            errprint("port unbind: not port bond on task %d.\n", task->pid);
            return -EPERM;
        }
        port_comm = task->port_comm;
    }
    *out_port_comm = port_comm;
    return 0;
}

static void msgpool_get_callback(msgpool_t *pool, void *buf)
{
    port_msg_header_t *mhead = (port_msg_header_t *)pool->tail;
    memcpy(buf, pool->tail, min(mhead->size, pool->msgsz));   /* copy data */
}

/**
 * 如果端口已经绑定，则直接返回错误
 * 当port为正的时候，查找端口地址，如果端口已经存在着返回错误，不然就分配一个新端口。
 * 当port为负时，直接分配一个新端口。
 */
static port_comm_t *__sys_port_comm_bind(int port, task_t *cur, int flags)
{
    port_comm_t *port_comm = NULL;
    if (cur->port_comm) {
        if (flags & PORT_BIND_ONCE) {   /* 带有只绑定一次的参数，多次绑定只进行一次绑定 */
            port_comm = cur->port_comm;
            goto bind_final;
        }
        
        errprint("port bind: port %d had bounded on task %d.\n", port_comm_p2i(cur->port_comm), cur->pid);
        return NULL;
    }
    if (port >= 0) {
        /* NOTE: 绑定端口时检测端口类型，如果是组端口，那么就可以返回该端口，组端口有引用计数机制 */
        port_comm = port_comm_find(port);
        if (port_comm) {
            //dbgprint("port bind: port %d not first bind.\n", port);
            if (flags & PORT_BIND_GROUP) {  /* 将端口绑定成组 */
                if (atomic_get(&port_comm->reference) < 1) {
                    errprint("port bind: port %d reference error!\n", port);
                    return NULL;
                }
                atomic_inc(&port_comm->reference);
                /*dbgprint("port bind: port=%d is group with reference=%d.\n",
                    port, atomic_get(&port_comm->reference));*/
            } else {
                errprint("port bind: port %d had used.\n", port);
                return NULL;
            }
        } else {    /* 端口没有找到，重新分配该端口 */
            //dbgprint("port bind: port %d first bind.\n", port);
            port_comm = port_comm_realloc(port);
            atomic_inc(&port_comm->reference);  /* 分配一个端口成功 */
        }
    } else {    /* 端口为负，则绑定一个尚未分配的端口 */
        port_comm = port_comm_alloc();
        if (!port_comm) {
            errprint("port bind: no port left.\n");
            return NULL;
        }
        atomic_inc(&port_comm->reference);  /* 分配一个端口成功 */
    }
bind_final:
    /* 如果是组，就加上端口组的标志 */
    if (flags & PORT_BIND_GROUP)
        port_comm->flags |= PORT_COMM_GROUP;

    return port_comm;
}

/* 任务绑定端口处理 */
#define TASK_BIND_PORT_COMM(task, port_comm) \
    do { \
        unsigned long __iflags; \
        spin_lock_irqsave(&(task)->lock, __iflags); \
        (task)->port_comm = (port_comm); \
        spin_unlock_irqrestore(&(task)->lock, __iflags); \
    } while(0)

/* 任务绑定端口处理 */
#define TASK_UNBIND_PORT_COMM(task, port_comm) \
    do { \
        unsigned long __iflags; \
        spin_lock_irqsave(&(task)->lock, __iflags); \
        if ((task)->port_comm != (port_comm)) \
            noteprint("task pid=%d unbind port comm not equal!\n", (task)->pid); \
        (task)->port_comm = NULL; \
        spin_unlock_irqrestore(&(task)->lock, __iflags); \
    } while(0)

/**
 * 绑定port指定的端口并创建消息池，如果消息池失败则返回错误
 * 最后把端口绑定到任务上返回端口值
 */
int sys_port_comm_bind(int port, int flags)
{
    if (BAD_PORT_COMM(port)) {
        dbgprint("port bind: port %d arg invalid\n", port);
        return -EINVAL;
    }
    task_t *cur = task_current;
    port_comm_t *port_comm = __sys_port_comm_bind(port, cur, flags);
    if (!port_comm) {
        errprint("port bind: port %d bind failed!\n", port);
        return -1;
    }
    
    /* 如果是有单次绑定标志，则判断引用计数是否为1，如果是则直接返回 */
    if (flags & PORT_BIND_ONCE) {
        if (cur->port_comm != NULL) {   /* 已经绑定了端口，则直接返回 */
            if (cur->port_comm != port_comm)
                errprint("port bind: task pid=%d port %d bind onece but not same port!\n",
                    cur->pid, port);        
            return 0;
        }
    }

    /* 如果有组标志，则检测一下端口组 */
    if (flags & PORT_BIND_GROUP) {
        /* 检测引用计数，如果大于1，则表明是组端口，就不用创建消息池了 */
        if (atomic_get(&port_comm->reference) > 1) {
            /* 当前任务绑定新的端口 */
            TASK_BIND_PORT_COMM(cur, port_comm);
            /*dbgprint("port bind: bind port=%d group reference=%d success.\n",
                port, atomic_get(&port_comm->reference));*/
            return 0;
        }
    }

    /* 分配新端口时，创建端口的消息池以及绑定端口 */
    int msgcnt = port < 0? 1 : PORT_MSG_NR;
    unsigned long iflags;
    spin_lock_irqsave(&port_comm->lock, iflags);
    port_comm->msgpool = msgpool_create(sizeof(port_msg_t), msgcnt);
    if (!port_comm->msgpool) {
        spin_unlock_irqrestore(&port_comm->lock, iflags);
        errprint("port bind: port %d create msgpool failed!\n", port);
        port_comm_free(port_comm);
        return -ENOMEM;
    }
    /* 第一次初始化的时候需要指定一下端口号 */
    port_comm->my_port = port_comm_p2i(port_comm);
    TASK_BIND_PORT_COMM(cur, port_comm);
    spin_unlock_irqrestore(&port_comm->lock, iflags);

    //dbgprint("port bind: bind port=%d task pid=%d success.\n", port, cur->pid);
    return 0;
}

/**
 * 解除对端口port的绑定。
 * 如果验证端口失败，则返回错误。成功则销毁消息池并释放端口
 */
int sys_port_comm_unbind(int port)
{
    if (BAD_PORT_COMM(port)) {
        errprint("port ubind: port %d invalid\n", port);
        return -EINVAL;
    }
    task_t *cur = task_current;
    port_comm_t *port_comm;
    if (port_comm_vertify(port, &port_comm, cur) < 0)
        return -EPERM;

    atomic_dec(&port_comm->reference);
    if (atomic_get(&port_comm->reference) > 0) {    /* 端口还未释放完，就直接返回 */
        /*dbgprint("port unbind: port %d not real unbind, reference=%d\n", 
            port, atomic_get(&port_comm->reference));*/
        TASK_UNBIND_PORT_COMM(cur, port_comm);
        return 0;
    }

    //dbgprint("port unbind: unbind port=%d task pid=%d success.\n", port < 0 ? cur->port_comm->my_port:port, cur->pid);

    /* 引用计数为0，需要真正地解除端口绑定 */
    unsigned long iflags;
    spin_lock_irqsave(&port_comm->lock, iflags);
    if (msgpool_destroy(port_comm->msgpool) < 0) {
        warnprint("port unbind: port %d destroy recv pool failed!\n", port_comm->my_port);
    }
    port_comm->msgpool = NULL;
    port_comm->my_port = -1;
    spin_unlock_irqrestore(&port_comm->lock, iflags);
    TASK_UNBIND_PORT_COMM(cur, port_comm);

    port_comm_free(port_comm);
    return 0;
}

/**
 * 向端口发送一个消息。
 * 如果端口值错误就直接返回错误
 * 首先验证端口，失败就返回错误
 * 如果是发送给自己的就返回错误
 * 生成一个消息id并发送到消息池。
 * 接着就从自己的消息池里面等待消息，如果消息id错误，则返回错误
 */
int sys_port_comm_request(uint32_t port, port_msg_t *msg)
{
    if (BAD_PORT_COMM(port)) {
        errprint("port request: port %d invalid\n", port);
        return -EINVAL;
    }
    task_t *cur = task_current;
    port_comm_t *port_comm, *myport_comm;
    if (port_comm_vertify(-1, &myport_comm, cur) < 0) {
        errprint("port request: port %d vertify failed!\n", port);
        return -EPERM;
    }
    port_comm = port_comm_find(port);
    if (!port_comm) {
        errprint("port request: port %d not bounded.\n", port);
        return -EPERM;
    }
    if (port_comm == myport_comm) {
        errprint("port request: port %d can't request self.\n", port);
        return -EPERM;
    }

    /* 生成一个id，用来做消息验证 */
    uint32_t msgid = port_comm_generate_msg_id();
    msg->header.id = msgid;
    msg->header.port = myport_comm->my_port;
    /* 往端口发出请求 */
    if (msgpool_put(port_comm->msgpool, msg, msg->header.size) < 0) {
        errprint("port request: msg put to %d failed!\n", port);
        return -EPERM;
    }
    /* 尝试获取消息，如果获取无果，就yield来降低cpu占用 */
    int try_count = 0;
    while (msgpool_try_get(myport_comm->msgpool, msg, msgpool_get_callback) < 0){
        // 如果有异常产生，则返回中断错误号
        if (exception_cause_exit(&task_current->exception_manager)) {
            noteprint("port_comm receive: port %d interrupt by exception!\n", myport_comm->my_port);
            return -EINTR;
        }
        try_count++;
        if (try_count > PORT_COMM_RETRY_GET_CNT) {
            task_yield();
            try_count = 0;
        }
    }
    /* 对消息进行验证，看是否存在丢失 */
    if (msg->header.id != msgid) {
        warnprint("port request: port %d msg id %d:%d invalid!\n", 
                myport_comm->my_port, msg->header.id, msgid);
        return -EPERM;
    }
    return 0;
}

/**
 * 从消息池接收一个消息。
 * 首先验证自己的端口和消息池，错误或者无消息池则返回
 * 接下来就是从自己的消息池里面获取一个消息，然后返回。
 */
int sys_port_comm_receive(int port, port_msg_t *msg)
{
    task_t *cur = task_current;
    port_comm_t *port_comm;
    if (port_comm_vertify(port, &port_comm, cur) < 0) {
        errprint("port receive: port=%d task pid=%d vertify failed!\n", port, cur->pid);
        return -EPERM;
    }
    if (!port_comm->msgpool)
        return -EPERM;
    int try_count = 0;
    /* 尝试获取消息，如果获取无果，就yield来降低cpu占用 */
    while (msgpool_try_get(port_comm->msgpool, msg, msgpool_get_callback) < 0){
        // 如果有异常产生，则返回中断错误号
        if (exception_cause_exit(&task_current->exception_manager)) {
            noteprint("port_comm receive: port %d interrupt by exception!\n", port);
            return -EINTR;
        }
        try_count++;
        if (try_count > PORT_COMM_RETRY_GET_CNT) {
            task_yield();
            try_count = 0;
        }
    }
    return 0;
}

/**
 * 应答一个消息。
 * 首先会验证自己的端口，失败则返回错误
 * 接着从消息中获取要应答的端口，如果端口没有找到或者无消息池则返回错误。
 * 最后把消息放入客户端的消息池。
 */
int sys_port_comm_reply(int port, port_msg_t *msg)
{
    if (BAD_PORT_COMM(port)) {
        errprint("port reply: port %d invalid\n", port);
        return -EINVAL;
    }
    task_t *cur = task_current;
    port_comm_t *port_comm;
    if (port_comm_vertify(port, &port_comm, cur) < 0) {
        errprint("port reply: port %d vertify failed!\n", port);
        return -EPERM;
    }
    /* 从消息中找到需要应答的端口，将应答消息传递回去 */
    port_comm_t *client_port = port_comm_i2p(msg->header.port);
    if (!client_port)
        return -EPERM;
    if (!client_port->msgpool)
        return -EPERM;
    return msgpool_put(client_port->msgpool, msg, msg->header.size);
}

void port_comm_thread(void *arg)
{
    infoprint("port_comm start.\n");
    sys_port_comm_bind(0, 0);
    sys_port_comm_bind(0, 0);
    sys_port_comm_unbind(0);
    sys_port_comm_unbind(0);
    sys_port_comm_bind(0, 0);
    port_msg_t smsg;
    while (1)
    {
        if (!sys_port_comm_receive(0, &smsg))
            infoprint("port receive: %d ok\n", smsg.header.id);
        if (!sys_port_comm_reply(0, &smsg))
            infoprint("port reply: %d ok\n", smsg.header.id);
    }
}

void port_comm_threada(void *arg)
{
    infoprint("port_comma start.\n");
    struct timeval tv;
    tv.tv_sec = 1;
    sys_usleep(&tv, NULL);
    sys_port_comm_bind(-1, 0);
    port_msg_t smsg;
    while (1)
    {
        strcpy((char *)smsg.data, "hello!\n");
        if (!sys_port_comm_request(0, &smsg))
            infoprint("A request: %d ok\n", smsg.header.id);
    }
}

void port_comm_threadb(void *arg)
{
    infoprint("port_commb start.\n");
    struct timeval tv;
    tv.tv_sec = 1;
    sys_usleep(&tv, NULL);
    sys_port_comm_bind(-1, 0);
    port_msg_t smsg;
    while (1)
    {
        strcpy((char *)smsg.data, "foo!\n");
        if (!sys_port_comm_request(0, &smsg))
            infoprint("B request: %d ok\n", smsg.header.id);
    }
}

void port_msg_reset(port_msg_t *msg)
{
    memset(msg, 0, sizeof(port_msg_t));    
}

void port_msg_copy_header(port_msg_t *src, port_msg_t *dest)
{
    memcpy(&dest->header, &src->header, sizeof(port_msg_header_t));
}

void port_comm_init()
{
    #if 0
    task_create("port_comm", 0, port_comm_thread, NULL);
    task_create("port_comm", 0, port_comm_threada, NULL);
    task_create("port_comm", 0, port_comm_threadb, NULL);
    task_create("port_comm", 0, port_comm_threadb, NULL);
    #endif
}