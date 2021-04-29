#ifndef _XBOOK_SOCKCALL_H
#define _XBOOK_SOCKCALL_H

#include <types.h>
#include <sys/socket.h>

#define SOCKOP_socket  1
#define SOCKOP_bind  2
#define SOCKOP_connect  3
#define SOCKOP_listen  4
#define SOCKOP_accept  5
#define SOCKOP_getsockname 6
#define SOCKOP_getpeername 7
#define SOCKOP_socketpair 8
#define SOCKOP_send  9
#define SOCKOP_recv  10
#define SOCKOP_sendto  11
#define SOCKOP_recvfrom  12
#define SOCKOP_shutdown  13
#define SOCKOP_setsockopt 14
#define SOCKOP_getsockopt 15
#define SOCKOP_sendmsg  16
#define SOCKOP_recvmsg  17

typedef union {
    struct {
        int domain;
        int type;
        int protocol;
    } socket;
    struct {
        int sock;
        struct sockaddr *my_addr;
        int addrlen;
    } bind;
} sock_param_t;

int sys_sockcall(int sockop, sock_param_t *param);

#endif  /* _XBOOK_SOCKCALL_H */