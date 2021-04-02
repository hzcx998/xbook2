#include <sys/times.h>
#include <sys/syscall.h>

clock_t times(struct tms *buf)
{
    return syscall1(clock_t, SYS_TIMES, buf);
}
