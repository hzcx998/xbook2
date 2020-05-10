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
int srvcall_listen(int port, srvcall_arg_t *arg)
{
    return syscall2(int, SYS_SRVCALL_LISTEN, port, arg);
}

int srvcall_ack(int port, srvcall_arg_t *arg)
{
    return syscall2(int, SYS_SRVCALL_ACK, port, arg);
}

int srvcall(int port, srvcall_arg_t *arg)
{
    return syscall2(int, SYS_SRVCALL, port, arg);
}
