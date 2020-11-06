#include <xbook/task.h>
#include <xbook/timer.h>
#include <xbook/clock.h>
#include <xbook/schedule.h>

// #define DEBUG_TASK_SLEEP

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
    task->sleep_timer = NULL;
    list_del_init(&task->list);
    task_wakeup(task);
}

unsigned long task_sleep_by_ticks(clock_t ticks)
{
    if (!ticks)     /* 为0就不休眠 */
        return 0;
    DEFINE_TIMER(sleep_timer, ticks, (unsigned long )task_current, sleep_task_timeout);
    task_current->sleep_timer = &sleep_timer;    /* 绑定休眠定时器 */
    sleep_timer.timeout += timer_ticks;
    timer_add(&sleep_timer);
#ifdef DEBUG_TASK_SLEEP 
    printk(KERN_DEBUG "task_sleep_by_ticks: start pid=%d\n", task_current->pid);
#endif    
    task_block(TASK_BLOCKED);   /* 阻塞自己 */
    unsigned long flags;
    interrupt_save_and_disable(flags);
    task_current->sleep_timer = NULL;           /* 解绑休眠定时器 */
    long dt = 0;
    /* 有可能还在休眠中就被唤醒了，那么就检查定时器是否已经被执行过了 */
    if (sleep_timer.timeout > timer_ticks) { /* 非正常唤醒 */
#ifdef DEBUG_TASK_SLEEP
        printk(KERN_DEBUG "task_sleep_by_ticks: not del\n");
#endif
        dt = sleep_timer.timeout - timer_ticks;
        timer_del(&sleep_timer);    /* 取消定时器 */
    }
#ifdef DEBUG_TASK_SLEEP
    printk(KERN_DEBUG "task_sleep_by_ticks: end pid=%d timeout=%d\n",
        task_current->pid, dt);
#endif
    interrupt_restore_state(flags);
    
    return dt; /* 返回剩余ticks */
}

unsigned long sys_sleep(unsigned long second)
{
    TASK_CHECK_THREAD_CANCELATION_POTINT(task_current);
    if (!second)
        return 0;
    clock_t ticks = second * HZ;
    second = task_sleep_by_ticks(ticks) / HZ;
    return second;  /* 返回剩余的秒数 */
}