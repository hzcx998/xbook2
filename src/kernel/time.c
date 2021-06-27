#include <sys/time.h>
#include <xbook/walltime.h>
#include <arch/time.h>
#include <xbook/clock.h>
#include <xbook/schedule.h>
#include <xbook/safety.h>
#include <errno.h>

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
    walltime_printf();
    struct timespec tmp_ts;
    switch (clockid)
    {
    case CLOCK_REALTIME:        /* 系统统当前时间，从1970年1.1日算起 */
        tmp_ts.tv_sec = walltime_make_timestamp(&walltime);
        tmp_ts.tv_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_MONOTONIC:       /*系统的启动时间，不能被设置*/
        tmp_ts.tv_sec = (systicks / HZ);
        tmp_ts.tv_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    #if !defined(CLOCK_NO_TASK)
    case CLOCK_PROCESS_CPUTIME_ID:  /* 本进程运行时间*/
        tmp_ts.tv_sec = task_current->elapsed_ticks / HZ;
        tmp_ts.tv_nsec = ((task_current->elapsed_ticks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_THREAD_CPUTIME_ID:   /*本线程运行时间*/
        tmp_ts.tv_sec = task_current->elapsed_ticks / HZ;
        tmp_ts.tv_nsec = ((task_current->elapsed_ticks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    #endif
    default:
        return -1;
    }
    return mem_copy_to_user(ts, &tmp_ts, sizeof(struct timespec));
}

int do_clock_settime(clockid_t clockid, struct timespec *ts)
{
    if (!ts)
        return -EFAULT;
    switch (clockid) {
    case CLOCK_REALTIME:        /* 系统统当前时间，从1970年1.1日算起 */
        {
            //walltime_printf();
            /* 计算秒值 */
            time_t sec = ts->tv_sec + ts->tv_nsec / (1000000 * 1000);
            int year, month, day;
            int hour, minute, second;
            /* FIXME: 时间转换算法有问题 */
            walltime_get_time(sec, &hour, &minute, &second);
            walltime_get_date(sec, &year, &month, &day);
            /* set to walltime */
            walltime.year = year;
            walltime.month = month;
            walltime.day = day;
            walltime.hour = hour;
            walltime.minute = minute;
            walltime.second = second;
            walltime.week_day = walltime_get_week_day(year, month, day);
            walltime.year_day = walltime_get_year_days();
        }
        //tmp_ts.tv_sec = walltime_make_timestamp(&walltime);
        //tmp_ts.tv_nsec = ((systicks % HZ) * MS_PER_TICKS) * 1000000;
        break;
    case CLOCK_MONOTONIC:       /* 修改系统的启动时间 */
        systicks = MSEC_TO_TICKS(ts->tv_sec * 1000 + ts->tv_nsec / 1000000); 
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

int sys_clock_settime(clockid_t clockid, const struct timespec *ts)
{
    if (!ts)
        return -EFAULT;
    struct timespec tmp_ts;
    if (mem_copy_from_user(&tmp_ts, (struct timespec *)ts, sizeof(struct timespec)) < 0)
        return -EFAULT;
    return do_clock_settime(clockid, &tmp_ts);
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
    unsigned long sec = ts->tv_sec;
    unsigned long nsec = ts->tv_nsec;
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
    ts->tv_sec = sec;
    ts->tv_nsec = nsec;
}
clock_t sys_times(struct tms *buf)
{
    if (buf) {
        task_t *cur = task_current;
        // keprint("systime:%d elapsed:%d\n", cur->syscall_ticks, cur->elapsed_ticks);
        struct tms _buf;
        _buf.tms_stime = cur->syscall_ticks;
        _buf.tms_utime = cur->elapsed_ticks;
        /* 统计子进程的时间 */
        clock_t cutime = 0, cstime = 0;
        task_t *child;
        list_for_each_owner (child, &task_global_list, list) {
            if (child->parent_pid == cur->pid) {
                cstime += child->syscall_ticks;
                cutime += child->elapsed_ticks;
            }
        }
        _buf.tms_cstime = cstime;
        _buf.tms_cutime = cutime;
        // keprint("%d %d %d %d\n", _buf.tms_stime, _buf.tms_utime, _buf.tms_cstime, _buf.tms_cutime);

        mem_copy_to_user(buf, &_buf, sizeof(struct tms));
    }
    return sys_get_ticks();
}