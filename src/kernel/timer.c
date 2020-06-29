#include <xbook/assert.h>
#include <xbook/timer.h>
#include <xbook/ktime.h>
#include <xbook/task.h>
#include <errno.h>
#include <arch/interrupt.h>
#include <arch/time.h>

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

/**
 * 以微妙为单位进行休眠。
 * 如果毫秒数小于等于2就是延时
 * 不然就是定时器阻塞休眠。
 */
long sys_usleep(struct timeval *inv, struct timeval *outv)
{
    struct timeval tv;
    unsigned long ticks;

    memcpy(&tv, inv, sizeof(struct timeval));

    /* 检测参数 */
    if (tv.tv_usec >= 1000000 || tv.tv_sec < 0 || tv.tv_usec < 0)
        return -EINVAL;

    /* 如果小于2毫秒就用延时的方式 */
    if (tv.tv_usec < 2000L || tv.tv_sec == 0) {
        udelay(tv.tv_usec);
        return 0;
    }
    /* 计算ticks */
    ticks = timeval_to_systicks(&tv);
    /* 休眠一定的ticks */
    ticks = task_sleep_by_ticks(ticks);
    /* 如果还剩下ticks，就传回去 */
    if (ticks > 0) {
        if (outv) {
            systicks_to_timeval(ticks, &tv);
            memcpy(outv, &tv, sizeof(struct timeval));
        }
        return -EINTR;  /* 休眠被打断 */
    }
    return 0;
}
