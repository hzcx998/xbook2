#include <sys/time.h>
#include <xbook/walltime.h>
#include <arch/time.h>
#include <xbook/clock.h>
#include <xbook/schedule.h>
#include <xbook/safety.h>

int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    if (tv) {
        struct timeval tmp_tv;
        tmp_tv.tv_sec = walltime_make_timestamp(&walltime);
        tmp_tv.tv_usec = ((systicks % HZ) * MS_PER_TICKS) * 1000;
        if (mem_copy_to_user(tv, &tmp_tv, sizeof(struct timeval)) < 0)
            return -1;
    }
    if (tz) {
        struct timezone tmp_tz;
        tmp_tz.tz_dsttime = 0;
        tmp_tz.tz_minuteswest = 0;
        if (mem_copy_to_user(tz, &tmp_tz, sizeof(struct timezone)) < 0)
            return -1;
    }
    return 0;
}

int sys_clock_gettime(clockid_t clockid, struct timespec *ts)
{
    if (!ts)
        return -1; 
    struct timespec tmp_ts;
    switch (clockid)
    {
    case CLOCK_REALTIME:        /* 系统统当前时间，从1970年1.1日算起 */
        tmp_ts.ts_sec = walltime_make_timestamp(&walltime);
        tmp_ts.ts_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_MONOTONIC:       /*系统的启动时间，不能被设置*/
        tmp_ts.ts_sec = (systicks / HZ);
        tmp_ts.ts_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_PROCESS_CPUTIME_ID:  /* 本进程运行时间*/
        tmp_ts.ts_sec = task_current->elapsed_ticks / HZ;
        tmp_ts.ts_nsec = ((task_current->elapsed_ticks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_THREAD_CPUTIME_ID:   /*本线程运行时间*/
        tmp_ts.ts_sec = task_current->elapsed_ticks / HZ;
        tmp_ts.ts_nsec = ((task_current->elapsed_ticks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    default:
        return -1;
    }
    return mem_copy_to_user(ts, &tmp_ts, sizeof(struct timespec));
}

#define MAX_SYSTICKS_VALUE  ((~0UL >> 1) -1)

unsigned long timeval_to_systicks(struct timeval *tv)
{
    unsigned long sec = tv->tv_sec;
    unsigned long usec = tv->tv_usec;
    if (sec >= (MAX_SYSTICKS_VALUE / HZ))
        return MAX_SYSTICKS_VALUE;
    usec /= 1000000L / HZ;  /* 秒/HZ=1秒的ticks数 */
    return HZ * sec + usec;
}

void systicks_to_timeval(unsigned long ticks, struct timeval *tv)
{
    unsigned long sec = ticks / HZ; 
    unsigned long usec = (ticks % HZ) * MS_PER_TICKS;
    usec *= 1000L;
    if (sec >= (MAX_SYSTICKS_VALUE / HZ))
        sec = MAX_SYSTICKS_VALUE;
    tv->tv_sec = sec;
    tv->tv_usec = usec;
}

unsigned long timespec_to_systicks(struct timespec *ts)
{
    unsigned long sec = ts->ts_sec;
    unsigned long nsec = ts->ts_nsec;
    if (sec >= (MAX_SYSTICKS_VALUE / HZ))
        return MAX_SYSTICKS_VALUE;
    nsec /= 1000000000L / HZ;  /* 秒/HZ=1秒的ticks数 */
    return HZ * sec + nsec;
}

void systicks_to_timespec(unsigned long ticks, struct timespec *ts)
{
    unsigned long sec = ticks / HZ;
    unsigned long nsec = (ticks % HZ) * MS_PER_TICKS;
    nsec *= 1000000L;
    if (sec >= (MAX_SYSTICKS_VALUE / HZ))
        sec = MAX_SYSTICKS_VALUE;
    ts->ts_sec = sec;
    ts->ts_nsec = nsec;
}
