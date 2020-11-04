#ifndef _XBOOK_ALARM_H
#define _XBOOK_ALARM_H

/* 闹钟结构 */
typedef struct alarm_struct {
    char flags;              /* 有效标志 */
    unsigned long ticks;    /* ticks计数 */
    unsigned long second;   /* 秒数 */
} alarm_t;

static inline void alarm_init(alarm_t *alarm)
{
    alarm->flags = 0;
    alarm->ticks = 0;
    alarm->second = 0;
}

unsigned long sys_alarm(unsigned long second);
void alarm_update_ticks();

#endif   /* _XBOOK_ALARM_H */
