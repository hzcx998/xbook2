#ifndef _SYS_SOCKCALL_H
#define _SYS_SOCKCALL_H

#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    struct {
        int sock;
        struct sockaddr *serv_addr;
        int addrlen;
    } connect;
    struct {
        int sock;
        struct sockaddr *addr;
        socklen_t *addrlen;
    } accept;
    struct {
        int sock;
        int backlog;
    } listen;
    struct {
        int sock;
        void *buf;
        int len;
        int flags;
    } recv;
    struct {
        int sock;
        void *buf;
        int len;
        unsigned int flags;
        struct sockaddr *from;
        socklen_t *fromlen;
    } recvfrom;
    struct {
        int sock;
        const void *buf;
        int len;
        int flags;
    } send;
    struct {
        int sock;
        const void *buf;
        int len;
        unsigned int flags;
        const struct sockaddr *to;
        socklen_t tolen;
    } sendto;
    struct {
        int sock;
        int how;
    } shutdown;
    struct {
        int sock;
        struct sockaddr *serv_addr;
        socklen_t *addrlen;
    } getpeername;
    struct {
        int sock;
        struct sockaddr *my_addr;
        socklen_t *addrlen;
    } getsockname;
    struct {
        int sock;
        int level;
        int optname;
        void *optval;
        socklen_t *optlen;
    } getsockopt;
    struct {
        int sock;
        int level;
        int optname;
        const void *optval;
        socklen_t optlen;
    } setsockopt;
} sock_param_t;

int sockcall(int sockop, sock_param_t *param);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_SOCKCALL_H */
