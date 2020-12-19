#ifndef _CLIENT_NETSOCKET_H
#define _CLIENT_NETSOCKET_H

#include <sys/lpc.h>

enum net_client_code {
    NETCALL_socket = FIRST_CALL_CODE,
    NETCALL_LAST_CALL,
};

#endif  /* _CLIENT_NETSOCKET_H */