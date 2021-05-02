#ifndef _XBOOK_NET_H
#define _XBOOK_NET_H

#include <sys/lpc.h>

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
    NETCALL_LAST_CALL,
};


int netif_close(int sock);
int netif_incref(int sock);
int netif_decref(int sock);

#endif  /* _XBOOK_NET_H */