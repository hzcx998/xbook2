#ifndef _X86_TIME_H
#define _X86_TIME_H

#include "config.h"
#include "cmos.h"

void __init_pit_clock();
/* clock hardware init */
#define __init_clock_hardware       __init_pit_clock

/* HZ */
#define __HZ                1000

/* get time */
#define __get_time_hour     cmos_get_hour_hex
#define __get_time_minute   cmos_get_min_hex
#define __get_time_second   cmos_get_sec_hex
#define __get_time_day      cmos_get_day_of_month
#define __get_time_month    cmos_get_mon_hex
#define __get_time_year     cmos_get_year
#define __get_time_week     cmos_get_day_of_week

#endif	/* _X86_TIME_H */