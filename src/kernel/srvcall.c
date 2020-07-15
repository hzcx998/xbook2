#include <string.h>
#include <xbook/srvcall.h>
#include <xbook/task.h>
#include <xbook/waitqueue.h>
#include <xbook/spinlock.h>

#define DEBUG_LOCAL 0

typedef struct _srvcall {
    int port;
    int state;                  /* 状态 */
    task_t *holder;             /* 服务持有者 */
    task_t *caller;             /* 服务调用者 */
    spinlock_t spin;            /* 自选锁 */
    srvarg_t arg;          /* 参数 */
    wait_queue_t wait_queue;    /* 等待队列 */
} srvcall_t;

/* 服务调用表 */
srvcall_t srvcall_table[SRVCALL_NR]; 

static int copy_srvarg_buffer(srvarg_t *dst, srvarg_t *src, int srvio, int new_buf)
{
    /* 设置返回值 */
    dst->retval = src->retval;

    int i;
    for (i = 0; i < SRVARG_NR; i++) {
        if (src->size[i] > 0) { /* 参数是地址 */
            if (srvio == SRVIO_USER) {      /* 有效：SRVIO_USER */
                if (src->io & (1 << i)) {   /* SRVIO_USER */
                    if (dst->data[i] && src->data[i]) {    
#if DEBUG_LOCAL == 1
                        printk(KERN_DEBUG "%s: data[%d],size=%d dst:%x src:%x\n", __func__, i, src->size[i], dst->data[i], src->data[i]);   
#endif
                        /* 需要将数据复制回到用户缓冲区，缓冲区长度由源指定 */
                        memcpy((void *) dst->data[i], (void *) src->data[i], src->size[i]);
                    }
                }
                if (new_buf) {  /* 需要将内核缓冲区释放 */
#if DEBUG_LOCAL == 1
                    printk(KERN_DEBUG "%s: data%d free buffer: %x size %d\n", __func__, i, src->data[i], src->size[i]);   
#endif
                    kfree((void *) src->data[i]);
                }
            } else if (srvio == SRVIO_SERVICE) {   /* 有效：SRVIO_SERVICE */
                /* 需要将数据复制回到服务缓冲区 */
                if (new_buf) {  /* 需要创建一个新的缓冲区来储存数据 */
                    dst->data[i] = (unsigned long) kmalloc(dst->size[i]);
                    if (dst->data[i] == 0) {
                        return -1;
                    }
#if DEBUG_LOCAL == 1
                    printk(KERN_DEBUG "%s: data%d alloc buffer: %x size %d\n", __func__, i, dst->data[i], dst->size[i]);
#endif
                    memset((void *) dst->data[i], 0, dst->size[i]);
                }
                if (!(src->io & (1 << i))) {   /* SRVIO_SERVICE */
                    if (dst->data[i] && src->data[i]) {   
                        /* 复制完整数据，数据长度由目标指定 */
                        memcpy((void *) dst->data[i], (void *) src->data[i], dst->size[i]);
                    }
                }
            } else if (srvio == SRVIO_BOTH) {
                
            }
        }
    }
    return 0;
}

/**
 * copy_srvarg - 复制参数
 * @dst: 目标
 * @src: 源
 * @srvio: 有效的数据流向方向
 * @new_buf: 需要新的缓冲区 
 */
static int copy_srvarg(srvarg_t *dst, srvarg_t *src, int srvio, int new_buf)
{
    memcpy(dst, src, sizeof(srvarg_t));
    return copy_srvarg_buffer(dst, src, srvio, new_buf);
}

/**
 * sys_srvcall_bind - 服务调用绑定
 * @port: 服务端口
 * 
 * 
 * 成功返回端口号，失败返回-1
 */
int sys_srvcall_bind(int port)
{
    if (IS_BAD_SRVCALL(port))
        return -1;
    /* 必须是绑定一个未绑定的端口 */
    if (srvcall_table[port].holder != NULL)
        return -1;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: task=%d bind port=%d.\n", __func__, current_task->pid, port);
#endif
    srvcall_table[port].holder = current_task;
    return srvcall_table[port].port;
}

/**
 * sys_srvcall_unbind - 服务调用解除绑定
 * @port: 服务端口
 * 
 * 
 * 成功返回0，失败返回-1
 */
int sys_srvcall_unbind(int port)
{
    if (IS_BAD_SRVCALL(port) && port != -1)
        return -1;
    if (port == -1) {   /* 寻找当前进程绑定的端口 */
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "%s: task=%d unbind used port.\n", __func__, current_task->pid);
#endif
        int i;
        for (i = 0; i < SRVCALL_NR; i++) {
            if (srvcall_table[port].holder == current_task) {
                srvcall_table[port].holder = NULL;
                return 0;
            }
        }
        return -1;
    } else {
        /* 不是当前任务解绑就返回 */
        if (srvcall_table[port].holder != current_task)
            return -1;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "%s: task=%d unbind port=%d.\n", __func__, current_task->pid, port);
#endif
        srvcall_table[port].holder = NULL;
        return 0;
    }
}


/**
 * sys_srvcall_listen - 服务调用监听
 * 
 */ 
int sys_srvcall_listen(int port, srvarg_t *arg)
{
    if (IS_BAD_SRVCALL(port))
        return -1;
    srvcall_t *call = srvcall_table + port;
    /* 必须是持有者监听 */
    if (call->holder != current_task)
        return -1;

    spin_lock(&call->spin);

#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: task=%d listen port=%d, wait.\n", __func__, current_task->pid, port);
#endif
    call->state = SRVCALL_LISTEN;   /* 处于监听状态 */
    spin_unlock(&call->spin);
    /* 等待被唤醒 */
    task_block(TASK_BLOCKED);
    spin_lock(&call->spin);
    /* 把参数写入服务进程 */
    COPY_SRVARG(arg, &call->arg);

    spin_unlock(&call->spin);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: task=%d listen port=%d, args: %x %x\n", 
    __func__, current_task->pid, port, arg->data[0], arg->data[1]);
#endif
    return 0;   /* 监听成功 */
}

/**
 * sys_srvcall_fetch - 获取参数缓冲区值
 * @port: 获取的端口
 * @arg: 参数
 * 
 * 成功返回0，失败返回-1
 */
int sys_srvcall_fetch(int port, srvarg_t *arg)
{
    if (IS_BAD_SRVCALL(port))
        return -1;
    srvcall_t *call = srvcall_table + port;
    /* 必须是持有者监听 */
    if (call->holder != current_task)
        return -1;
    
    spin_lock(&call->spin);

    if (copy_srvarg_buffer(arg, &call->arg, SRVIO_SERVICE, 0)) {
        spin_unlock(&call->spin);
        return -1;
    }
    /* 检测参数，并复制数据 */
    spin_unlock(&call->spin);

    return 0;
}


/**
 * sys_srvcall_ack - 服务调用应答
 * 
 */
int sys_srvcall_ack(int port, srvarg_t *arg)
{
    if (IS_BAD_SRVCALL(port))
        return -1;
    srvcall_t *call = srvcall_table + port;
    /* 必须是持有者监听 */
    if (call->holder != current_task)
        return -1;
    spin_lock(&call->spin);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: task=%d ack port=%d.\n", __func__, current_task->pid, port);
#endif
    /* 处于应答状态 */
    call->state = SRVCALL_ACK;
    /* 把参数写回服务参数 */
    if (copy_srvarg_buffer(&call->arg, arg, SRVIO_USER, 0)) {
        spin_unlock(&call->spin);
        return -1;
    }
    
    /* 从进程中复制到内核 */
    if (call->caller) {
        if (call->caller->state == TASK_BLOCKED) {  
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: task=%d ack port=%d. unblock task=%d\n", __func__, 
        current_task->pid, port, call->caller->pid);
#endif
            task_unblock(call->caller);
        }
        call->caller = NULL;
    }
    
    if (call->state == SRVCALL_ACK) {
        spin_unlock(&call->spin);
        /* 阻塞自己，等待服务完成 */
        task_block(TASK_BLOCKED);
    } else {
        spin_unlock(&call->spin);
    }
    
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "%s: task=%d ack port=%d. done\n", __func__, 
        current_task->pid, port);
#endif
    return 0;
}

/**
 * sys_srvcall - 服务调用
 * port: 请求的端口
 * 
 */
int sys_srvcall(int port, srvarg_t *arg)
{
    if (IS_BAD_SRVCALL(port))
        return -1;
    srvcall_t *call = srvcall_table + port;
    /* 没有绑定就返回 */
    if (call->holder == NULL)
        return -1;
    
    while (1)
    {
        spin_lock(&call->spin);

        if (call->state == SRVCALL_LISTEN) {   /* 服务正在监听 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "%s: task=%d call port=%d. in listen...\n", __func__, 
                current_task->pid, port);
#endif
            /* 把参数复制到内核 */
            if (copy_srvarg(&call->arg, arg, SRVIO_SERVICE, 1)) {
                spin_unlock(&call->spin);
                return -1;
            }

            call->state = SRVCALL_PENDING;
            call->caller = current_task;
            /* 唤醒服务 */
            task_unblock(call->holder);

            if (call->state != SRVCALL_ACK) {    /* 只要还没应答，就可以阻塞 */  
#if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "%s: task=%d call port=%d. block.\n", __func__, 
                    current_task->pid, port);
#endif
                spin_unlock(&call->spin);
                /* 阻塞自己 */
                task_block(TASK_BLOCKED);
                spin_lock(&call->spin);
            }
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "%s: task=%d call port=%d. copy to user.\n", __func__, 
                current_task->pid, port);
#endif
            /* 被唤醒后，复制参数到进程空间 */
            if (copy_srvarg_buffer(arg, &call->arg, SRVIO_USER, 1)) {
                spin_unlock(&call->spin);
                return -1;
            }

            /* 复制缓冲区 */
            call->state = SRVCALL_FINISH;    /* 调用完成 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "%s: task=%d call port=%d. finished.\n", __func__, 
                current_task->pid, port);
#endif
            spin_unlock(&call->spin);
            /* 完成后需要唤醒服务进程 */
            if (call->holder->state == TASK_BLOCKED) {  
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "%s: task=%d call port=%d. unblock holder=%d\n", __func__, 
                current_task->pid, port, call->holder->pid);
#endif
                task_unblock(call->holder);
            }
            
            break;
        }
#if DEBUG_LOCAL == 1
        /*printk(KERN_DEBUG "%s: task=%d call port=%d. not listen\n", __func__, 
            current_task->pid, port);*/
#endif
        spin_unlock(&call->spin);
    }
    return 0;
}

/**
 * init_srvcall - 初始化服务调用
 * 
 */
void init_srvcall()
{
    int i;
    for (i = 0; i < SRVCALL_NR; i++) {
        srvcall_table[i].port = i;
        srvcall_table[i].state = SRVCALL_FINISH;
        srvcall_table[i].holder = NULL;
        srvcall_table[i].caller = NULL;
        wait_queue_init(&srvcall_table[i].wait_queue);
        spinlock_init(&srvcall_table[i].spin);
    }
}