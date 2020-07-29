#include <xbook/net.h>
#include <xbook/debug.h>
#include <lwip/sockets.h>
#include <fsal/fsal.h>

/**
 * 将套接字纳入用户的local fd table中，
 * 这样用户就可以通过read,write,close等来操作
 * socket套接字了。
 */
int sys_socket(int domain, int type, int protocol)
{
    //pr_dbg("%s: domain=%x type=%x protocol=%x\n", __func__, domain, type, protocol);
    int socket_id = lwip_socket(domain, type, protocol);
    if (socket_id < 0) {
        return -1;    
    }
    
    return local_fd_install(socket_id);
}

int sys_bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_bind(socket_id, my_addr, addrlen);
}
int sys_connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_connect(socket_id, serv_addr, addrlen);
}

int sys_listen(int sockfd, int backlog)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_listen(socket_id, backlog);
}

int sys_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    int new_socket = lwip_accept(socket_id, addr, addrlen);
    if (new_socket < 0)
        return -1;
    return local_fd_install(new_socket);    /* 由于是接收一个客户端，需要安装新fd到文件描述表 */
}

int sys_send(int sockfd, const void *buf, int len, int flags)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_send(socket_id, buf, len, flags);
}
int sys_recv(int sockfd, void *buf, int len, unsigned int flags)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_recv(socket_id, buf, len, flags);
}

int sys_sendto(int sockfd, struct _sockarg *arg)
{
    if (arg == NULL) {
        return -1;
    }
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_sendto(socket_id, arg->buf, arg->len, 
        arg->flags, arg->to_from, (socklen_t) arg->tolen);
}

int sys_recvfrom(int sockfd, struct _sockarg *arg)
{
    if (arg == NULL) {
        return -1;
    }
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_recvfrom(socket_id, arg->buf, arg->len, 
        arg->flags, arg->to_from, (socklen_t *) arg->fromlen);
}

int sys_shutdown(int sockfd, int how)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_shutdown(socket_id, how);
}

int sys_getpeername(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_getsockname(socket_id, serv_addr, addrlen);
}
int sys_getsockname(int sockfd, struct sockaddr *my_addr, socklen_t *addrlen)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_getpeername(socket_id, my_addr, addrlen);
}

int sys_getsockopt(int sockfd, unsigned int flags, void *optval, socklen_t *optlen)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    int level = (int) ((flags >> 16) & 0xffff);
    int optname = (int) (flags & 0xffff);
    return lwip_getsockopt(socket_id, level, optname, optval, optlen);
}

int sys_setsockopt(int sockfd, unsigned int flags, const void *optval, socklen_t optlen)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    int level = (int) ((flags >> 16) & 0xffff);
    int optname = (int) (flags & 0xffff);
    return lwip_setsockopt(socket_id, level, optname, optval, optlen);
}

int sys_ioctlsocket(int sockfd, int request, void *arg)
{
    int socket_id = fd_local_to_global(sockfd);
    if (socket_id < 0)
        return -1;
    return lwip_ioctl(socket_id, request, arg);
}

int sys_select(int maxfdp, struct _sockfd_set *fd_sets, struct timeval *timeout)
{
    if (fd_sets == NULL)
        return -1;
    /* TODO: 需要将文件描述符转换成socket套接字 */
    return lwip_select(maxfdp, fd_sets->readfds, fd_sets->writefds, fd_sets->errorfds, timeout);
}
