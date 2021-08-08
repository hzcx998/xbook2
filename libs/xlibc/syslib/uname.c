#include <sys/syscall.h>
#include <sys/utsname.h>

int uname(struct utsname *buf)
{
    return syscall1(int, SYS_UNAME, buf);
}