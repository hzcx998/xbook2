#include <sys/time.h>
#include <xbook/walltime.h>
#include <arch/time.h>
#include <xbook/clock.h>
#include <xbook/schedule.h>

int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    if (!tv && !tv)
        return -1;

    if (tv) {
        tv->tv_sec = walltime_make_timestamp(&walltime);
        tv->tv_usec = ((systicks % HZ) * MS_PER_TICKS) * 1000;
    }
    if (tz) {
        tz->tz_dsttime = 0;
        tz->tz_minuteswest = 0;
    }
    return 0;
}

int sys_clock_gettime(clockid_t clockid, struct timespec *ts)
{
    if (!ts)
        return -1; 
    switch (clockid)
    {
    case CLOCK_REALTIME:        /* 系统统当前时间，从1970年1.1日算起 */
        ts->tv_sec = walltime_make_timestamp(&walltime);
        ts->tv_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_MONOTONIC:       /*系统的启动时间，不能被设置*/
        ts->tv_sec = (systicks / HZ);
        ts->tv_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_PROCESS_CPUTIME_ID:  /* 本进程运行时间*/
        ts->tv_sec = current_task->elapsed_ticks / HZ;
        ts->tv_nsec = ((current_task->elapsed_ticks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_THREAD_CPUTIME_ID:   /*本线程运行时间*/
        ts->tv_sec = current_task->elapsed_ticks / HZ;
        ts->tv_nsec = ((current_task->elapsed_ticks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    default:
        return -1;
    }
    return 0;
}

#define MAX_SYSTICKS_VALUE  ((~0UL >> 1) -1)

unsigned long timeval_to_systicks(struct timeval *tv)
{
    unsigned long sec = tv->tv_sec;
    unsigned long usec = tv->tv_usec;
    if (sec >= (MAX_SYSTICKS_VALUE / HZ))
        return MAX_SYSTICKS_VALUE;
    usec -= 1000000L / HZ - 1;
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
    unsigned long sec = ts->tv_sec;
    unsigned long nsec = ts->tv_nsec;
    if (sec >= (MAX_SYSTICKS_VALUE / HZ))
        return MAX_SYSTICKS_VALUE;
    nsec -= 1000000000L / HZ - 1;
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
    ts->tv_sec = sec;
    ts->tv_nsec = nsec;
}
