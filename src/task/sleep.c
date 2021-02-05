#include <xbook/task.h>
#include <xbook/timer.h>
#include <xbook/clock.h>
#include <xbook/schedule.h>

static void task_sleep_timeout_handler(timer_t *timer_self, void *arg)
{
    task_t *task = (task_t *)arg;
    //list_del_init(&task->list);
    task_wakeup(task);
}

unsigned long task_sleep_by_ticks(clock_t ticks)
{
    if (!ticks)
        return 0;
    task_t *cur = task_current;
    timer_t *timer = &cur->sleep_timer;
    timer_modify(timer, ticks);
    timer_set_arg(timer, cur);
    timer_set_handler(timer, task_sleep_timeout_handler);
    timer_add(timer);
    task_block(TASK_BLOCKED);
    unsigned long flags;
    interrupt_save_and_disable(flags);
    long delta_ticks = 0;
    if (timer->timeout > timer_ticks) {
        delta_ticks = timer->timeout - timer_ticks;
        timer_del(timer);
        //keprint("sleep intrrupted!\n");
    }
    interrupt_restore_state(flags);    
    return delta_ticks;
}

unsigned long sys_sleep(unsigned long second)
{
    TASK_CHECK_THREAD_CANCELATION_POTINT(task_current);
    if (!second)
        return 0;
    clock_t ticks = second * HZ;
    second = task_sleep_by_ticks(ticks) / HZ;
    return second;
}