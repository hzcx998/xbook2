#ifndef _NETSOCKET_H
#define _NETSOCKET_H

#include <sys/socket.h>

int net_socket(int domain, int type, int protocol);
int net_bind(int sock, struct sockaddr *my_addr, int addrlen);
int net_connect(int sock, struct sockaddr *serv_addr, int addrlen);
int net_listen(int sock, int backlog);
int net_accept(int sock, struct sockaddr *addr, socklen_t *addrlen);
int net_send(int sock, const void *buf, int len, int flags);
int net_recv(int sock, void *buf, int len, int flags);
int net_sendto(int sock, const void *buf, int len, unsigned int flags,
    const struct sockaddr *to, socklen_t tolen);
int net_recvfrom(int sock, void *buf, int len, unsigned int flags,
    struct sockaddr *from, socklen_t *fromlen);
int net_close(int sock);
int net_ioctl(int sock, int request, void *arg);
int net_shutdown(int sock, int how);
int net_getpeername(int sock, struct sockaddr *serv_addr, socklen_t *addrlen);
int net_getsockname(int sock, struct sockaddr *my_addr, socklen_t *addrlen);
int net_getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);
int net_setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);

#endif  /* _NETSOCKET_H */