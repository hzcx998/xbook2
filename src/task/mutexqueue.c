#include <arch/interrupt.h>
#include <xbook/schedule.h>
#include <xbook/debug.h>
#include <xbook/task.h>
#include <xbook/process.h>
#include <xbook/pthread.h>
#include <xbook/mutexqueue.h>
#include <xbook/clock.h>
#include <sys/time.h>
#include <xbook/mutexlock.h>
#include <sys/mutexqueue.h>
#include <errno.h>

mutex_queue_t *mutex_queue_head;
DEFINE_MUTEX_LOCK(mutex_queue_lock);

int sys_mutex_queue_alloc(int handle)
{
    if (MUTEX_QUEUE_IS_BAD(handle))
        return -1;
    
    mutex_lock(&mutex_queue_lock);
    mutex_queue_t *mutex_queue = &mutex_queue_head[handle];
    if (mutex_queue->flags) {
        while (!list_empty(&mutex_queue->wait_queue.wait_list)) {
            wait_queue_wakeup(&mutex_queue->wait_queue);
        }
        mutex_queue->flags = 0;
    }
    mutex_unlock(&mutex_queue_lock);
    return 0;
}

int sys_mutex_queue_free()
{
    mutex_queue_t *mutex_queue = &mutex_queue_head[0];
    int i;
    mutex_lock(&mutex_queue_lock);
    
    for (i = 0; i < MUTEX_QUEUE_NR_MAX; i++) {
        if (!mutex_queue->flags) {
            wait_queue_init(&mutex_queue->wait_queue);
            mutex_queue->flags = MUTEX_QUEUE_USING;
            mutex_unlock(&mutex_queue_lock);
            return i;
        }
        mutex_queue++;
    }
    mutex_unlock(&mutex_queue_lock);
    return -1;
}

/**
 * @handle: 等待队列句柄
 * @addr: 需要操作的数据地址
 * @wqflags: 操作标志：
 *          MUTEX_QUEUE_SET： 设置addr里面的值为value
 *          MUTEX_QUEUE_ADD： 往addr里面的值加上value
 *          MUTEX_QUEUE_SUB： 往addr里面的值减去value
 * @value: 参数变量:
 *          当有WAOTQIE_TIMED标志时，表示时间结构体的地址
 * 
 * 把自己添加到等待队列并阻塞
 * 修改值和阻塞的过程是原子操作
 */
int sys_mutex_queue_wait(int handle, void *addr, unsigned int wqflags, unsigned long value)
{
    if (MUTEX_QUEUE_IS_BAD(handle))
        return EINVAL;

    TASK_CHECK_THREAD_CANCELATION_POTINT(task_current);

    unsigned long flags;
    interrupt_save_and_disable(flags);
    mutex_queue_t *mutex_queue = &mutex_queue_head[handle];
    if (mutex_queue->flags) {
        if (addr) {
            switch (wqflags & MUTEX_QUEUE_OPMASK) {
            case MUTEX_QUEUE_ADD:
                *(unsigned int *) addr = *(unsigned int *) addr + value;
                break;
            case MUTEX_QUEUE_SUB:
                *(unsigned int *) addr = *(unsigned int *) addr - value;
                break;
            case MUTEX_QUEUE_SET:
                *(unsigned int *) addr = value;
                break;
            case MUTEX_QUEUE_ZERO:
                *(unsigned int *) addr = 0;
                break;
            case MUTEX_QUEUE_ONE:
                *(unsigned int *) addr = 1;
                break;
            case MUTEX_QUEUE_INC:
                *(unsigned int *) addr = *(unsigned int *) addr + 1;
                break;
            case MUTEX_QUEUE_DEC:
                *(unsigned int *) addr = *(unsigned int *) addr - 1;
                break;
            default:
                break;
            }
        }
        wait_queue_add(&mutex_queue->wait_queue, task_current);
        if (wqflags & MUTEX_QUEUE_TIMED) {
            clock_t ticks = 0;
            if (value > 0) {
                struct timespec *abstm = (struct timespec *)value;
                struct timespec curtm;
                sys_clock_gettime(CLOCK_REALTIME, &curtm);
                struct timespec newtm;
                newtm.tv_sec = abstm->tv_sec - curtm.tv_sec;
                newtm.tv_nsec = abstm->tv_nsec - curtm.tv_nsec;
                ticks = timespec_to_systicks(&newtm);
            }
            if (ticks <= 0) {
                wait_queue_remove(&mutex_queue->wait_queue, task_current);
                interrupt_restore_state(flags);
                return ETIMEDOUT;
            }
            if (ticks < 2 * MS_PER_TICKS)
                ticks = 2 * MS_PER_TICKS;
            if (task_sleep_by_ticks(ticks) > 0) {
                goto out;
            } else {
                task_t *task, *next;
                list_for_each_owner_safe (task, next, &mutex_queue->wait_queue.wait_list, list) {
                    if (task == task_current) {
                        list_del(&task->list);
                        break;
                    }          
                }
                interrupt_restore_state(flags);
                return ETIMEDOUT;
            }
        } else {
            TASK_ENTER_WAITLIST(task_current);
            task_block(TASK_BLOCKED);
        }
    }
out:
    interrupt_restore_state(flags);
    return 0;
}

/**
 * @handle: 等待队列句柄
 * @addr: 需要操作的数据地址
 * @wqflags: 操作标志：
 *          MUTEX_QUEUE_SET： 设置addr里面的值为value
 *          MUTEX_QUEUE_ADD： 往addr里面的值加上value
 *          MUTEX_QUEUE_SUB： 往addr里面的值减去value
 * @value: 参数变量
 * 
 * 唤醒第一个等待中的任务
 * 修改值和阻塞的过程是原子操作
 */
int sys_mutex_queue_wake(int handle, void *addr, unsigned int wqflags, unsigned long value)
{
    if (MUTEX_QUEUE_IS_BAD(handle))
        return -1;
    
    unsigned long flags;
    interrupt_save_and_disable(flags);
    mutex_queue_t *mutex_queue = &mutex_queue_head[handle];
    if (mutex_queue->flags) {
        if (addr) {
            switch (wqflags & MUTEX_QUEUE_OPMASK)
            {
            case MUTEX_QUEUE_ADD:
                *(unsigned int *) addr = *(unsigned int *) addr + value;
                break;
            case MUTEX_QUEUE_SUB:
                *(unsigned int *) addr = *(unsigned int *) addr - value;
                break;
            case MUTEX_QUEUE_SET:
                *(unsigned int *) addr = value;
                break;
            case MUTEX_QUEUE_ZERO:
                *(unsigned int *) addr = 0;
                break;
            case MUTEX_QUEUE_ONE:
                *(unsigned int *) addr = 1;
                break;
            case MUTEX_QUEUE_INC:
                *(unsigned int *) addr = *(unsigned int *) addr + 1;
                break;
            case MUTEX_QUEUE_DEC:
                *(unsigned int *) addr = *(unsigned int *) addr - 1;
                break;
            default:
                break;
            }
        }
        task_t *task, *next;
        list_for_each_owner_safe (task, next, &mutex_queue->wait_queue.wait_list, list) {
            list_del(&task->list);
            TASK_LEAVE_WAITLIST(task);
            task_wakeup(task);
            if (wqflags & MUTEX_QUEUE_ALL) {
                continue;
            } else {
                break;
            }                       
        }
    }
    interrupt_restore_state(flags);
    return 0;
}

void mutex_queue_init()
{
    mutex_queue_head = mem_alloc(MUTEX_QUEUE_NR_MAX * sizeof(mutex_queue_t));
    if (mutex_queue_head == NULL)
        panic("%s: alloc memory for mutex_queue table failed!\n", __func__);
    int i;
    for (i = 0; i < MUTEX_QUEUE_NR_MAX; i++) {
        wait_queue_init(&mutex_queue_head[i].wait_queue);
        mutex_queue_head[i].flags = 0;
    }
}
