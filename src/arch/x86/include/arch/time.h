#ifndef _X86_TIME_H
#define _X86_TIME_H

#include "cpu.h"
#include "cmos.h"

#define HZ                (100 * 5)

void pit_clock_init();

#define clock_hardware_init pit_clock_init

#define time_get_hour       cmos_get_hour_hex
#define time_get_minute     cmos_get_min_hex
#define time_get_second     cmos_get_sec_hex
#define time_get_day        cmos_get_day_of_month
#define time_get_month      cmos_get_mon_hex
#define time_get_year       cmos_get_year
#define time_get_week       cmos_get_day_of_week

#endif  /* _X86_TIME_H */
