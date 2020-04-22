#include <xbook/task.h>
#include <xbook/timer.h>
#include <xbook/clock.h>

#define DEBUG_LOCAL 0

/**
 * sleep_task_timeout - 休眠任务超时
 * @arg: 休眠的任务
 * 
 * 超时后唤醒任务，这是一般的形式，但是还有种
 * 被触发器触发唤醒。
 */
static void sleep_task_timeout(unsigned long arg)
{
#if DEBUG_LOCAL == 1 
    printk(KERN_DEBUG "sleep_task_timeout: wakeup\n");
#endif    
    /* 超时后唤醒休眠的任务 */
    task_wakeup((task_t *)arg);
}

unsigned long task_sleep_by_ticks(clock_t ticks)
{
    DEFINE_TIMER(sleep_timer, ticks, (unsigned long )current_task, sleep_task_timeout);
    current_task->sleep_timer = &sleep_timer;    /* 绑定休眠定时器 */
    timer_add(&sleep_timer);
#if DEBUG_LOCAL == 1 
    printk(KERN_DEBUG "task_sleep_by_ticks: start pid=%d\n", current_task->pid);
#endif    
    task_block(TASK_BLOCKED);   /* 阻塞自己 */
    unsigned long flags;
    save_intr(flags);
    current_task->sleep_timer = NULL;           /* 解绑休眠定时器 */
    /* 有可能还在休眠中就被唤醒了，那么就检查定时器是否已经被执行过了 */
    if (sleep_timer.timeout > 0) { /* 非正常唤醒 */
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "task_sleep_by_ticks: not del\n");
#endif
        timer_del(&sleep_timer);    /* 取消定时器 */
    }
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "task_sleep_by_ticks: end pid=%d timeout=%d\n",
        current_task->pid, sleep_timer.timeout);
#endif
    restore_intr(flags);
    
    return sleep_timer.timeout; /* 返回剩余ticks */
}

unsigned long sys_sleep(unsigned long second)
{
    if (!second)
        return 0;
    clock_t ticks = second * HZ;
    second = task_sleep_by_ticks(ticks) / HZ;
    return second;  /* 返回剩余的秒数 */
}