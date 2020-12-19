#ifndef _CLIENT_NETSOCKET_H
#define _CLIENT_NETSOCKET_H

#include <sys/lpc.h>

enum net_client_code {
    NETCALL_socket = FIRST_CALL_CODE,
    NETCALL_bind,
    NETCALL_connect,
    NETCALL_listen,
    NETCALL_accept,
    NETCALL_send,
    NETCALL_recv,
    //NETCALL_sendto,
    //NETCALL_recvfrom,
    NETCALL_close,
    NETCALL_ioctl,
    NETCALL_LAST_CALL,
};

#endif  /* _CLIENT_NETSOCKET_H */