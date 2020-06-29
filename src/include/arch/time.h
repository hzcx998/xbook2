#ifndef _ARCH_TIME_H
#define _ARCH_TIME_H

#include "general.h"

/* clock hardware init */
#define init_clock_hardware __init_pit_clock

/* HZ */
#define HZ                  __HZ

/* get time */
#define get_time_hour       __get_time_hour
#define get_time_minute     __get_time_minute
#define get_time_second     __get_time_second
#define get_time_day        __get_time_day
#define get_time_month      __get_time_month
#define get_time_year       __get_time_year
#define get_time_week       __get_time_week

#define udelay              __udelay

#endif  /* _ARCH_TIME_H */
