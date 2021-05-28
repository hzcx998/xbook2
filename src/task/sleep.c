#include <xbook/task.h>
#include <xbook/timer.h>
#include <xbook/clock.h>
#include <xbook/schedule.h>
#include <xbook/safety.h>
#include <sys/time.h>
#include <errno.h>

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

long sys_usleep(struct timeval *inv, struct timeval *outv)
{
    if (!inv)
        return -EINVAL;
    struct timeval tv;
    unsigned long ticks;
    if (mem_copy_from_user(&tv, inv, sizeof(struct timeval)) < 0)
        return -EFAULT;
    if (tv.tv_usec >= 1000000 || tv.tv_sec < 0 || tv.tv_usec < 0)
        return -EINVAL;
    /* 如果小于2毫秒就用延时的方式 */
    if (tv.tv_usec < 2000L && tv.tv_sec == 0) {
        udelay(tv.tv_usec);
        return 0;
    }
    ticks = timeval_to_systicks(&tv);
    ticks = task_sleep_by_ticks(ticks);
    if (ticks > 0) {
        if (outv) {
            systicks_to_timeval(ticks, &tv);
            if (mem_copy_to_user(outv, &tv, sizeof(struct timeval)) < 0)
                return -EFAULT;
        }
        return -EINTR;
    }
    return 0;
}

int sys_nanosleep(struct timespec *req, struct timespec *rem)
{
    if (!req)
        return -EINVAL;
    struct timespec ts;
    unsigned long ticks;
    if (mem_copy_from_user(&ts, req, sizeof(struct timespec)) < 0)
        return -EFAULT;
    if (ts.tv_nsec >= 1000000000 || ts.tv_sec < 0 || ts.tv_nsec < 0)
        return -EINVAL;
    /* 如果小于2毫秒就用延时的方式 */
    if (ts.tv_nsec < (2000L * 1000) && ts.tv_sec == 0) {
        udelay(ts.tv_nsec);
        return 0;
    }
    ticks = timespec_to_systicks(&ts);
    ticks = task_sleep_by_ticks(ticks);
    if (ticks > 0) {
        if (rem) {
            systicks_to_timespec(ticks, &ts);
            if (mem_copy_to_user(rem, &ts, sizeof(struct timespec)) < 0)
                return -EFAULT;
        }
        return -EINTR;
    }
    return 0;
}
