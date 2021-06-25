#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <types.h>
#include <xbook/config.h>

#define CLOCK_REALTIME            0 /*系统统当前时间，从1970年1.1日算起*/
#define CLOCK_MONOTONIC           1 /*系统的启动时间，不能被设置*/
#define CLOCK_PROCESS_CPUTIME_ID  2 /* 本进程运行时间*/
#define CLOCK_THREAD_CPUTIME_ID   3 /*本线程运行时间*/

// #define CLOCK_NO_TASK

struct timeval {
    long tv_sec;         /* seconds */
    long tv_usec;        /* and microseconds */
};

struct timezone { 
    int tz_minuteswest; //miniutes west of Greenwich 
    int tz_dsttime; //type of DST correction 
};

struct timespec {
    time_t tv_sec; // seconds
    long tv_nsec; // and nanoseconds
};

struct tms {
    clock_t tms_utime; //用户CPU时间
    clock_t tms_stime; //系统CPU时间
    clock_t tms_cutime; //以终止子进程的用户CPU时间
    clock_t tms_cstime; //已终止子进程的系统CPU时间
};

struct itimerval {
    struct timeval it_interval; /* next value */
    struct timeval it_value;    /* current value */
};

int sys_gettimeofday(struct timeval *tv, struct timezone *tz);
int sys_clock_gettime(clockid_t clockid, struct timespec *ts);
int sys_clock_settime(clockid_t clockid, const struct timespec *ts);

unsigned long timeval_to_systicks(struct timeval *tv);
void systicks_to_timeval(unsigned long ticks, struct timeval *tv);
unsigned long timespec_to_systicks(struct timespec *ts);
void systicks_to_timespec(unsigned long ticks, struct timespec *ts);
#if !defined(CONFIG_NO_SYS_TIMES)
clock_t sys_times(struct tms *buf);
#endif

#endif  /* _SYS_TIME_H */
