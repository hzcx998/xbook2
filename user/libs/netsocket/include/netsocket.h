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

#endif  /* _NETSOCKET_H */