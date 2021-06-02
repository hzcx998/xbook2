#include <sys/select.h>
#include <sys/syscall.h>
#include <errno.h>

int select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout)
{
    int ret = syscall5(int, SYS_SELECT, maxfdp, readfds, writefds, exceptfds, timeout);
    if (ret < 0) {
        _set_errno(-ret);
        ret = -1;
    }
    return ret;
}
