#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "walltime.h"
#include <sys/types.h>

#define CLOCK_REALTIME            0 /*系统统当前时间，从1970年1.1日算起*/
#define CLOCK_MONOTONIC           1 /*系统的启动时间，不能被设置*/
#define CLOCK_PROCESS_CPUTIME_ID  2 /* 本进程运行时间*/
#define CLOCK_THREAD_CPUTIME_ID   3 /*本线程运行时间*/

#define CLOCKS_PER_SEC  (100 * 5)   /* 1秒的时钟数 */
#define HZ_PER_CLOCKS   (CLOCKS_PER_SEC / 100)   /* 每个时钟的HZ数 */

/* 1 ticks 对应的毫秒数 */
#define MS_PER_TICKS (1000 / CLOCKS_PER_SEC)

/* 毫秒转换成ticks */
#define MSEC_TO_TICKS(msec) ((msec) / MS_PER_TICKS)

/* ticks转换成毫秒 */
#define TICKS_to_MSEC(ticks) ((ticks) * MS_PER_TICKS)

struct timeval {
    long tv_sec;         /* seconds */
    long tv_usec;        /* and microseconds */
};

struct timezone{ 
    int tz_minuteswest; //miniutes west of Greenwich 
    int tz_dsttime; //type of DST correction 
};

#ifndef _TIMESPEC
#define _TIMESPEC
struct timespec {
    time_t tv_sec; // seconds
    long tv_nsec; // and nanoseconds
};
#endif

int gettimeofday(struct timeval *tv, struct timezone *tz);
int clock_gettime(clockid_t clockid, struct timespec *ts);
clock_t getticks(void);
unsigned long alarm(unsigned long second);
void mdelay(time_t msec);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_TIME_H */
