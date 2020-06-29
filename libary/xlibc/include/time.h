#ifndef _LIB_TIME_H
#define _LIB_TIME_H

#include "types.h"


struct tm
{
  int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
  int tm_min;			/* Minutes.	[0-59] */
  int tm_hour;			/* Hours.	[0-23] */
  int tm_mday;			/* Day.		[1-31] */
  int tm_mon;			/* Month.	[0-11] */
  int tm_year;			/* Year	- 1900.  */
  int tm_wday;			/* Day of week.	[0-6] */
  int tm_yday;			/* Days in year.[0-365]	*/
  int tm_isdst;			/* DST.		[-1/0/1]*/
};

time_t time(time_t *t);

/* 结构转换成时间戳 */
time_t mktime(struct tm *tp);

/* 时间戳转换成结构 */
struct tm *gmtime(const time_t *t);                                          
struct tm *localtime(const time_t *t);

/* tm和t转换成字符 */
char *asctime(const struct tm *tp);
char *ctime(const time_t *t);

/* difftime函数计算t1-t2的差，并把结果值转换为秒，返回一个double类型。 */
double difftime(time_t t1, time_t t0);


#endif  /* _LIB_TIME_H */
