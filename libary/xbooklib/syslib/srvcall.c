#include <sys/syscall.h>
#include <sys/srvcall.h>

int srvcall_bind(int port)
{
    return syscall1(int, SYS_SRVCALL_BIND, port);
}

int srvcall_unbind(int port)
{
    return syscall1(int, SYS_SRVCALL_UNBIND, port);
}
int srvcall_listen(int port, srvarg_t *arg)
{
    return syscall2(int, SYS_SRVCALL_LISTEN, port, arg);
}

int srvcall_ack(int port, srvarg_t *arg)
{
    return syscall2(int, SYS_SRVCALL_ACK, port, arg);
}

int srvcall(int port, srvarg_t *arg)
{
    return syscall2(int, SYS_SRVCALL, port, arg);
}

int srvcall_fetch(int port, srvarg_t *arg)
{
    return syscall2(int, SYS_SRVCALL_FETCH, port, arg);
}

/**
 * srvcall_check - 检测参数是否有缓冲区
 * 
 */
int srvcall_check(srvarg_t *arg)
{
    int i;
    for (i = 0; i < SRVARG_NR; i++) {
        if (arg->size[i] > 0) { /* 非0长度 */
            if (!(arg->io & (1 << i))) {   /* SRVIO_SERVICE */
                return 0;
            }
        }
    }
    return -1;
}
