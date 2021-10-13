#include <assert.h>
#include <xbook/timer.h>
#include <sys/walltime.h>
#include <xbook/task.h>
#include <xbook/clock.h>
#include <xbook/safety.h>
#include <errno.h>
#include <string.h>
#include <arch/interrupt.h>
#include <arch/time.h>

/* 10day = 10 * 60 * 60 * 24 * HZ */
#define TIMER_IDLE_TIMEOUT  (864000 * HZ)

LIST_HEAD(timer_list_head);
unsigned long timer_id_next = 1; /* 从1开始，0是无效的id */
static clock_t minim_timeout_val = 0;
static timer_t timer_idle;

/* idle定时器超时
 * 重新调整所有定时器的值，所有值都减去timer_ticks（idle除外）
 */
static void timer_idle_handler(timer_t *timer_self, void *arg)
{
    clock_t dt = timer_idle.timeout;
    timer_ticks -= dt;
    timer_t *tmp;
    list_for_each_owner (tmp, &timer_list_head, list) {
        if (tmp != &timer_idle)
            tmp->timeout -= dt;
    }
    timer_add(&timer_idle);
}

void timer_init(
    timer_t *timer,
    unsigned long timeout,
    void *arg,
    timer_callback_t callback)
{
    list_init(&timer->list);
    timer->timeval = timeout;
    timer->timeout = timer_ticks + timeout;
    timer->arg = arg;
    timer->id = timer_id_next++;
    timer->callback = callback;
    timer->flags = 0;
}

void timer_add(timer_t *timer)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    if (!timer->id)
        timer->id = timer_id_next++;
    assert(!list_find(&timer->list, &timer_list_head));
    if (list_empty(&timer_list_head)) {    
        list_add_tail(&timer->list, &timer_list_head);
        minim_timeout_val = timer->timeout;
    } else {
        timer_t *first = list_first_owner(&timer_list_head, timer_t, list);
        if (timer->timeout < first->timeout) {
            list_add(&timer->list, &timer_list_head);
            minim_timeout_val = timer->timeout;
        } else {
            timer_t *tmp;
            list_for_each_owner (tmp, &timer_list_head, list) {
                if (timer->timeout >= tmp->timeout) {
                    list_add_after(&timer->list, &tmp->list);
                    break;
                }
            }
        }
    }
    interrupt_restore_state(flags);
}

void timer_del(timer_t *timer)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    assert(list_find(&timer->list, &timer_list_head));
    list_del(&timer->list);
    interrupt_restore_state(flags);
}

int timer_alive(timer_t *timer)
{
    int alive = 0; 
    unsigned long flags;
    interrupt_save_and_disable(flags);
    if (list_find(&timer->list, &timer_list_head))
        alive = 1;
    interrupt_restore_state(flags);
    return alive;
}

void timer_modify(timer_t *timer, unsigned long timeout)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    timer->timeout = timer_ticks + timeout;
    interrupt_restore_state(flags);
}

timer_t *timer_find(unsigned long id)
{
    timer_t *timer, *tmr_find = NULL;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    list_for_each_owner (timer, &timer_list_head, list) {
        if (timer->id == id) {
            tmr_find = timer;
            break;
        }
    }
    interrupt_restore_state(flags);
    return tmr_find;
}

int timer_cancel(timer_t *timer)
{
    int retval = -1;
    if (timer) {
        if (timer_alive(timer)) {
            timer_del(timer);
        }
        retval = 0;
    }
    return retval;
}

void timer_add_period(timer_t *timer)
{
    timer->flags |= TIMER_PERIOD;
}

void timer_del_period(timer_t *timer)
{
    timer->flags &= ~TIMER_PERIOD;
}

static void timer_do_action(timer_t *timer)
{
    if (!(timer->flags & TIMER_PERIOD))
    {
        /* 不是周期定时才删除定时器 */
        list_del(&timer->list);
    }
    else
    {
        /* 不删除定时器，并更新定时器超时值 */
        timer->timeout = timer_ticks + timer->timeval;
    }
    timer->callback(timer, timer->arg);
}

void timer_update_ticks()
{
    timer_t *timer, *next;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    if (timer_ticks < minim_timeout_val) { // no timer timeout
        interrupt_restore_state(flags);
        return;
    }
    list_for_each_owner_safe (timer, next, &timer_list_head, list) {
        if (timer->timeout > timer_ticks) {
            break;
        }
        timer_do_action(timer); // time out
    }

    /* 寻找超时最小的定时器超时值 */
    list_for_each_owner (timer, &timer_list_head, list) {
        if (timer->timeout > timer_ticks) {
            break;
        }
    }
    minim_timeout_val = timer->timeout;
    interrupt_restore_state(flags);
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

int timers_init()
{
    timer_init(&timer_idle, TIMER_IDLE_TIMEOUT, NULL, timer_idle_handler);
    timer_add(&timer_idle);
    return 0;
}