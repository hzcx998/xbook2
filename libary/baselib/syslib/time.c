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
