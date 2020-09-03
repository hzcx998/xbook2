#ifndef _XBOOK_TIMER_H
#define _XBOOK_TIMER_H

#include <list.h>
#include "memcache.h"
#include "ktime.h"

typedef void (*timer_callback_t) (unsigned long); 

/* 定时器 */
typedef struct timer_struct {
    list_t list;                /* 定时器链表 */
    long timeout;      /* 超时计数器，以ticks为单位 */
    unsigned long arg;          /* 参数 */
    unsigned long id;           /* 定时器的id值 */
    timer_callback_t callback;    /* 回调函数 */
} timer_t;

#define TIMER_INIT(timer, _timeout, _arg, _callback) \
    { .list = LIST_HEAD_INIT((timer).list) \
    , .timeout = (_timeout) \
    , .arg = (_arg) \
    , .id = (0) \
    , .callback = (_callback) \
    }

#define DEFINE_TIMER(timer_name, timeout, arg, callback) \
    timer_t timer_name = TIMER_INIT(timer_name, timeout, arg, callback); \

#define timer_alloc()       kmalloc(sizeof(timer_t))
#define timer_free(timer)   kfree(timer)

void timer_init(
    timer_t *timer,
    unsigned long timeout,
    unsigned long arg,
    timer_callback_t callback);

void timer_add(timer_t *timer);
void timer_del(timer_t *timer);
void timer_mod(timer_t *timer, unsigned long timeout);
int timer_cancel(timer_t *timer);
int timer_alive(timer_t *timer);

void update_timers();
long sys_usleep(struct timeval *inv, struct timeval *outv);


#endif   /* _XBOOK_TIMER_H */
