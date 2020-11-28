#include <plugin/net.h>
#include <xbook/debug.h>
#include <lwip/sockets.h>
#include <lwip/err.h>
#include <lwip/dns.h>
#include <xbook/fsal.h>
#include <xbook/fd.h>
#include <xbook/safety.h>
#include <string.h>
#include <errno.h>
#include <string.h>

int sys_socket(int domain, int type, int protocol)
{
    int socket_id = lwip_socket(domain, type, protocol);
    if (socket_id < 0) {
        return -EPERM;    
    }
    return local_fd_install(socket_id, FILE_FD_SOCKET);
}

int sys_bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    if (mem_copy_from_user(NULL, my_addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    return lwip_bind(ffd->handle, my_addr, addrlen);
}

int sys_connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    if (mem_copy_from_user(NULL, serv_addr, sizeof(struct sockaddr)) < 0)
        return -EFAULT;
    return lwip_connect(ffd->handle, serv_addr, addrlen);
}

int sys_listen(int sockfd, int backlog)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    return lwip_listen(ffd->handle, backlog);
}

int sys_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    struct sockaddr _addr;
    socklen_t _addrlen;
    int new_socket = lwip_accept(ffd->handle, &_addr, &_addrlen);
    if (new_socket < 0)
        return -EPERM;
    if (addr)
        if (mem_copy_to_user(addr, &_addr, sizeof(struct sockaddr)) < 0)
            return -EFAULT;
    if (addrlen)
        if (mem_copy_to_user(addrlen, &_addrlen, sizeof(socklen_t)) < 0)
            return -EFAULT;
    /* 由于是接收一个客户端，需要安装新fd到文件描述表 */
    return local_fd_install(new_socket, FILE_FD_SOCKET);
}

int sys_send(int sockfd, const void *buf, int len, int flags)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    void *tmpbuf = mem_alloc(len);
    if (tmpbuf == NULL)
        return -ENOMEM;
    if (mem_copy_from_user(tmpbuf, (void *)buf, len) < 0) {
        mem_free(tmpbuf);
        return -EFAULT;
    }
    int retval = lwip_send(ffd->handle, tmpbuf, len, flags);
    mem_free(tmpbuf);
    return retval;
}

int sys_recv(int sockfd, void *buf, int len, unsigned int flags)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    void *tmpbuf = mem_alloc(len);
    if (tmpbuf == NULL)
        return -ENOMEM;
    int retval = lwip_recv(ffd->handle, tmpbuf, len, flags);
    if (mem_copy_to_user(buf, tmpbuf, len) < 0) {
        mem_free(tmpbuf);
        return -EFAULT;
    }
    mem_free(tmpbuf);
    return retval;
}

int sys_sendto(int sockfd, struct _sockarg *arg)
{
    if (arg == NULL) {
        return -EINVAL;
    }
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    void *tmpbuf = mem_alloc(arg->len);
    if (tmpbuf == NULL)
        return -ENOMEM;
    if (mem_copy_from_user(tmpbuf, arg->buf, arg->len) < 0) {
        mem_free(tmpbuf);
        return -EFAULT;
    }
    int retval = lwip_sendto(ffd->handle, tmpbuf, arg->len, 
        arg->flags, arg->to_from, arg->tolen);
    mem_free(tmpbuf);
    return retval;
}

int sys_recvfrom(int sockfd, struct _sockarg *arg)
{
    if (arg == NULL) {
        return -EINVAL;
    }
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    void *tmpbuf = mem_alloc(arg->len);
    if (tmpbuf == NULL)
        return -ENOMEM;
    struct sockaddr from;
    socklen_t fromlen;
    int retval = lwip_recvfrom(ffd->handle, tmpbuf, arg->len, 
            arg->flags, &from, &fromlen);
    if (mem_copy_to_user(arg->buf, tmpbuf, arg->len) < 0) {
        mem_free(tmpbuf);
        return -EFAULT;
    }
    if (arg->to_from) {
        if (mem_copy_to_user(arg->to_from, &from, sizeof(struct sockaddr)) < 0) {
            mem_free(tmpbuf);
            return -EFAULT;
        }
    }
    if (arg->fromlen) {
        if (mem_copy_to_user(arg->fromlen, &fromlen, sizeof(socklen_t)) < 0) {
            mem_free(tmpbuf);
            return -EFAULT;
        }
    }
    mem_free(tmpbuf);
    return retval;
}

int sys_shutdown(int sockfd, int how)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    return lwip_shutdown(ffd->handle, how);
}

int sys_getpeername(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    struct sockaddr _serv_addr; 
    socklen_t _addrlen;
    if (lwip_getsockname(ffd->handle, &_serv_addr, &_addrlen) < 0)
        return -1;
    if (serv_addr) {
        if (mem_copy_to_user(serv_addr, &_serv_addr, sizeof(struct sockaddr)) < 0) {
            return -EFAULT;
        }
    }
    if (addrlen) {
        if (mem_copy_to_user(addrlen, &_addrlen, sizeof(socklen_t)) < 0) {
            return -EFAULT;
        }
    }
    return 0;
}

int sys_getsockname(int sockfd, struct sockaddr *my_addr, socklen_t *addrlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    struct sockaddr _my_addr; 
    socklen_t _addrlen;
    if (lwip_getpeername(ffd->handle, &_my_addr, &_addrlen) < 0)
        return -1;
    if (my_addr) {
        if (mem_copy_to_user(my_addr, &_my_addr, sizeof(struct sockaddr)) < 0) {
            return -EFAULT;
        }
    }
    if (addrlen) {
        if (mem_copy_to_user(addrlen, &_addrlen, sizeof(socklen_t)) < 0) {
            return -EFAULT;
        }
    }
    return 0;
}

int sys_getsockopt(int sockfd, unsigned int flags, void *optval, socklen_t *optlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    int level = (int) ((flags >> 16) & 0xffff);
    int optname = (int) (flags & 0xffff);
    unsigned long _optval;
    socklen_t _optlen;
    if (lwip_getsockopt(ffd->handle, level, optname, (void *)&_optval, &_optlen) < 0)
        return -1;
    if (optval) {
        if (mem_copy_to_user(optval, (void *)&_optval, sizeof(void *)) < 0) {
            return -EFAULT;
        }
    }
    if (optlen) {
        if (mem_copy_to_user(optlen, &_optlen, sizeof(socklen_t)) < 0) {
            return -EFAULT;
        }
    }
    return 0;
}

int sys_setsockopt(int sockfd, unsigned int flags, const void *optval, socklen_t optlen)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    if (!optval)
        return -EINVAL;
    int level = (int) ((flags >> 16) & 0xffff);
    int optname = (int) (flags & 0xffff);
    if (mem_copy_from_user(NULL, (void *)optval, sizeof(void *)) < 0) {
        return -EFAULT;
    }
    return lwip_setsockopt(ffd->handle, level, optname, optval, optlen);
}

int sys_ioctlsocket(int sockfd, int request, void *arg)
{
    file_fd_t *ffd = fd_local_to_file(sockfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
        return -EINVAL;
    unsigned long _arg;
    if (mem_copy_from_user((void *)&_arg, arg, sizeof(void *)) < 0) {
        return -EFAULT;
    }
    return lwip_ioctl(ffd->handle, request, (void *)&_arg);
}

int sys_select(int maxfdp, struct _sockfd_set *fd_sets, struct timeval *timeout)
{
    if (fd_sets == NULL || maxfdp < 0 || maxfdp > LOCAL_FILE_OPEN_NR)
        return -EINVAL;
    /* TODO: use safety copy fd sets from user and copy to user */
    fd_set readfds, writefds, errorfds;
    fd_set *preadfds = NULL, *pwritefds = NULL, *perrorfds = NULL;
    int i, maxsock = 0, fd;
    file_fd_t *ffd;

    if (fd_sets->readfds) { /* 有读集合 */
        for (i = 0; i < maxfdp; i++) { /* 遍历所有文件描述符，填写套接字集合 */
            if (FD_ISSET(i, fd_sets->readfds)) {
                ffd = fd_local_to_file(i); /* 将fd转换成socket */
                if (FILE_FD_IS_BAD(ffd))
                    continue;
                if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
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
                if (FILE_FD_IS_BAD(ffd))
                    continue;
                if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
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
                if (FILE_FD_IS_BAD(ffd))
                    continue;
                if (INVALID_FD_TYPE(ffd, FILE_FD_SOCKET))
                    continue;
                FD_SET(ffd->handle, &errorfds); /* 设置套接字集合 */
                maxsock = max(maxsock, ffd->handle);
            }
        }
        perrorfds = &errorfds;
    }
    maxsock++;  /* 最大的socket值+1 */
    struct timeval _tv;
    if (timeout) {
        if (mem_copy_from_user(&_tv, timeout, sizeof(struct timeval)) < 0) {
            return -EFAULT;
        }
    }
    /* 返回就绪的套接字数量 */
    int nready = lwip_select(maxsock, preadfds, pwritefds, perrorfds, timeout == NULL ? NULL : &_tv);
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
    if (!str)
        return -EINVAL;
    ip_addr_t dnsserver;
    char buf[32] = {0};
    if (mem_copy_from_user(buf, (void *)str, strlen(str)) < 0)
        return -EINVAL;
    ip4_addr_set_u32(&dnsserver, ipaddr_addr(buf));
    dns_setserver((u8_t) numdns, &dnsserver);
    return 0;
}

static int netif_read(int sock, void *buffer, size_t nbytes)
{
    if (sock < 0 || !nbytes || !buffer)
        return -EINVAL;
    void *tmpbuffer = mem_alloc(nbytes);
    if (tmpbuffer == NULL)
        return -ENOMEM;
    int rdbytes = lwip_read(sock, tmpbuffer, nbytes);  
    if (mem_copy_to_user(buffer, tmpbuffer, nbytes) < 0) {
        mem_free(tmpbuffer);
        return -EFAULT;
    }
    mem_free(tmpbuffer);
    return rdbytes;
}

static int netif_write(int sock, void *buffer, size_t nbytes)
{
    if (sock < 0 || !nbytes || !buffer)
        return -EINVAL;
    if (mem_copy_from_user(NULL, buffer, nbytes) < 0)
        return -EINVAL;
    void *tmpbuffer = mem_alloc(nbytes);
    if (tmpbuffer == NULL)
        return -ENOMEM;
    if (mem_copy_from_user(tmpbuffer, buffer, nbytes)) {
        mem_free(tmpbuffer);
        return -EFAULT;    
    }
    int retval = lwip_write(sock, tmpbuffer, nbytes);  
    mem_free(tmpbuffer);
    return retval;
}

static int netif_fcntl(int sock, int cmd, long arg)
{
    return lwip_fcntl(sock, cmd, arg);  
}

static int netif_close(int sock)
{
    // TODO: add ref dec, if ref==0, then call close.
    return lwip_close(sock);  
}

static int netif_ioctl(int sock, int cmd, unsigned long arg)
{
    return lwip_ioctl(sock, cmd, (void *)arg);
}

static int netif_incref(int sock)
{
    // TODO: add ref inc
    return 0;  
}

static int netif_decref(int sock)
{
    // TODO: add ref dec
    return 0;  
}

fsal_t fsal_netif = {
    .name       = "netif",
    .subtable   = NULL,
    .mkfs       = NULL,
    .mount      = NULL,
    .unmount    = NULL,
    .open       = NULL,
    .close      = netif_close,
    .read       = netif_read,
    .write      = netif_write,
    .lseek      = NULL,
    .opendir    = NULL,
    .closedir   = NULL,
    .readdir    = NULL,
    .mkdir      = NULL,
    .unlink     = NULL,
    .rename     = NULL,
    .ftruncate  = NULL,
    .fsync      = NULL,
    .state      = NULL,
    .chmod      = NULL,
    .fchmod     = NULL,
    .utime      = NULL,
    .feof       = NULL,
    .ferror     = NULL,
    .ftell      = NULL,
    .fsize      = NULL,
    .rewind     = NULL,
    .rewinddir  = NULL,
    .rmdir      = NULL,
    .chdir      = NULL,
    .ioctl      = netif_ioctl,
    .fcntl      = netif_fcntl,
    .fstat      = NULL,
    .access     = NULL,
    .incref     = netif_incref,
    .decref     = netif_decref,
};

