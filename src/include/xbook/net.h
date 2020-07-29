
#ifndef _XBOOK_NET_H
#define _XBOOK_NET_H

#include <lwip/sockets.h>

void init_net(void);


/* 套接字参数传递 */
struct _sockarg {
    void *buf;                  /* 缓冲区 */
    int len;                    /* 缓冲区长度 */
    unsigned int flags;         /* 标志 */
    struct sockaddr *to_from;   /* 传输目的地或者传输源 */
    int tolen;                  /* 套接字结构长度 */
    int *fromlen;               /* 来源套接字结构长度 */
};

struct _sockfd_set {
    fd_set *readfds;
    fd_set *writefds;
    fd_set *errorfds;
};

int sys_socket(int domain, int type, int protocol);
int sys_bind(int sockfd, struct sockaddr *my_addr, int addrlen);
int sys_connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
int sys_listen(int sockfd, int backlog);
int sys_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int sys_send(int sockfd, const void *buf, int len, int flags);
int sys_recv(int sockfd, void *buf, int len, unsigned int flags);
int sys_sendto(int sockfd, struct _sockarg *arg);
int sys_recvfrom(int sockfd, struct _sockarg *arg);
int sys_shutdown(int sockfd, int how);
int sys_getpeername(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen);
int sys_getsockname(int sockfd, struct sockaddr *my_addr, socklen_t *addrlen);
int sys_getsockopt(int sockfd, unsigned int flags, void *optval, socklen_t *optlen);
int sys_setsockopt(int sockfd, unsigned int flags, const void *optval, socklen_t optlen);
int sys_ioctlsocket(int sockfd, int request, void *arg);
int sys_select(int maxfdp, struct _sockfd_set *fd_sets, struct timeval *timeout);

#endif  /* _XBOOK_NET_H */
