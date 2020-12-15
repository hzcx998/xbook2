#include <sys/syscall.h>
#include <sys/servcall.h>
#include <string.h>

int bind_port(int port)
{
    return syscall1(int, SYS_BIND_PORT, port);
}

int unbind_port(int port)
{
    return syscall1(int, SYS_UNBIND_PORT, port);
}

int reply_port(int port, servmsg_t *msg)
{
    return syscall2(int, SYS_REPLY_PORT, port, msg);
}

int receive_port(int port, servmsg_t *msg)
{
    return syscall2(int, SYS_RECEIVE_PORT, port, msg);
}

int request_port(int port, servmsg_t *msg)
{
    return syscall2(int, SYS_REQUEST_PORT, port, msg);
}

void servmsg_reset(servmsg_t *msg)
{
    memset(msg, 0, sizeof(servmsg_t));    
}
