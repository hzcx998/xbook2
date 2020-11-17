#include <xbook/net.h>
#include <xbook/debug.h>
#include <lwip/sockets.h>
#include <lwip/err.h>
#include <lwip/dns.h>

#include <fsal/fsal.h>
#include <fsal/fd.h>
#include <string.h>

// #define DEBUG_NETIF

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
    #ifdef DEBUG_NETIF
    printk("[NET]: %s: create socket id %d.\n", __func__, socket_id);
    #endif
    return local_fd_install(socket_id, FILE_FD_SOCKET);
}

int sys_bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    return lwip_bind(ffd->handle, my_addr, addrlen);
}
int sys_connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    return lwip_connect(ffd->handle, serv_addr, addrlen);
}

int sys_listen(int sockfd, int backlog)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    return lwip_listen(ffd->handle, backlog);
}

int sys_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    int new_socket = lwip_accept(ffd->handle, addr, addrlen);
    if (new_socket < 0)
        return -1;
    return local_fd_install(new_socket, FILE_FD_SOCKET);    /* 由于是接收一个客户端，需要安装新fd到文件描述表 */
}

int sys_send(int sockfd, const void *buf, int len, int flags)
{
#ifdef DEBUG_NETIF    
    printk("[NET]: %s: fd=%d buf=%x len=%d flags=%x\n", __func__, sockfd, buf, len, flags);
#endif
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    /* 使用内核缓冲区代替用户缓冲区，发送时涉及到任务切换 */
    void *tmpbuf = mem_alloc(len);
    if (tmpbuf == NULL)
        return -1;
    memcpy(tmpbuf, buf, len);
    int retval = lwip_send(ffd->handle, tmpbuf, len, flags);
    mem_free(tmpbuf);
    return retval;
}
int sys_recv(int sockfd, void *buf, int len, unsigned int flags)
{
#ifdef DEBUG_NETIF    
    printk("[NET]: %s: fd=%d buf=%x len=%d flags=%x\n", __func__, sockfd, buf, len, flags);
#endif    
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    /* 接收时是把接收的数据放到buf缓冲区，所以不用内核的缓冲区来存放 */
    return lwip_recv(ffd->handle, buf, len, flags);
}

int sys_sendto(int sockfd, struct _sockarg *arg)
{
    if (arg == NULL) {
        return -1;
    }
#ifdef DEBUG_NETIF    
    printk("[NET]: %s: fd=%d buf=%x len=%d flags=%x to=%x tolen=%d\n",
        __func__, sockfd, arg->buf, arg->len, arg->flags, arg->to_from, arg->tolen);
#endif    
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    /* 使用内核缓冲区代替用户缓冲区，发送时涉及到任务切换 */
    void *tmpbuf = mem_alloc(arg->len);
    if (tmpbuf == NULL)
        return -1;
    memcpy(tmpbuf, arg->buf, arg->len);
    int retval = lwip_sendto(ffd->handle, tmpbuf, arg->len, 
        arg->flags, arg->to_from, arg->tolen);
    mem_free(tmpbuf);
    return retval;
}

int sys_recvfrom(int sockfd, struct _sockarg *arg)
{
    if (arg == NULL) {
        return -1;
    }
#ifdef DEBUG_NETIF    
    printk("[NET]: %s: fd=%d buf=%x len=%d flags=%x to=%x fromlen=%d\n",
        __func__, sockfd, arg->buf, arg->len, arg->flags, arg->to_from, arg->tolen);
#endif    
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    /* 接收时是把接收的数据放到buf缓冲区，所以不用内核的缓冲区来存放 */
    return lwip_recvfrom(ffd->handle, arg->buf, arg->len, 
        arg->flags, arg->to_from, arg->fromlen);
}

int sys_shutdown(int sockfd, int how)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    return lwip_shutdown(ffd->handle, how);
}

int sys_getpeername(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    return lwip_getsockname(ffd->handle, serv_addr, addrlen);
}
int sys_getsockname(int sockfd, struct sockaddr *my_addr, socklen_t *addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    return lwip_getpeername(ffd->handle, my_addr, addrlen);
}

int sys_getsockopt(int sockfd, unsigned int flags, void *optval, socklen_t *optlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    int level = (int) ((flags >> 16) & 0xffff);
    int optname = (int) (flags & 0xffff);
    return lwip_getsockopt(ffd->handle, level, optname, optval, optlen);
}

int sys_setsockopt(int sockfd, unsigned int flags, const void *optval, socklen_t optlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    int level = (int) ((flags >> 16) & 0xffff);
    int optname = (int) (flags & 0xffff);
    return lwip_setsockopt(ffd->handle, level, optname, optval, optlen);
}

int sys_ioctlsocket(int sockfd, int request, void *arg)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
        return -1;
    if (!(ffd->flags & FILE_FD_SOCKET))
        return -1;

    return lwip_ioctl(ffd->handle, request, arg);
}

int sys_select(int maxfdp, struct _sockfd_set *fd_sets, struct timeval *timeout)
{
    if (fd_sets == NULL || maxfdp < 0 || maxfdp > LOCAL_FILE_OPEN_NR)
        return -1;
    
    fd_set readfds, writefds, errorfds;
    fd_set *preadfds = NULL, *pwritefds = NULL, *perrorfds = NULL;
    int i, maxsock = 0, fd;
    file_fd_t *ffd;

    if (fd_sets->readfds) { /* 有读集合 */
        for (i = 0; i < maxfdp; i++) { /* 遍历所有文件描述符，填写套接字集合 */
            if (FD_ISSET(i, fd_sets->readfds)) {
                ffd = fd_local_to_file(i); /* 将fd转换成socket */
                if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
                    continue;
                if (!(ffd->flags & FILE_FD_SOCKET))
                    continue;
                FD_SET(ffd->handle, &readfds); /* 设置套接字集合 */
                maxsock = max(maxsock, ffd->handle);
            }
        }
        preadfds = &readfds;
    }
    if (fd_sets->writefds) { /* 有写集合 */
        for (i = 0; i < maxfdp; i++) { /* 遍历所有文件描述符，填写套接字集合 */
            if (FD_ISSET(i, fd_sets->writefds)) {
                ffd = fd_local_to_file(i); /* 将fd转换成socket */
                if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
                    continue;
                if (!(ffd->flags & FILE_FD_SOCKET))
                    continue;
                FD_SET(ffd->handle, &writefds); /* 设置套接字集合 */
                maxsock = max(maxsock, ffd->handle);
            }
        }
        pwritefds = &writefds;
    }
    if (fd_sets->errorfds) { /* 有错误集合 */
        for (i = 0; i < maxfdp; i++) { /* 遍历所有文件描述符，填写套接字集合 */
            if (FD_ISSET(i, fd_sets->errorfds)) {
                ffd = fd_local_to_file(i); /* 将fd转换成socket */
                if (ffd == NULL || ffd->handle < 0 || ffd->flags == 0)
                    continue;
                if (!(ffd->flags & FILE_FD_SOCKET))
                    continue;
                FD_SET(ffd->handle, &errorfds); /* 设置套接字集合 */
                maxsock = max(maxsock, ffd->handle);
            }
        }
        perrorfds = &errorfds;
    }
    maxsock++;  /* 最大的socket值+1 */
    /* 返回就绪的套接字数量 */
    int nready = lwip_select(maxsock, preadfds, pwritefds, perrorfds, timeout);
    if (nready > 0) {   /* 有套接字就绪，回写到文件描述符集 */
        if (preadfds) { /* 有读集合 */
            /* 先将文件描述符集合清空 */
            FD_ZERO(fd_sets->readfds);
            for (i = 0; i < maxsock; i++) { /* 遍历所有套接字，填写文件描述符集 */
                if (FD_ISSET(i, preadfds)) { /* 有可读的套接字 */
                    /* 将套接字转换成文件描述符 */
                    fd = handle_to_local_fd(i, FILE_FD_SOCKET);
                    if (fd < 0)
                        continue;
                    FD_SET(fd, fd_sets->readfds); /* 设置套接字集合 */
                }
            }
        }
        if (pwritefds) { /* 有写集合 */
            /* 先将文件描述符集合清空 */
            FD_ZERO(fd_sets->writefds);
            for (i = 0; i < maxsock; i++) { /* 遍历所有套接字，填写文件描述符集 */
                if (FD_ISSET(i, pwritefds)) { /* 有可写的套接字 */
                    /* 将套接字转换成文件描述符 */
                    fd = handle_to_local_fd(i, FILE_FD_SOCKET);
                    if (fd < 0)
                        continue;
                    FD_SET(fd, fd_sets->writefds); /* 设置套接字集合 */
                }
            }
        }
        if (perrorfds) { /* 有写集合 */
            /* 先将文件描述符集合清空 */
            FD_ZERO(fd_sets->errorfds);
            for (i = 0; i < maxsock; i++) { /* 遍历所有套接字，填写文件描述符集 */
                if (FD_ISSET(i, perrorfds)) { /* 有可写的套接字 */
                    /* 将套接字转换成文件描述符 */
                    fd = handle_to_local_fd(i, FILE_FD_SOCKET);
                    if (fd < 0)
                        continue;
                    FD_SET(fd, fd_sets->errorfds); /* 设置套接字集合 */
                }
            }
        }
    }
    return nready;
}

int sys_dns_setserver(uint8_t numdns, const char *str)
{
    ip_addr_t dnsserver;
    ip4_addr_set_u32(&dnsserver, ipaddr_addr(str));
    dns_setserver((u8_t) numdns, &dnsserver);
    return 0;
}
