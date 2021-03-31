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
            port_comm->flags |= PORT_COMM_USING;
            spinlock_init(&port_comm->lock);
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

port_comm_t *port_comm_alloci(uint32_t port)
{
    unsigned long iflags;
    spin_lock_irqsave(&port_comm_lock, iflags);
    port_comm_t *port_comm = port_comm_i2p(port);
    port_comm->flags |= PORT_COMM_USING;
    spinlock_init(&port_comm->lock);
    spin_unlock_irqrestore(&port_comm_lock, iflags);
    return port_comm;
}

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
        if (port_comm != task->port_comm) {
            errprint("port unbind: port %d not bond on task %d.\n", port, task->pid);
            return -EPERM;                
        }
    } else {
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
 * 绑定时检测端口是否已经占用，并分配一个可用端口
 */
static port_comm_t *__sys_port_comm_bind(int port, task_t *cur)
{
    if (cur->port_comm) {
        errprint("port bind: port %d had bounded on task %d.\n", port_comm_p2i(cur->port_comm), cur->pid);
        return NULL;
    }
    port_comm_t *port_comm;
    if (port >= 0) {
        port_comm = port_comm_find(port);
        if (port_comm) {
            errprint("port bind: port %d had used.\n", port);
            return NULL;
        }
        port_comm = port_comm_alloci(port);
    } else {

        port_comm = port_comm_alloc();
        if (!port_comm) {
            errprint("port bind: no port left.\n");
            return NULL;
        }
    }
    return port_comm;
}

int sys_port_comm_bind(int port)
{
    task_t *cur = task_current;
    port_comm_t *port_comm = __sys_port_comm_bind(port, cur);
    if (!port_comm)
        return -1;
    int msgcnt = port < 0? 1 : PORT_MSG_NR;
    unsigned long iflags;
    spin_lock_irqsave(&port_comm->lock, iflags);
    port_comm->msgpool = msgpool_create(sizeof(port_msg_t), msgcnt);
    if (!port_comm->msgpool) {
        spin_unlock_irqrestore(&port_comm->lock, iflags);
        port_comm_free(port_comm);
        return -ENOMEM;
    }
    port_comm->my_port = port_comm_p2i(port_comm);
    cur->port_comm = port_comm;
    spin_unlock_irqrestore(&port_comm->lock, iflags);
    return 0;
}

int sys_port_comm_unbind(int port)
{
    task_t *cur = task_current;
    port_comm_t *port_comm;
    if (port_comm_vertify(port, &port_comm, cur) < 0)
        return -EPERM;
    unsigned long iflags;
    spin_lock_irqsave(&port_comm->lock, iflags);
    if (msgpool_destroy(port_comm->msgpool) < 0) {
        warnprint("port unbind: port %d destroy recv pool failed!\n", port_comm->my_port);
    }
    port_comm->msgpool = NULL;
    port_comm->my_port = -1;
    spin_unlock_irqrestore(&port_comm->lock, iflags);
    port_comm_free(port_comm);
    cur->port_comm = NULL;
    return 0;
}

int sys_port_comm_request(uint32_t port, port_msg_t *msg)
{
    if (BAD_PORT_COMM(port)) {
        return -EINVAL;
    }
    task_t *cur = task_current;
    port_comm_t *port_comm, *myport_comm;
    if (port_comm_vertify(-1, &myport_comm, cur) < 0)
        return -EPERM;
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

int sys_port_comm_receive(int port, port_msg_t *msg)
{
    task_t *cur = task_current;
    port_comm_t *port_comm;
    if (port_comm_vertify(port, &port_comm, cur) < 0)
        return -EPERM;
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

int sys_port_comm_reply(int port, port_msg_t *msg)
{
    task_t *cur = task_current;
    port_comm_t *port_comm;
    if (port_comm_vertify(port, &port_comm, cur) < 0)
        return -EPERM;
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
    sys_port_comm_bind(0);
    sys_port_comm_bind(0);
    sys_port_comm_unbind(0);
    sys_port_comm_unbind(0);
    sys_port_comm_bind(0);
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
    sys_port_comm_bind(-1);
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
    sys_port_comm_bind(-1);
    port_msg_t smsg;
    while (1)
    {
        strcpy((char *)smsg.data, "foo!\n");
        if (!sys_port_comm_request(0, &smsg))
            infoprint("B request: %d ok\n", smsg.header.id);
    }
}

void port_comm_init()
{
    #if 0
    kern_thread_start("port_comm", 0, port_comm_thread, NULL);
    kern_thread_start("port_comm", 0, port_comm_threada, NULL);
    kern_thread_start("port_comm", 0, port_comm_threadb, NULL);
    kern_thread_start("port_comm", 0, port_comm_threadb, NULL);
    #endif
}