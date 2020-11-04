#include <arch/interrupt.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/process.h>
#include <xbook/pthread.h>
#include <xbook/waitque.h>
#include <xbook/clock.h>
#include <sys/time.h>
#include <xbook/mutexlock.h>
#include <sys/waitque.h>
#include <errno.h>

#define DEBUG_WAITQUE

#define USE_MUTEX_LOCK

/* waitque储存池 */
waitque_t *waitque_table;

#ifdef USE_MUTEX_LOCK
/* 访问储存池需要互斥 */
DEFINE_MUTEX_LOCK(waitque_table_lock);
#else
DEFINE_SPIN_LOCK(waitque_table_lock);
#endif

/**
 * sys_waitque_create - 创建一个等待队列
 * 
 * 返回一个等待队列句柄
 */
int sys_waitque_create()
{
    waitque_t *waitque = &waitque_table[0];
    int i;
    
#ifdef USE_MUTEX_LOCK
    mutex_lock(&waitque_table_lock);
#else

    irqno_t irqflags;
    spin_lock_irqsave(&waitque_table_lock, irqflags);
#endif    
    for (i = 0; i < WAITQUE_NR; i++) {
        if (!waitque->flags) { /* 没有使用 */
            wait_queue_init(&waitque->wait_queue);
            waitque->flags = WAITQUE_USING;
            
            #ifdef USE_MUTEX_LOCK
            mutex_unlock(&waitque_table_lock);
            #else
            spin_unlock_irqrestore(&waitque_table_lock, irqflags);
            #endif
#ifdef DEBUG_WAITQUE
            printk(KERN_DEBUG "%s: pid=%d get handle=%d\n", __func__, current_task->pid, i);
#endif
            return i;
        }
        waitque++;
    }
    #ifdef USE_MUTEX_LOCK
    mutex_unlock(&waitque_table_lock);
    #else
    spin_unlock_irqrestore(&waitque_table_lock, irqflags);
    #endif
#ifdef DEBUG_WAITQUE
    printk(KERN_DEBUG "%s: pid=%d get handle failed!\n", __func__, current_task->pid);
#endif
    return -1;
}

/**
 * sys_waitque_destroy - 销毁一个等待队列
 * @handle: 等待队列句柄
 * 
 * 销毁一个等待队列，移除等待队列上面的任务
 */
int sys_waitque_destroy(int handle)
{
    if (IS_BAD_WAITQUE(handle))
        return -1;
    
    #ifdef USE_MUTEX_LOCK
    mutex_lock(&waitque_table_lock);
    #else
    irqno_t irqflags;
    spin_lock_irqsave(&waitque_table_lock, irqflags);
    #endif   
    waitque_t *waitque = &waitque_table[handle];
    if (waitque->flags) {
        /* 遍历唤醒等待队列上的任务 */
        while (!list_empty(&waitque->wait_queue.wait_list))
        {
            wait_queue_wakeup(&waitque->wait_queue);
        }
        waitque->flags = 0;
    }
    #ifdef USE_MUTEX_LOCK
    mutex_unlock(&waitque_table_lock);
    #else
    spin_unlock_irqrestore(&waitque_table_lock, irqflags);
    #endif
    return 0;
}


/**
 * sys_waitque_wait - 添加到等待队列
 * @handle: 等待队列句柄
 * @addr: 需要操作的数据地址
 * @wqflags: 操作标志：
 *          WAITQUE_SET： 设置addr里面的值为value
 *          WAITQUE_ADD： 往addr里面的值加上value
 *          WAITQUE_SUB： 往addr里面的值减去value
 * @value: 参数变量:
 *          当有WAOTQIE_TIMED标志时，表示时间结构体的地址
 * 
 * 把自己添加到等待队列并阻塞
 * 修改值和阻塞的过程是原子操作
 */
int sys_waitque_wait(int handle, void *addr, unsigned int wqflags, unsigned long value)
{
    if (IS_BAD_WAITQUE(handle))
        return EINVAL;

    CHECK_THREAD_CANCELATION_POTINT(current_task);

    unsigned long flags;
    interrupt_save_state(flags);
    waitque_t *waitque = &waitque_table[handle];
    if (waitque->flags) {
#ifdef DEBUG_WAITQUE
        printk(KERN_DEBUG "%s: pid=%d wait\n", __func__, current_task->pid);
#endif
        /* 对变量进行操作 */
        if (addr) {
            switch (wqflags & WAITQUE_OPMASK)
            {
            case WAITQUE_ADD:
                *(unsigned int *) addr = *(unsigned int *) addr + value;
                break;
            case WAITQUE_SUB:
                *(unsigned int *) addr = *(unsigned int *) addr - value;
                break;
            case WAITQUE_SET:
                *(unsigned int *) addr = value;
                break;
            case WAITQUE_ZERO:
                *(unsigned int *) addr = 0;
                break;
            case WAITQUE_ONE:
                *(unsigned int *) addr = 1;
                break;
            case WAITQUE_INC:
                *(unsigned int *) addr = *(unsigned int *) addr + 1;
                break;
            case WAITQUE_DEC:
                *(unsigned int *) addr = *(unsigned int *) addr - 1;
                break;
            default:
                break;
            }
        }

        wait_queue_add(&waitque->wait_queue, current_task);
        if (wqflags & WAITQUE_TIMED) {   /* 有定时 */
#ifdef DEBUG_WAITQUE
            printk(KERN_DEBUG "%s: pid=%d wait with timer\n", __func__, current_task->pid);
#endif
            /* 如果是中途被打断，剩余ticks就大于0，一般是被其它线程唤醒。
            不然就是正常超时 */
            clock_t ticks = 0;      /* 需要休眠的ticks数 */
            
            /* 计算需要休眠的时间，如果value为NULL，表示直接返回 */
            if (value > 0) {
                struct timespec *abstm = (struct timespec *)value;
                /* 求差 */
                struct timespec curtm;
                sys_clock_gettime(CLOCK_REALTIME, &curtm);
                struct timespec newtm;
                newtm.tv_sec = abstm->tv_sec - curtm.tv_sec;
                newtm.tv_nsec = abstm->tv_nsec - curtm.tv_nsec;

                ticks = timespec_to_systicks(&newtm);
            }
            if (ticks <= 0) {   /* ticks为0，就直接返回 */
                /* 从等待队列删除 */
                wait_queue_remove(&waitque->wait_queue, current_task);
                interrupt_restore_state(flags);
                return ETIMEDOUT;   /* 和超时效果一样 */
            }
            /* 休眠的ticks太小，导致调度过于频繁，因此，在此进行增加,
            越小加的越多，越多，加的越少 */
            /*if (ticks < 10)
                ticks += 10 - ticks;
            */
            if (ticks < 2 * MS_PER_TICKS)
                ticks = 2 * MS_PER_TICKS;

#ifdef DEBUG_WAITQUE
            printk(KERN_DEBUG "%s: pid=%d sleep ticks %u\n", __func__, current_task->pid, ticks);
#endif

            if (task_sleep_by_ticks(ticks) > 0) {
          
#ifdef DEBUG_WAITQUE
                printk(KERN_DEBUG "%s: pid=%d was breaked\n", __func__, current_task->pid);
#endif      
                goto out;
            } else { /* timeout */
#ifdef DEBUG_WAITQUE
                printk(KERN_DEBUG "%s: pid=%d timeout\n", __func__, current_task->pid);
#endif
                task_t *task, *next;
                list_for_each_owner_safe (task, next, &waitque->wait_queue.wait_list, list) {
                    if (task == current_task) {
                        /* 从当前队列删除 */
                        list_del(&task->list);
                        break;
                    }          
                }
                interrupt_restore_state(flags);
                return ETIMEDOUT;
            }
        } else {
            task_block(TASK_BLOCKED);   /* 阻塞 */
        }
    }
out:
    interrupt_restore_state(flags);
    return 0;
}

/**
 * sys_waitque_wake - 从等待队列唤醒一个任务
 * @handle: 等待队列句柄
 * @addr: 需要操作的数据地址
 * @wqflags: 操作标志：
 *          WAITQUE_SET： 设置addr里面的值为value
 *          WAITQUE_ADD： 往addr里面的值加上value
 *          WAITQUE_SUB： 往addr里面的值减去value
 * @value: 参数变量
 * 
 * 唤醒第一个等待中的任务
 * 修改值和阻塞的过程是原子操作
 */
int sys_waitque_wake(int handle, void *addr, unsigned int wqflags, unsigned long value)
{
    if (IS_BAD_WAITQUE(handle))
        return -1;
    
    unsigned long flags;
    interrupt_save_state(flags);
    waitque_t *waitque = &waitque_table[handle];
    if (waitque->flags) {
#ifdef DEBUG_WAITQUE
        printk(KERN_DEBUG "%s: pid=%d wake handle=%d\n", __func__, current_task->pid, handle);
#endif
        /* 对变量进行操作 */
        if (addr) {
            switch (wqflags & WAITQUE_OPMASK)
            {
            case WAITQUE_ADD:
                *(unsigned int *) addr = *(unsigned int *) addr + value;
                break;
            case WAITQUE_SUB:
                *(unsigned int *) addr = *(unsigned int *) addr - value;
                break;
            case WAITQUE_SET:
                *(unsigned int *) addr = value;
                break;
            case WAITQUE_ZERO:
                *(unsigned int *) addr = 0;
                break;
            case WAITQUE_ONE:
                *(unsigned int *) addr = 1;
                break;
            case WAITQUE_INC:
                *(unsigned int *) addr = *(unsigned int *) addr + 1;
                break;
            case WAITQUE_DEC:
                *(unsigned int *) addr = *(unsigned int *) addr - 1;
                break;
            default:
                break;
            }
        }
        
        task_t *task, *next;
        list_for_each_owner_safe (task, next, &waitque->wait_queue.wait_list, list) {
            /* 如果是唤醒一个有定时的线程，本来需要删除其定时器的，但是
            唤醒后，会自动删除定时器，因此，不再这里做对应的处理 */
            if (wqflags & WAITQUE_ALL) {    /* 唤醒全部 */
#ifdef DEBUG_WAITQUE
            printk(KERN_DEBUG "%s: pid=%d wake broadcast thread.\n", __func__, current_task->pid);
#endif
            } else {        /* 只唤醒单线程 */
#ifdef DEBUG_WAITQUE
                printk(KERN_DEBUG "%s: pid=%d wake single thread.\n", __func__, current_task->pid);
#endif
            }
            /* 从当前队列删除 */
            list_del(&task->list);
            /* 唤醒任务 */
            task_wakeup(task);
            if (wqflags & WAITQUE_ALL) {    /* 唤醒全部 */
                continue;
            } else {        /* 只唤醒单线程 */
                break;
            }                       
        }
    }
    interrupt_restore_state(flags);
    return 0;
}

/**
 * init_waitque - 初始化用户态锁
 * 
 */
void init_waitque()
{
    waitque_table = mem_alloc(WAITQUE_NR * sizeof(waitque_t));
    if (waitque_table == NULL)
        panic("%s: alloc memory for waitque table failed!\n", __func__);
    int i;
    for (i = 0; i < WAITQUE_NR; i++) {
        wait_queue_init(&waitque_table[i].wait_queue);
        waitque_table[i].flags = 0;
    }
}
