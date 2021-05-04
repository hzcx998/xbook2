#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <types.h>

struct sockaddr {
  u8_t sa_len;
  u8_t sa_family;
  char sa_data[14];
};

int sys_socket(int domain, int type, int protocol);
int sys_socket_bind(int sock, struct sockaddr *my_addr, int addrlen);
int sys_socket_accept(int sock, struct sockaddr *addr, socklen_t *addrlen);
int sys_socket_listen(int sock, int backlog);
int sys_socket_connect(int sock, struct sockaddr *serv_addr, int addrlen);

int sys_socket_recv(int sock, void *buf, int len, int flags);
int sys_socket_recvfrom(int sock, void *buf, int len, unsigned int flags,
    struct sockaddr *from, socklen_t *fromlen);
int sys_socket_send(int sock, const void *buf, int len, int flags);
int sys_socket_sendto(int sock, const void *buf, int len, unsigned int flags,
    const struct sockaddr *to, socklen_t tolen);
int sys_socket_shutdown(int sock, int how);

int sys_socket_getpeername(int sock, struct sockaddr *serv_addr, socklen_t *addrlen);
int sys_socket_getsockname(int sock, struct sockaddr *my_addr, socklen_t *addrlen);
int sys_socket_getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);
int sys_socket_setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);

#endif   /* _SYS_SOCKET_H */

