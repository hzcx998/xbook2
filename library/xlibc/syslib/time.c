#include <sys/time.h>
#include <sys/syscall.h>
#include <time.h>
#include <errno.h>
/**
 * alarm - 设置一个闹钟
 * @second: 闹钟产生的时间
 * 
 * @return: return left second before alarm
 */
unsigned long alarm(unsigned long second)
{
    return syscall1(unsigned long , SYS_ALARM, second);
}

/**
 * ktime - 获取内核时间
 * @ktm: 内核时间结构
 */
unsigned long ktime(ktime_t *ktm)
{
    return syscall1(int, SYS_KTIME, ktm);
}


/**
 * getticks - 获取内核运行时钟数
 */
clock_t getticks()
{
    return syscall0(clock_t, SYS_GETTICKS);
}

/**
 * gettimeofday - 获取当前的时间
 * @tv: 时间
 * @tz: 时区
 * 
 * 成功返回0，失败返回-1
 */
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    return syscall2(int, SYS_GETTIMEOFDAY, tv, tz);
}
/**
 * clock_gettime - 获取时机
 * @clockid: 获取的类型
 * @ts: 时间结构
 * 
 * 成功返回0，失败返回-1
 */
int clock_gettime(clockid_t clockid, struct timespec *ts)
{
    return syscall2(int, SYS_CLOCK_GETTIME, clockid, ts);
}

int ktimeto(ktime_t *ktm, struct tm *tm)
{
    if (!ktm || !tm)
        return -1;
    tm->tm_year = ktm->year - 1900;
    tm->tm_yday = ktm->year_day;
    tm->tm_mon = ktm->month;
    tm->tm_mday = ktm->day;
    tm->tm_hour = ktm->hour;
    tm->tm_min = ktm->minute;
    tm->tm_sec = ktm->second;
    tm->tm_wday = ktm->week_day;
    tm->tm_isdst = -1;
    return 0;
}

/**
 * usleep - 以微妙为单位休眠
 * @usec: 要休眠的时间（微秒）
 * 
 * 休眠过程中被打断，那么剩余的休眠时间（微秒）将被返回
 * 
 * 成功返回0，失败返回-1
 */
int usleep(useconds_t usec)
{
    struct timeval tvin, tvout;
    tvin.tv_sec = usec / 1000000L;
    tvin.tv_usec = usec % 1000000L;
    
    long retv = syscall2(int, SYS_USLEEP, &tvin, &tvout);
    if (!retv)
        return 0;

    _set_errno(retv);
    return -1;
}


/**
 * 根据ticks延迟
 */
void mdelay(time_t msec)
{
    clock_t ticks = MSEC_TO_TICKS(msec);
    /* at least one ticket */
    if (!ticks)
        ticks = 1;

    clock_t start = getticks();
    while (getticks() - start < ticks) {}
}
