#include <sys/syscall.h>
#include <sys/sockcall.h>
#include <sys/socket.h>
#include <errno.h>

/**
 * sockcall is a syscall for socket call
 * @sockop: socket operate
 * @param:  socket param for different operate
 * 
 * @return:failed return -1, success return retval
 */
int sockcall(int sockop, sock_param_t *param)
{
    int retval = syscall2(int, SYS_SOCKCALL, sockop, param);
    if (retval < 0) {
        _set_errno(-retval);
        return -1;
    }
    return retval;
}

int socket(int domain, int type, int protocol)
{
    sock_param_t param;
    param.socket.domain = domain;
    param.socket.type = type;
    param.socket.protocol = protocol;
    return sockcall(SOCKOP_socket, &param);
}

int bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    sock_param_t param;
    param.bind.sock = sockfd;
    param.bind.my_addr = my_addr;
    param.bind.addrlen = addrlen;
    return sockcall(SOCKOP_bind, &param);
}

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    sock_param_t param;
    param.connect.sock = sockfd;
    param.connect.serv_addr = serv_addr;
    param.connect.addrlen = addrlen;
    return sockcall(SOCKOP_connect, &param);
}

int listen(int sockfd, int backlog)
{
    sock_param_t param;
    param.listen.sock = sockfd;
    param.listen.backlog = backlog;
    return sockcall(SOCKOP_listen, &param);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    sock_param_t param;
    param.accept.sock = sockfd;
    param.accept.addr = addr;
    param.accept.addrlen = addrlen;
    return sockcall(SOCKOP_accept, &param);
}

int send(int sockfd, const void *buf, int len, int flags)
{
    sock_param_t param;
    param.send.sock = sockfd;
    param.send.buf = buf;
    param.send.len = len;
    param.send.flags = flags;
    return sockcall(SOCKOP_send, &param);
}

int recv(int sockfd, void *buf, int len, int flags)
{
    sock_param_t param;
    param.recv.sock = sockfd;
    param.recv.buf = buf;
    param.recv.len = len;
    param.recv.flags = flags;
    return sockcall(SOCKOP_recv, &param);
}

int sendto(int sockfd, const void *buf, int len, unsigned int flags,
    const struct sockaddr *to, socklen_t tolen)
{
    sock_param_t param;
    param.sendto.sock = sockfd;
    param.sendto.buf = buf;
    param.sendto.len = len;
    param.sendto.flags = flags;
    param.sendto.to = to;
    param.sendto.tolen = tolen;
    return sockcall(SOCKOP_sendto, &param);
}

int recvfrom(int sockfd, void *buf, int len, unsigned int flags,
    struct sockaddr *from, socklen_t *fromlen)
{
    sock_param_t param;
    param.recvfrom.sock = sockfd;
    param.recvfrom.buf = buf;
    param.recvfrom.len = len;
    param.recvfrom.flags = flags;
    param.recvfrom.from = from;
    param.recvfrom.fromlen = fromlen;
    return sockcall(SOCKOP_recvfrom, &param);
}

int shutdown(int sockfd, int how)
{
    sock_param_t param;
    param.shutdown.sock = sockfd;
    param.shutdown.how = how;
    return sockcall(SOCKOP_shutdown, &param);
}

int getpeername(int sockfd, struct sockaddr *serv_addr, socklen_t *addrlen)
{
    sock_param_t param;
    param.getpeername.sock = sockfd;
    param.getpeername.serv_addr = serv_addr;
    param.getpeername.addrlen = addrlen;
    return sockcall(SOCKOP_getpeername, &param);
}

int getsockname(int sockfd, struct sockaddr *my_addr, socklen_t *addrlen)
{
    sock_param_t param;
    param.getsockname.sock = sockfd;
    param.getsockname.my_addr = my_addr;
    param.getsockname.addrlen = addrlen;
    return sockcall(SOCKOP_getsockname, &param);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    sock_param_t param;
    param.getsockopt.sock = sockfd;
    param.getsockopt.level = level;
    param.getsockopt.optname = optname;
    param.getsockopt.optval = optval;
    param.getsockopt.optlen = optlen;
    return sockcall(SOCKOP_getsockopt, &param);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    sock_param_t param;
    param.setsockopt.sock = sockfd;
    param.setsockopt.level = level;
    param.setsockopt.optname = optname;
    param.setsockopt.optval = optval;
    param.setsockopt.optlen = optlen;
    return sockcall(SOCKOP_setsockopt, &param);
}
