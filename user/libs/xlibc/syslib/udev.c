#include <sys/udev.h>
#include <sys/syscall.h>

int scandev(devent_t *de, device_type_t type, devent_t *out_de)
{
    return syscall3(int, SYS_SCANDEV, de, type, out_de);
}

int fastio(int fd, int cmd, void *arg)
{
    return syscall3(int, SYS_FASTIO, fd, cmd, arg);
}

int fastread(int fd, void *buf, size_t size)
{
    return syscall3(int, SYS_FASTREAD, fd, buf, size);
}

int fastwrite(int fd, void *buf, size_t size)
{
    return syscall3(int, SYS_FASTWRITE, fd, buf, size);
}
