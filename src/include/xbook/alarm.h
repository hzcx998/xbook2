#ifndef _XBOOK_ALARM_H
#define _XBOOK_ALARM_H

#include <sys/time.h>

#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF 2

#define ALARM_ACITVE    1
#define ALARM_INACITVE  0

/* 闹钟结构 */
typedef struct alarm_struct {
    int flags;              /* 有效标志 */
    clock_t ticks;    /* ticks计数 */
    struct itimerval itimer; /* 间隔定时器 */
    int which;              /* 哪种闹钟定时器 */
} alarm_t;

static inline void alarm_init(alarm_t *alarm)
{
    alarm->flags = ALARM_INACITVE;
    alarm->ticks = 0;
    alarm->which = ITIMER_REAL;
}

unsigned long sys_alarm(unsigned long second);
void alarm_update_ticks();

int sys_getitimer(int which, struct itimerval *value);
int sys_setitimer(int which, const struct itimerval *new_value,
    struct itimerval *old_value);

#endif   /* _XBOOK_ALARM_H */
