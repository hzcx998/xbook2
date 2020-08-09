#include <sys/syscall.h>
#include <xcons.h>

int xcons_clear()
{
    syscall0(int, SYS_XCONCLEAR);
    return 0;
}

int xcons_getkey(int *buf, int flags)
{
    return syscall2(int, SYS_XCONGET, buf, flags);
}

int xcons_putstr(void *buf, int len)
{
    return syscall2(int, SYS_XCONPUT, buf, len);
}
