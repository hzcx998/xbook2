#include <assert.h>
#include <xbook/timer.h>
#include <xbook/ktime.h>
#include <xbook/task.h>
#include <xbook/clock.h>
#include <errno.h>
#include <string.h>
#include <arch/interrupt.h>
#include <arch/time.h>

// #define DEBUG_TIMER

LIST_HEAD(timer_list_head);

unsigned long timer_id_next = 1; /* 从1开始，0是无效的id */

/* 下一个超时的值 */
static clock_t minim_timeout_val = 0;

/* 30day = 30 * 60 * 60 * 24 * HZ */
#define TIMER_IDLE_TIMEOUT  (2592000 * HZ)

static void timer_idle_handler(unsigned long arg);
/* 定义idle定时器 */
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
    timer_add(&timer_idle); // add idle again
    #ifdef DEBUG_TIMER 
    printk(KERN_NOTICE "[timer]: timer idle timeout!\n");
    #endif
}

void timer_init(
    timer_t *timer,
    unsigned long timeout,
    unsigned long arg,
    timer_callback_t callback)
{
    INIT_LIST_HEAD(&timer->list);
    timer->timeout = timer_ticks + timeout;
    timer->arg = arg;
    timer->id = timer_id_next++;
    timer->callback = callback;
}

/**
 * 将定时器按超时点进行排序，插入到合适的位置，从小到大插入排序
 * 
 */
void timer_add(timer_t *timer)
{
    unsigned long flags;
    interrupt_save_state(flags);
    if (!timer->id) /* 无效id，重新分配 */
        timer->id = timer_id_next++;
    /* 定时器必须不在队列中 */
    ASSERT(!list_find(&timer->list, &timer_list_head));
    
    if (list_empty(&timer_list_head)) { // null list, add to tail    
        list_add_tail(&timer->list, &timer_list_head);
        minim_timeout_val = timer->timeout;
        #ifdef DEBUG_TIMER 
        printk("[timer]: timer list is null, timeout %d - %x add to tail!\n", timer->timeout, timer->timeout);
        #endif /* DEBUG_TIMER */
    } else {
        /* 插入最前面或者中间 */
        timer_t *first = list_first_owner(&timer_list_head, timer_t, list);
        if (timer->timeout < first->timeout) { // add to head
            list_add(&timer->list, &timer_list_head);
            minim_timeout_val = timer->timeout;
            #ifdef DEBUG_TIMER 
            printk("[timer]: timer is early, timeout %d add to head!\n", timer->timeout);
            #endif /* DEBUG_TIMER */
        } else {    // add to middle
            timer_t *tmp;
            list_for_each_owner (tmp, &timer_list_head, list) {
                if (timer->timeout >= tmp->timeout) {
                    list_add_after(&timer->list, &tmp->list);
                    #ifdef DEBUG_TIMER 
                    printk("[timer]: timer is normal, timeout %d add to middle!\n", timer->timeout);
                    #endif /* DEBUG_TIMER */
                    break;
                }
            }
        }
    }
    /* 根据超时的时间点插入到对应的位置 */
    interrupt_restore_state(flags);
}

void timer_del(timer_t *timer)
{
    unsigned long flags;
    interrupt_save_state(flags);
    /* 定时器必须在队列中 */
    ASSERT(list_find(&timer->list, &timer_list_head));
    list_del(&timer->list);
    interrupt_restore_state(flags);
}

int timer_alive(timer_t *timer)
{
    int alive = 0; 
    unsigned long flags;
    interrupt_save_state(flags);
    /* 定时器必须在队列中 */
    if (list_find(&timer->list, &timer_list_head))
        alive = 1;
    interrupt_restore_state(flags);
    return alive;
}

void timer_mod(timer_t *timer, unsigned long timeout)
{
    unsigned long flags;
    interrupt_save_state(flags);
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

static void do_timer_action(timer_t *timer)
{
    list_del(&timer->list); /* 从定时器链表删除 */
    /* 调用定时器回调 */
    timer->callback(timer->arg);
}

/**
 * update_timers - 更新定时器
 * 
 */
void update_timers()
{
    timer_t *timer, *next;
    unsigned long flags;
    interrupt_save_state(flags);
    if (timer_ticks < minim_timeout_val) { // no timer timeout
        interrupt_restore_state(flags);
        return;
    }
    list_for_each_owner_safe (timer, next, &timer_list_head, list) {
        if (timer->timeout > timer_ticks) {
            break;
        }
        do_timer_action(timer); // time out
    }

    /* update minim */
    minim_timeout_val = timer->timeout;

    interrupt_restore_state(flags);
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
    #ifdef DEBUG_TIMER 
    printk("[timer]: usleep sec %d, usec %d\n", tv.tv_sec, tv.tv_usec);
    #endif
    /* 检测参数 */
    if (tv.tv_usec >= 1000000 || tv.tv_sec < 0 || tv.tv_usec < 0)
        return -EINVAL;

    /* 如果小于2毫秒就用延时的方式 */
    if (tv.tv_usec < 2000L && tv.tv_sec == 0) {
        udelay(tv.tv_usec);
        return 0;
    }
    /* 计算ticks */
    ticks = timeval_to_systicks(&tv);
    #ifdef DEBUG_TIMER 
    printk("[timer]: usleep ticks %d\n", ticks);
    #endif
    /* 休眠一定的ticks */
    ticks = task_sleep_by_ticks(ticks);
    #ifdef DEBUG_TIMER 
    printk("[timer]: usleep left ticks %d\n", ticks);
    #endif
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

int init_timer_system()
{
    /* 创建一个idle定时器，并插入到链表中 */
    timer_add(&timer_idle);

    return 0;
}