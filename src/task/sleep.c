#include <xbook/task.h>
#include <xbook/timer.h>
#include <xbook/clock.h>

#define DEBUG_TASK_SLEEP

/**
 * sleep_task_timeout - 休眠任务超时
 * @arg: 休眠的任务
 * 
 * 超时后唤醒任务，这是一般的形式，但是还有种
 * 被触发器触发唤醒。
 */
static void sleep_task_timeout(unsigned long arg)
{
#ifdef DEBUG_TASK_SLEEP 
    printk(KERN_DEBUG "sleep_task_timeout: wakeup\n");
#endif    
    task_t *task = (task_t *)arg;
    /* 需要在触发的时候就置空，避免还未唤醒的时候，进程退出，检测定时器
    指针还指向定时器，但是定时器在早已从定时器链表删除。 */
    task->sleep_timer = NULL;
    
    /* 如果已经在其他队列，那么先从其他队列删除
    案例：在waitque中，会先添加到就绪队列，然后休眠。
    当定时器触发时，会把任务唤醒，添加到新的就绪队列。
    而此时，task本来在waitque中，这就破坏了waitque，
    因此，需要先在这里把原有链表删除，再唤醒。
     */
    list_del_init(&task->list); 
    /* 超时后唤醒休眠的任务 */
    task_wakeup(task);
}

unsigned long task_sleep_by_ticks(clock_t ticks)
{
    if (!ticks)     /* 为0就不休眠 */
        return 0;
    DEFINE_TIMER(sleep_timer, ticks, (unsigned long )current_task, sleep_task_timeout);
    current_task->sleep_timer = &sleep_timer;    /* 绑定休眠定时器 */
    timer_add(&sleep_timer);
#ifdef DEBUG_TASK_SLEEP 
    printk(KERN_DEBUG "task_sleep_by_ticks: start pid=%d\n", current_task->pid);
#endif    
    task_block(TASK_BLOCKED);   /* 阻塞自己 */
    unsigned long flags;
    save_intr(flags);
    current_task->sleep_timer = NULL;           /* 解绑休眠定时器 */
    /* 有可能还在休眠中就被唤醒了，那么就检查定时器是否已经被执行过了 */
    if (sleep_timer.timeout > timer_ticks) { /* 非正常唤醒 */
#ifdef DEBUG_TASK_SLEEP
        printk(KERN_DEBUG "task_sleep_by_ticks: not del\n");
#endif
        timer_del(&sleep_timer);    /* 取消定时器 */
    }
#ifdef DEBUG_TASK_SLEEP
    printk(KERN_DEBUG "task_sleep_by_ticks: end pid=%d timeout=%d\n",
        current_task->pid, sleep_timer.timeout - timer_ticks);
#endif
    restore_intr(flags);
    
    return sleep_timer.timeout - timer_ticks; /* 返回剩余ticks */
}

unsigned long sys_sleep(unsigned long second)
{
    CHECK_THREAD_CANCELATION_POTINT(current_task);
    if (!second)
        return 0;
    clock_t ticks = second * HZ;
    second = task_sleep_by_ticks(ticks) / HZ;
    return second;  /* 返回剩余的秒数 */
}