#include <xbook/servcall.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

static servport_t servport_table[SERVPORT_NR];
DEFINE_SPIN_LOCK_UNLOCKED(servport_lock);

servport_t *servport_alloc()
{
    unsigned long iflags;
    spin_lock_irqsave(&servport_lock, iflags);
    int i; for (i = 0; i < SERVPORT_NR; i++) {
        servport_t *servport = &servport_table[i];
        if (!servport->flags) {
            servport->flags |= SERVPORT_USING;
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
    spin_unlock_irqrestore(&servport_lock, iflags);
    return servport;
}

int servport_vertify(int port, servport_t **out_servport, task_t *task)
{
    if (!task->servport) {
        errprint("serv unbind: not port bound on task %d.\n", task->pid);
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

int servport_bind(int port)
{
    servport_t *servport;
    task_t *cur = task_current;
    if (cur->servport) {
        errprint("serv bind: port %d had bounded on task %d.\n", servport_p2i(cur->servport), cur->pid);
        return -EPERM;
    }
    if (port >= 0) {
        servport = servport_find(port);
        if (servport) {
            errprint("serv bind: port %d had used.\n", port);
            return -EPERM;
        }
        servport = servport_alloci(port);
    } else {
        servport = servport_alloc();
        if (!servport) {
            errprint("serv bind: no port left.\n");
            return -EPERM;
        }
    }
    servport->recv_pool = msgpool_create(sizeof(servmsg_t), 4);
    if (!servport->recv_pool) {
        servport_free(servport);
        return -ENOMEM;
    }
    servport->send_pool = msgpool_create(sizeof(servmsg_t), 4);
    if (!servport->send_pool) {
        msgpool_destroy(servport->recv_pool);
        servport->recv_pool = NULL;
        servport_free(servport);
        return -ENOMEM;
    }
    cur->servport = servport;
    return 0;
}

int servport_unbind(int port)
{
    task_t *cur = task_current;
    servport_t *servport;
    if (servport_vertify(port, &servport, cur) < 0)
        return -EPERM;
    
    if (msgpool_destroy(servport->recv_pool) < 0) {
        warnprint("serv unbind: port %d destroy recv pool failed!\n", port);
    }
    servport->recv_pool = NULL;
    if (msgpool_destroy(servport->send_pool) < 0) {
        warnprint("serv unbind: port %d destroy send pool failed!\n", port);
    }
    servport->send_pool = NULL;
    servport_free(servport);
    cur->servport = NULL;
    return 0;
}

int servport_request(uint32_t port, servmsg_t *msg)
{
    if (BAD_SERVPORT(port)) {
        return -EINVAL;
    }
    task_t *cur = task_current;
    servport_t *servport;
    servport = servport_find(port);
    if (!servport) {
        errprint("serv request: port %d not bounded.\n", port);
        return -EPERM;
    }
    if (msgpool_put(servport->recv_pool, msg) < 0) {
        errprint("serv request: msg put to %d failed!\n", port);
        return -EPERM;
    }

    /*  */

    if (msgpool_get(servport->send_pool, msg) < 0) {
        errprint("serv request: msg get from %d failed!\n", port);
        return -EPERM;
    }
    return 0;
}

int servport_receive(int port, servmsg_t *msg)
{
    task_t *cur = task_current;
    servport_t *servport;
    if (servport_vertify(port, &servport, cur) < 0)
        return -EPERM;
    if (!servport->recv_pool)
        return -EPERM;
    return msgpool_get(servport->recv_pool, msg);
}

int servport_reply(int port, servmsg_t *msg)
{
    task_t *cur = task_current;
    servport_t *servport;
    if (servport_vertify(port, &servport, cur) < 0)
        return -EPERM;
    if (!servport->send_pool)
        return -EPERM;
    return msgpool_put(servport->send_pool, msg);
}

void servcall_thread(void *arg)
{
    infoprint("servcall start.\n");
    servport_bind(0);
    servport_bind(0);
    servport_unbind(0);
    servport_unbind(0);
    servport_bind(0);
    servmsg_t smsg;
    while (1)
    {
        servport_receive(0, &smsg);
        infoprint("serv recv: %s\n", smsg.data);
        strcpy(smsg.data, "world!\n");
        servport_reply(0, &smsg);
    }
    
}

void servcall_threada(void *arg)
{
    infoprint("servcalla start.\n");
    servport_bind(0);
    servmsg_t smsg;
    while (1)
    {
        strcpy(smsg.data, "hello!\n");
        servport_request(0, &smsg);
        infoprint("serv reply: %s\n", smsg.data);
    }
    
}


void servcall_threadb(void *arg)
{
    infoprint("servcallb start.\n");
    servport_bind(0);
    servmsg_t smsg;
    while (1)
    {
        strcpy(smsg.data, "foo!\n");
        servport_request(0, &smsg);
        infoprint("serv reply: %s\n", smsg.data);
    }
    
}

void servcall_init()
{
    kern_thread_start("servcall", 0, servcall_thread, NULL);
    kern_thread_start("servcall", 0, servcall_threada, NULL);
    kern_thread_start("servcall", 0, servcall_threadb, NULL);
    
}