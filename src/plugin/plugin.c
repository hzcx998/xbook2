#include <xbook/plugin.h>
#include <plugin/net.h>
#include <xbook/syscall.h>

void plugin_init(void)
{
    #ifdef PLIGIN_NETWORK
    network_init();
    #endif
}

#ifdef PLIGIN_NETWORK
static void plugin_net_syscall_init()
{   
    syscalls[SYS_SOCKET] = sys_socket;
    syscalls[SYS_BIND] = sys_bind;
    syscalls[SYS_CONNECT] = sys_connect;
    syscalls[SYS_LISTEN] = sys_listen;
    syscalls[SYS_ACCEPT] = sys_accept;
    syscalls[SYS_SEND] = sys_send;
    syscalls[SYS_RECV] = sys_recv;
    syscalls[SYS_SENDTO] = sys_sendto;
    syscalls[SYS_RECVFROM] = sys_recvfrom;
    syscalls[SYS_SHUTDOWN] = sys_shutdown;
    syscalls[SYS_GETPEERNAME] = sys_getpeername;
    syscalls[SYS_GETSOCKNAME] = sys_getsockname;
    syscalls[SYS_GETSOCKOPT] = sys_getsockopt;
    syscalls[SYS_SETSOCKOPT] = sys_setsockopt;
    syscalls[SYS_IOCTLSOCKET] = sys_ioctlsocket;
    syscalls[SYS_SELECT] = sys_select;
}
#endif /* PLIGIN_NETWORK */

void plugin_syscall_init()
{   
    #ifdef PLIGIN_NETWORK
    plugin_net_syscall_init();
    #endif
}
