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

/* 30day = 30 * 60 * 60 * 24 * HZ */
#define TIMER_IDLE_TIMEOUT  (2592000 * HZ)

LIST_HEAD(timer_list_head);
unsigned long timer_id_next = 1; /* 从1开始，0是无效的id */
static clock_t minim_timeout_val = 0;

static void timer_idle_handler(unsigned long arg);
static DEFINE_TIMER(timer_idle, TIMER_IDLE_TIMEOUT, 0, timer_idle_handler);

/* idle定时器超时
 * 重新调整所有定时器的值，所有值都减去timer_ticks（idle除外）
 */
static void timer_idle_handler(unsigned long arg)
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
    unsigned long arg,
    timer_callback_t callback)
{
    list_init(&timer->list);
    timer->timeout = timer_ticks + timeout;
    timer->arg = arg;
    timer->id = timer_id_next++;
    timer->callback = callback;
}

void timer_add(timer_t *timer)
{
    unsigned long flags;
    interrupt_save_and_disable(flags);
    if (!timer->id)
        timer->id = timer_id_next++;
    ASSERT(!list_find(&timer->list, &timer_list_head));
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
    ASSERT(list_find(&timer->list, &timer_list_head));
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

static void timer_do_action(timer_t *timer)
{
    list_del(&timer->list);
    timer->callback(timer->arg);
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
    timer_add(&timer_idle);
    return 0;
}