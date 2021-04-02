#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>

int gethostname(char *name, size_t len)
{
    if (!name)
        return -1;
    return syscall2(int, SYS_GETHOSTNAME, name, len);
}