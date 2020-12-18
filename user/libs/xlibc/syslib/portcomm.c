#include <sys/syscall.h>
#include <sys/portcomm.h>
#include <string.h>

int bind_port(int port)
{
    return syscall1(int, SYS_BIND_PORT, port);
}

int unbind_port(int port)
{
    return syscall1(int, SYS_UNBIND_PORT, port);
}

int reply_port(int port, port_msg_t *msg)
{
    return syscall2(int, SYS_REPLY_PORT, port, msg);
}

int receive_port(int port, port_msg_t *msg)
{
    return syscall2(int, SYS_RECEIVE_PORT, port, msg);
}

int request_port(int port, port_msg_t *msg)
{
    return syscall2(int, SYS_REQUEST_PORT, port, msg);
}

void port_msg_reset(port_msg_t *msg)
{
    memset(msg, 0, sizeof(port_msg_t));    
}

void port_msg_copy_header(port_msg_t *src, port_msg_t *dest)
{
    memcpy(&dest->header, &src->header, sizeof(port_msg_header_t));
}
