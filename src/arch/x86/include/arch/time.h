#ifndef _X86_TIME_H
#define _X86_TIME_H

unsigned int cmos_get_hour_hex();
unsigned int cmos_get_min_hex();
unsigned char cmos_get_min_hex8();
unsigned int cmos_get_sec_hex();
unsigned int cmos_get_day_of_month();
unsigned int cmos_get_day_of_week();
unsigned int cmos_get_mon_hex();
unsigned int cmos_get_year();

void __init_pit_clock();
/* clock hardware init */
#define __init_clock_hardware       __init_pit_clock

/* HZ */
#define __HZ                (100 * 5)

/* get time */
#define __get_time_hour     cmos_get_hour_hex
#define __get_time_minute   cmos_get_min_hex
#define __get_time_second   cmos_get_sec_hex
#define __get_time_day      cmos_get_day_of_month
#define __get_time_month    cmos_get_mon_hex
#define __get_time_year     cmos_get_year
#define __get_time_week     cmos_get_day_of_week

void __udelay(int usec);

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

#endif  /* _X86_TIME_H */
