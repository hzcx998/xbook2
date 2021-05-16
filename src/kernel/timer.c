#include <assert.h>
#include <xbook/timer.h>
#include <sys/walltime.h>
#include <xbook/task.h>
#include <xbook/clock.h>
#include <xbook/safety.h>
#include <xbook/debug.h>
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

static void timer_do_action(timer_t *timer)
{
    list_del(&timer->list);
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
    /* 在执行action过程中，可能会添加新的定时器，并且比现在超时的定时器的值更小，
    于是只有超时的值大于minim值时才更新minim */
    if (minim_timeout_val > timer->timeout)
        minim_timeout_val = timer->timeout;
    interrupt_restore_state(flags);
}

int timers_init()
{
    timer_init(&timer_idle, TIMER_IDLE_TIMEOUT, NULL, timer_idle_handler);
    timer_add(&timer_idle);
    return 0;
}