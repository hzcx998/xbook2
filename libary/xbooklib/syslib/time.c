#include <sys/time.h>
#include <sys/syscall.h>
#include <time.h>

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
 * clock - 获取内核运行时钟数
 */
clock_t clock()
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
