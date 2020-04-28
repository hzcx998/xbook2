
#ifndef _XBOOK_CLOCK_H
#define _XBOOK_CLOCK_H

#include "types.h"
#include <arch/time.h>

/* 1 ticks 对应的毫秒数 */
#define MS_PER_TICKS (1000 / HZ)

extern volatile clock_t systicks;
/* 基于systicks的时间 */
#define time_after(unknown, known) ((long)(known) - (long)(unknown) < 0)
#define time_before(unknown, known) ((long)(unknown) - (long)(known) < 0)
#define time_after_eq(unknown, known) ((long)(known) - (long)(unknown) <= 0)
#define time_before_eq(unknown, known) ((long)(unknown) - (long)(known) <= 0)

void init_clock();
void clock_msleep(unsigned long msecond);
void loop_delay(int t);
clock_t sys_get_ticks();

#endif  /* _XBOOK_CLOCK_H */
