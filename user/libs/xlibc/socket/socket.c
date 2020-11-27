#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/syscall.h>

int socket(int domain, int type, int protocol)
{
    return syscall3(int, SYS_SOCKET, domain, type, protocol);
}

int bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    return syscall3(int, SYS_BIND, sockfd, my_addr, addrlen);
}

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    return syscall3(int, SYS_CONNECT, sockfd, serv_addr, addrlen);
}

int listen(int sockfd, int backlog)
{
    return syscall2(int, SYS_LISTEN, sockfd, backlog);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    return syscall3(int, SYS_ACCEPT, sockfd, addr, addrlen);
}

int send(int sockfd, const void *buf, int len, int flags)
{
    return syscall4(int, SYS_SEND, sockfd, buf, len, flags);
}

int recv(int sockfd, void *buf, int len, unsigned int flags)
{
    return syscall4(int, SYS_RECV, sockfd, buf, len, flags);
}

int sendto(int sockfd, const void *buf, int len, unsigned int flags,
    const struct sockaddr *to, socklen_t tolen)
{
    struct _sockarg arg;
    arg.buf = (void *)buf;
    arg.len = len;
    arg.flags = flags;
    arg.to_from = (struct sockaddr *) to;
    arg.tolen = tolen;
    return syscall2(int, SYS_SENDTO, sockfd, &arg);
}

int recvfrom(int sockfd, void *buf, int len, unsigned int flags,
    struct sockaddr *from, socklen_t *fromlen)
{
    struct _sockarg arg;
    arg.buf = (void *)buf;
    arg.len = len;
    arg.flags = flags;
    arg.to_from = (struct sockaddr *) from;
    arg.fromlen = fromlen;
    return syscall2(int, SYS_RECVFROM, sockfd, &arg);
}


int shutdown(int sockfd, int how)
{
    return syscall2(int, SYS_SHUTDOWN, sockfd, how);
}

int getpeername(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen)
{
    return syscall3(int, SYS_GETPEERNAME, sockfd, serv_addr, addrlen);
}

int getsockname(int sockfd, struct sockaddr *my_addr, socklen_t *addrlen)
{
    return syscall3(int, SYS_GETSOCKNAME, sockfd, my_addr, addrlen);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    return syscall4(int, SYS_GETSOCKOPT, sockfd, ((level & 0xffff) << 16) | (optname & 0xffff),
        optval, optlen);
}
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    return syscall4(int, SYS_SETSOCKOPT, sockfd, ((level & 0xffff) << 16) | (optname & 0xffff),
        optval, optlen);
}

int ioctlsocket(int sockfd, int request, void *arg)
{
    return syscall3(int, SYS_IOCTLSOCKET, sockfd, request, arg);
}

int select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptset, struct timeval *timeout)
{
    struct _sockfd_set fd_sets;
    fd_sets.readfds = readfds;
    fd_sets.writefds = writefds;
    fd_sets.errorfds = exceptset;
    return syscall3(int, SYS_SELECT, maxfdp, &fd_sets, timeout);
}
