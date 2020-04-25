#include <xbook/assert.h>
#include <xbook/timer.h>
#include <arch/interrupt.h>

LIST_HEAD(timer_list_head);

void timer_add(timer_t *timer)
{
    unsigned long flags;
    save_intr(flags);
    /* 定时器必须不在队列中 */
    ASSERT(!list_find(&timer->list, &timer_list_head));
    list_add_tail(&timer->list, &timer_list_head);
    restore_intr(flags);
}

void timer_del(timer_t *timer)
{
    unsigned long flags;
    save_intr(flags);
    /* 定时器必须在队列中 */
    ASSERT(list_find(&timer->list, &timer_list_head));
    list_del(&timer->list);
    restore_intr(flags);
}

void timer_mod(timer_t *timer, unsigned long timeout)
{
    unsigned long flags;
    save_intr(flags);
    timer->timeout = timeout;
    restore_intr(flags);
}

int timer_cancel(timer_t *timer)
{
    int retval = -1;
    if (timer) {
        timer_del(timer);
        retval = 0;
    }
    return retval;
}

static void do_timer_action(timer_t *timer)
{
    --timer->timeout;
    if (timer->timeout <= 0) {
        list_del(&timer->list); /* 从定时器链表删除 */
        /* 调用定时器回调 */
        timer->callback(timer->arg);
    }
}

/**
 * update_timers - 更新定时器
 * 
 */
void update_timers()
{
    timer_t *timer, *next;
    unsigned long flags;
    save_intr(flags);
    list_for_each_owner_safe (timer, next, &timer_list_head, list) {
        do_timer_action(timer);
    }
    restore_intr(flags);
}