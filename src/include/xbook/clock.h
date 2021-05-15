
#ifndef _XBOOK_CLOCK_H
#define _XBOOK_CLOCK_H

#include <types.h>
#include <arch/time.h>

/* 1 ticks 对应的毫秒数 */
#define MS_PER_TICKS (1000 / HZ)

/* 毫秒转换成ticks */
#define MSEC_TO_TICKS(msec) ((msec) / MS_PER_TICKS)

/* ticks转换成毫秒 */
#define TICKS_to_MSEC(ticks) ((ticks) * MS_PER_TICKS)

extern volatile clock_t systicks;   // system ticks
extern volatile clock_t timer_ticks;    // timer ticks

/* 基于systicks的时间 */
#define time_after(unknown, known) ((long)(known) - (long)(unknown) < 0)
#define time_before(unknown, known) ((long)(unknown) - (long)(known) < 0)
#define time_after_eq(unknown, known) ((long)(known) - (long)(unknown) <= 0)
#define time_before_eq(unknown, known) ((long)(unknown) - (long)(known) <= 0)

void clock_init();
void clock_msleep(unsigned long msecond);

clock_t sys_get_ticks();
clock_t clock_delay_by_ticks(clock_t ticks);
void mdelay(time_t msec);

/* 全局函数 */
int clock_handler(irqno_t irq, void *data);


#endif  /* _XBOOK_CLOCK_H */
