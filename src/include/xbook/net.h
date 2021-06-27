#ifndef _XBOOK_NET_H
#define _XBOOK_NET_H

#include <sys/lpc.h>
#include <sys/socket.h>

#define HOSTNAME_MAX_LEN 65
#define HOSTNAME_DEFAULT "xbook"

enum net_client_code {
    NETCALL_socket = FIRST_CALL_CODE,
    NETCALL_bind,
    NETCALL_connect,
    NETCALL_listen,
    NETCALL_accept,
    NETCALL_send,
    NETCALL_recv,
    NETCALL_close,
    NETCALL_sendto,
    NETCALL_recvfrom,
    NETCALL_ioctl,
    NETCALL_shutdown,
    NETCALL_getpeername,
    NETCALL_getsockname,
    NETCALL_getsockopt,
    NETCALL_setsockopt,
    NETCALL_read,
    NETCALL_write,
    NETCALL_fcntl,
    NETCALL_LAST_CALL,
};


int netif_close(int sock);
int netif_incref(int sock);
int netif_decref(int sock);
int netif_read(int sock, void *buffer, size_t nbytes);
int netif_write(int sock, void *buffer, size_t nbytes);
int netif_ioctl(int sock, int request, void *arg);
int netif_fcntl(int sock, int cmd, long val);
int do_socket_close(int sock);

int sys_sethostname(const char *name, size_t len);
int sys_gethostname(char *name, size_t len);
void net_init();

#endif  /* _XBOOK_NET_H */