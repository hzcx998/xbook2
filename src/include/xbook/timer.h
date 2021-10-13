#ifndef _XBOOK_TIMER_H
#define _XBOOK_TIMER_H

#include <xbook/list.h>
#include "memcache.h"
#include <sys/time.h>

struct timer_struct;
// callback(timer, arg)
typedef void (*timer_callback_t) (struct timer_struct *, void *); 

#define TIMER_PERIOD 0x01

/* 定时器 */
typedef struct timer_struct {
    list_t list;                /* 定时器链表 */
    clock_t timeout;            /* 超时点，以ticks为单位 */
    clock_t timeval;            /* 超时值 */
    void *arg;                  /* 参数 */
    unsigned long id;           /* 定时器的id值 */
    int flags;
    timer_callback_t callback;  /* 回调函数 */
} timer_t;

#define TIMER_INIT(timer, _timeout, _timeval, _arg, _callback) \
    { .list = LIST_HEAD_INIT((timer).list) \
    , .timeout = (_timeout) \
    , .timeval = (_timeval) \
    , .arg = (_arg) \
    , .id = (0) \
    , .flags = (0) \
    , .callback = (_callback) \
    }

#define DEFINE_TIMER(timer_name, timeout, timeval, arg, callback) \
    timer_t timer_name = TIMER_INIT(timer_name, timeout, timeval, arg, callback); \

#define timer_alloc()       mem_alloc(sizeof(timer_t))
#define timer_free(timer)   mem_free(timer)

#define timer_set_handler(tmr, handler) (tmr)->callback = (timer_callback_t)(handler)
#define timer_set_arg(tmr, _arg) (tmr)->arg = (void *)(_arg)

void timer_init(
    timer_t *timer,
    unsigned long timeout,
    void *arg,
    timer_callback_t callback);

void timer_add(timer_t *timer);
void timer_del(timer_t *timer);
void timer_modify(timer_t *timer, unsigned long timeout);
int timer_cancel(timer_t *timer);
int timer_alive(timer_t *timer);
void timer_add_period(timer_t *timer);
void timer_del_period(timer_t *timer);

timer_t *timer_find(unsigned long id);

void timer_update_ticks();
long sys_usleep(struct timeval *inv, struct timeval *outv);

int timers_init();

#endif   /* _XBOOK_TIMER_H */
