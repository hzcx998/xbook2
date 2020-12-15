#include <xbook/servcall.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

static servport_t servport_table[SERVPORT_NR];
DEFINE_SPIN_LOCK_UNLOCKED(servport_lock);
static uint32_t servcall_msg_next_id = 0;

uint32_t servcall_generate_msg_id()
{
    return servcall_msg_next_id++;
}

servport_t *servport_alloc()
{
    unsigned long iflags;
    spin_lock_irqsave(&servport_lock, iflags);
    int i; for (i = 0; i < SERVPORT_NR; i++) {
        servport_t *servport = &servport_table[i];
        if (!servport->flags) {
            servport->flags |= SERVPORT_USING;
            spinlock_init(&servport->lock);
            spin_unlock_irqrestore(&servport_lock, iflags);
            return servport;
        }
    }
    spin_unlock_irqrestore(&servport_lock, iflags);
    return NULL;
}

int servport_free(servport_t *servport)
{
    unsigned long iflags;
    spin_lock_irqsave(&servport_lock, iflags);
    int i; for (i = 0; i < SERVPORT_NR; i++) {
        servport_t *port = &servport_table[i];
        if (port == servport && port->flags) {
            port->flags = 0;
            spin_unlock_irqrestore(&servport_lock, iflags);
            return i;
        }
    }
    spin_unlock_irqrestore(&servport_lock, iflags);
    return -1;
}

servport_t *servport_find(uint32_t port)
{
    unsigned long iflags;
    spin_lock_irqsave(&servport_lock, iflags);
    int i; for (i = 0; i < SERVPORT_NR; i++) {
        servport_t *servport = &servport_table[i];
        if (servport->flags && i == port) {
            spin_unlock_irqrestore(&servport_lock, iflags);
            return servport;
        }
    }
    spin_unlock_irqrestore(&servport_lock, iflags);
    return NULL;
}

uint32_t servport_p2i(servport_t *servport)
{
    uint32_t index = servport - servport_table; 
    ASSERT(index < SERVPORT_NR);
    return index;
}

servport_t *servport_i2p(uint32_t port)
{
    ASSERT(port < SERVPORT_NR);
    servport_t *servport = servport_table + port; 
    return servport;
}

servport_t *servport_alloci(uint32_t port)
{
    unsigned long iflags;
    spin_lock_irqsave(&servport_lock, iflags);
    servport_t *servport = servport_i2p(port);
    servport->flags |= SERVPORT_USING;
    spinlock_init(&servport->lock);
    spin_unlock_irqrestore(&servport_lock, iflags);
    return servport;
}

int servport_vertify(int port, servport_t **out_servport, task_t *task)
{
    if (!task->servport) {
        //errprint("serv unbind: not port bound on task %d.\n", task->pid);
        return -EPERM;
    }
    servport_t *servport;
    if (port >= 0) {
        servport = servport_find(port);
        if (!servport) {
            errprint("serv unbind: port %d not bounded.\n", port);
            return -EPERM;
        }
        if (servport != task->servport) {
            errprint("serv unbind: port %d not bond on task %d.\n", port, task->pid);
            return -EPERM;                
        }
    } else {
        if (!(task->servport && task->servport->flags)) {
            errprint("serv unbind: not port bond on task %d.\n", task->pid);
            return -EPERM;
        }
        servport = task->servport;
    }
    *out_servport = servport;
    return 0;
}

/**
 * 绑定时检测端口是否已经占用，并分配一个可用端口
 */
static servport_t *__sys_servport_bind(int port, task_t *cur)
{
    if (cur->servport) {
        errprint("serv bind: port %d had bounded on task %d.\n", servport_p2i(cur->servport), cur->pid);
        return NULL;
    }
    servport_t *servport;
    if (port >= 0) {
        servport = servport_find(port);
        if (servport) {
            errprint("serv bind: port %d had used.\n", port);
            return NULL;
        }
        servport = servport_alloci(port);
    } else {
        servport = servport_alloc();
        if (!servport) {
            errprint("serv bind: no port left.\n");
            return NULL;
        }
    }
    return servport;
}

int sys_servport_bind(int port)
{
    task_t *cur = task_current;
    servport_t *servport = __sys_servport_bind(port, cur);
    if (!servport)
        return -1;
    int msgcnt = port < 0? 1 : SERVMSG_NR;
    unsigned long iflags;
    spin_lock_irqsave(&servport->lock, iflags);
    servport->msgpool = msgpool_create(sizeof(servmsg_t), msgcnt);
    if (!servport->msgpool) {
        spin_unlock_irqrestore(&servport->lock, iflags);
        servport_free(servport);
        return -ENOMEM;
    }
    servport->my_port = servport_p2i(servport);
    cur->servport = servport;
    spin_unlock_irqrestore(&servport->lock, iflags);
    return 0;
}

int sys_servport_unbind(int port)
{
    task_t *cur = task_current;
    servport_t *servport;
    if (servport_vertify(port, &servport, cur) < 0)
        return -EPERM;
    unsigned long iflags;
    spin_lock_irqsave(&servport->lock, iflags);
    if (msgpool_destroy(servport->msgpool) < 0) {
        warnprint("serv unbind: port %d destroy recv pool failed!\n", servport->my_port);
    }
    servport->msgpool = NULL;
    servport->my_port = -1;
    spin_unlock_irqrestore(&servport->lock, iflags);
    servport_free(servport);
    cur->servport = NULL;
    return 0;
}

int sys_servport_request(uint32_t port, servmsg_t *msg)
{
    if (BAD_SERVPORT(port)) {
        return -EINVAL;
    }
    task_t *cur = task_current;
    servport_t *servport, *myservport;
    if (servport_vertify(-1, &myservport, cur) < 0)
        return -EPERM;
    servport = servport_find(port);
    if (!servport) {
        errprint("serv request: port %d not bounded.\n", port);
        return -EPERM;
    }
    if (servport == myservport) {
        errprint("serv request: port %d can't request self.\n", port);
        return -EPERM;
    }

    /* 生成一个id，用来做消息验证 */
    uint32_t msgid = servcall_generate_msg_id();
    msg->id = msgid;
    msg->port = myservport->my_port;
    /* 往端口发出请求 */
    if (msgpool_put(servport->msgpool, msg) < 0) {
        errprint("serv request: msg put to %d failed!\n", port);
        return -EPERM;
    }
    /* 尝试获取消息，如果获取无果，就yeild来降低cpu占用 */
    int try_count = 0;
    while (msgpool_try_get(myservport->msgpool, msg) < 0){
        // 如果有异常产生，则返回中断错误号
        if (exception_cause_exit(&task_current->exception_manager)) {
            noteprint("servport receive: port %d interrupt by exception!\n", myservport->my_port);
            return -EINTR;
        }
        try_count++;
        if (try_count > SERVPORT_RETRY_GET_CNT) {
            task_yeild();
            try_count = 0;
        }
    }
    /* 对消息进行验证，看是否存在丢失 */
    if (msg->id != msgid) {
        warnprint("serv request: port %d msg id %d:%d invalid!\n", 
                myservport->my_port, msg->id, msgid);
        return -EPERM;
    }
    return 0;
}

int sys_servport_receive(int port, servmsg_t *msg)
{
    task_t *cur = task_current;
    servport_t *servport;
    if (servport_vertify(port, &servport, cur) < 0)
        return -EPERM;
    if (!servport->msgpool)
        return -EPERM;
    int try_count = 0;
    /* 尝试获取消息，如果获取无果，就yeild来降低cpu占用 */
    while (msgpool_try_get(servport->msgpool, msg) < 0){
        // 如果有异常产生，则返回中断错误号
        if (exception_cause_exit(&task_current->exception_manager)) {
            noteprint("servport receive: port %d interrupt by exception!\n", port);
            return -EINTR;
        }
        try_count++;
        if (try_count > SERVPORT_RETRY_GET_CNT) {
            task_yeild();
            try_count = 0;
        }
    }
    return 0;
}

int sys_servport_reply(int port, servmsg_t *msg)
{
    task_t *cur = task_current;
    servport_t *servport;
    if (servport_vertify(port, &servport, cur) < 0)
        return -EPERM;
    /* 从消息中找到需要应答的端口，将应答消息传递回去 */
    servport_t *client_port = servport_i2p(msg->port);
    if (!client_port)
        return -EPERM;
    if (!client_port->msgpool)
        return -EPERM;
    return msgpool_put(client_port->msgpool, msg);
}

void servcall_thread(void *arg)
{
    infoprint("servcall start.\n");
    sys_servport_bind(0);
    sys_servport_bind(0);
    sys_servport_unbind(0);
    sys_servport_unbind(0);
    sys_servport_bind(0);
    servmsg_t smsg;
    while (1)
    {
        if (!sys_servport_receive(0, &smsg))
            infoprint("serv receive: %d ok\n", smsg.id);
        if (!sys_servport_reply(0, &smsg))
            infoprint("serv reply: %d ok\n", smsg.id);
    }
}

void servcall_threada(void *arg)
{
    infoprint("servcalla start.\n");
    struct timeval tv;
    tv.tv_sec = 1;
    sys_usleep(&tv, NULL);
    sys_servport_bind(-1);
    servmsg_t smsg;
    while (1)
    {
        strcpy(smsg.data, "hello!\n");
        if (!sys_servport_request(0, &smsg))
            infoprint("A request: %d ok\n", smsg.id);
    }
}

void servcall_threadb(void *arg)
{
    infoprint("servcallb start.\n");
    struct timeval tv;
    tv.tv_sec = 1;
    sys_usleep(&tv, NULL);
    sys_servport_bind(-1);
    servmsg_t smsg;
    while (1)
    {
        strcpy(smsg.data, "foo!\n");
        if (!sys_servport_request(0, &smsg))
            infoprint("B request: %d ok\n", smsg.id);
    }
}

void servcall_init()
{
    #if 0
    kern_thread_start("servcall", 0, servcall_thread, NULL);
    kern_thread_start("servcall", 0, servcall_threada, NULL);
    kern_thread_start("servcall", 0, servcall_threadb, NULL);
    kern_thread_start("servcall", 0, servcall_threadb, NULL);
    #endif
}