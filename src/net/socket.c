#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>

int sys_socket(int domain, int type, int protocol)
{
    dbgprint("call sys_socket: domain=%d type=%d protocol=%d\n", domain, type, protocol);
    /* make request to net service */
    
    /* install sock into fd */
    
    /* return fd */
    return -ENOSYS;
}