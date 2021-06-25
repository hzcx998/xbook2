#ifndef _XBOOK_WALLTIME_H
#define _XBOOK_WALLTIME_H

#include <sys/walltime.h>

extern walltime_t walltime;
void walltime_update_second();
void walltime_printf();
void walltime_init();
int sys_get_walltime(walltime_t *wt);
long walltime_make_timestamp(walltime_t *wt);

void walltime_get_date(int second, int *year, int *month, int *day);
void walltime_get_time(int seconds, int *hour, int *minute, int *second);
int walltime_get_week_day(int year, int month, int day);
int walltime_get_year_days();

#endif   /* _XBOOK_WALLTIME_H */
