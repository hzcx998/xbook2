#include <unistd.h>
#include <sys/syscall.h>

long sysconf(int name)
{
    return syscall1(long, SYS_SYSCONF, name);
}
