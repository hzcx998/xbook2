#ifndef __TIME_H__
#define __TIME_H__

#include <stddef.h>

#define CLOCK_REALTIME            0 /*系统统当前时间，从1970年1.1日算起*/
#define CLOCK_MONOTONIC           1 /*系统的启动时间，不能被设置*/
#define CLOCK_PROCESS_CPUTIME_ID  2 /* 本进程运行时间*/
#define CLOCK_THREAD_CPUTIME_ID   3 /*本线程运行时间*/

#ifndef _TIMESPEC
#define _TIMESPEC
struct timespec {
    time_t tv_sec; // seconds
    long tv_nsec; // and nanoseconds
};
#endif

int clock_gettime(clockid_t clockid, struct timespec *tp);
int clock_settime(clockid_t clockid, const struct timespec *tp);

#endif //__TIME_H__
