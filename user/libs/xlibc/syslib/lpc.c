#include <sys/syscall.h>
#include <sys/lpc.h>
#include <string.h>

int create_port(char *name, uint32_t max_connects, uint32_t max_msgsz)
{
    return syscall3(int, SYS_CREATE_PORT, name, max_connects, max_msgsz);
}

int close_port(int phandle)
{
    return syscall1(int, SYS_CLOSE_PORT, phandle);
}

int accept_port(int phandle, int isaccept)
{
    return syscall2(int, SYS_ACCEPT_PORT, phandle, isaccept);
}

int connect_port(char *name, uint32_t *max_msgsz)
{
    return syscall2(int, SYS_CONNECT_PORT, name, max_msgsz);
}

int reply_port(int phandle, lpc_message_t *lpc_msg)
{
    return syscall2(int, SYS_REPLY_PORT, phandle, lpc_msg);
}

int receive_port(int phandle, lpc_message_t *lpc_msg)
{
    return syscall2(int, SYS_RECEIVE_PORT, phandle, lpc_msg);
}

int request_port(int phandle, lpc_message_t *lpc_msg)
{
    return syscall2(int, SYS_REQUEST_PORT, phandle, lpc_msg);
}

void lpc_reset_message(lpc_message_t *msg)
{
    memset(msg, 0, sizeof(lpc_message_t));    
}
