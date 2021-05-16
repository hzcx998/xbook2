#ifndef _RSICV64_TIME_H
#define _RSICV64_TIME_H

#include <k210_rtc.h>

#define HZ                (100)

void timer_interrupt_set_next_timeout();
void clock_handler2(int irqno, void *data);
void timer_interrupt_init();

#define clock_hardware_init timer_interrupt_init

#define time_get_hour       rtc_get_hour_hex
#define time_get_minute     rtc_get_min_hex
#define time_get_second     rtc_get_sec_hex
#define time_get_day        rtc_get_day_of_month
#define time_get_month      rtc_get_mon_hex
#define time_get_year       rtc_get_year
#define time_get_week       rtc_get_day_of_week

#endif  /* _RSICV64_TIME_H */
