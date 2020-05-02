#include <sys/time.h>
#include <sys/syscall.h>

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
