
#include <sys/lpc.h>
#include <net.client.h>
#include <errno.h>
#include <lwip/sockets.h>

static bool remote_socket(lpc_parcel_t data, lpc_parcel_t reply)
{
    int domain, type, protocol;
    lpc_parcel_read_int(data, (uint32_t *)&domain);
    lpc_parcel_read_int(data, (uint32_t *)&type);
    lpc_parcel_read_int(data, (uint32_t *)&protocol);
    int socket_id = lwip_socket(domain, type, protocol);
    if (socket_id < 0) {
        lpc_parcel_write_int(reply, -EPERM);
        return false;    
    }
    lpc_parcel_write_int(reply, socket_id);
    return true;    
}

static bool remote_bind(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    struct sockaddr my_addr;
    int addrlen;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    lpc_parcel_read_sequence(data, (void *)&my_addr, (size_t *)&addrlen);
    int retval = lwip_bind(sock, (const struct sockaddr *)&my_addr, addrlen);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    return true;    
}

static bool remote_connect(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    struct sockaddr serv_addr;
    int addrlen;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_sequence(data, (void *)&serv_addr, (size_t *)&addrlen);
    int retval = lwip_connect(sock, (const struct sockaddr *)&serv_addr, addrlen);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    return true;    
}

static bool remote_listen(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    int backlog;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_int(data, (uint32_t *)&backlog);

    int retval = lwip_listen(sock, backlog);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;
    }
    lpc_parcel_write_int(reply, retval);
    return true;    
}

static bool remote_accept(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    struct sockaddr addr;
    socklen_t addrlen;
    int newsock = lwip_accept(sock, &addr, &addrlen);
    if (newsock < 0) {
        lpc_parcel_write_int(reply, newsock);
        return false;
    }
    lpc_parcel_write_int(reply, newsock);
    lpc_parcel_write_sequence(reply, &addr, addrlen);
    return true;    
}

static bool remote_send(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    void *buf;
    int len;
    int flags;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_sequence_buf(data, (void **)&buf, (size_t *)&len);
    lpc_parcel_read_int(data, (uint32_t *)&flags);
    
    int sndbytes = lwip_send(sock, buf, len, flags);
    if (sndbytes < 0) {
        lpc_parcel_write_int(reply, sndbytes);
        return false;
    }
    lpc_parcel_write_int(reply, sndbytes);
    return true;    
}

static bool remote_recv(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    void *buf;
    int len;
    int flags;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_sequence_buf(data, (void **)&buf, (size_t *)&len);
    lpc_parcel_read_int(data, (uint32_t *)&flags);
    int recvbytes = lwip_recv(sock, buf, len, flags);
    if (recvbytes < 0) {
        lpc_parcel_write_int(reply, recvbytes);
        return false;
    }
    lpc_parcel_write_int(reply, recvbytes);
    lpc_parcel_write_sequence(reply, buf, recvbytes);
    return true;    
}

static bool remote_sendto(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    void *buf;
    int len;
    int flags;
    struct sockaddr to;
    socklen_t tolen;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_sequence_buf(data, (void **)&buf, (size_t *)&len);
    lpc_parcel_read_int(data, (uint32_t *)&flags);
    lpc_parcel_read_sequence(data, (void *)&to, (size_t *)&tolen);
    int sndbytes = lwip_sendto(sock, buf, len, flags, (const struct sockaddr *)&to, tolen);
    if (sndbytes < 0) {
        lpc_parcel_write_int(reply, sndbytes);
        return false;
    }
    lpc_parcel_write_int(reply, sndbytes);
    return true;    
}

static bool remote_recvfrom(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    void *buf;
    int len;
    int flags;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_sequence_buf(data, (void **)&buf, (size_t *)&len);
    lpc_parcel_read_int(data, (uint32_t *)&flags);
    struct sockaddr from;
    socklen_t fromlen;
    int recvbytes = lwip_recvfrom(sock, buf, len, flags, &from, &fromlen);
    if (recvbytes < 0) {
        lpc_parcel_write_int(reply, recvbytes);
        return false;
    }
    lpc_parcel_write_int(reply, recvbytes);
    lpc_parcel_write_sequence(reply, buf, recvbytes);
    lpc_parcel_write_sequence(reply, &from, fromlen);
    
    return true;    
}

static bool remote_close(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock; lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    int retval = lwip_close(sock);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    return true;    
}

static bool remote_ioctl(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock; lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    int request;
    void *arg;
    size_t size;
    lpc_parcel_read_int(data, (uint32_t *)&request);
    lpc_parcel_read_sequence_buf(data, (void **)&arg, &size);
    int retval = lwip_ioctl(sock, request, arg);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    lpc_parcel_write_sequence(reply, arg, size);
    return true;    
}

static bool remote_shutdown(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock; lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    int how; lpc_parcel_read_int(data, (uint32_t *)&how);
    int retval = lwip_shutdown(sock, how);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    return true;    
}

static bool remote_getpeername(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock; lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    struct sockaddr serv_addr;
    socklen_t addrlen;
    int retval = lwip_getpeername(sock, &serv_addr, &addrlen);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    lpc_parcel_write_sequence(reply, (void *)&serv_addr, addrlen);
    return true;    
}

static bool remote_getsockname(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock; lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    struct sockaddr serv_addr;
    socklen_t addrlen;
    int retval = lwip_getsockname(sock, &serv_addr, &addrlen);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    lpc_parcel_write_sequence(reply, (void *)&serv_addr, addrlen);
    return true;    
}

static bool remote_getsockopt(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock; lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    int level; lpc_parcel_read_int(data, (uint32_t *)&level);
    int optname; lpc_parcel_read_int(data, (uint32_t *)&optname);
    unsigned long optval;
    socklen_t optlen;
    int retval = lwip_getsockopt(sock, level, optname, (void *)&optval, &optlen);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    lpc_parcel_write_sequence(reply, (void *)&optval, optlen);
    return true;
}

static bool remote_setsockopt(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock; lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    int level; lpc_parcel_read_int(data, (uint32_t *)&level);
    int optname; lpc_parcel_read_int(data, (uint32_t *)&optname);
    void *optval;
    socklen_t optlen;
    lpc_parcel_read_sequence_buf(data, (void **)&optval, (size_t *)&optlen);
    int retval = lwip_setsockopt(sock, level, optname, optval, optlen);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    return true;
}

static bool remote_write(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    void *buf;
    size_t len;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_sequence_buf(data, (void **)&buf, (size_t *)&len);
    
    int sndbytes = lwip_write(sock, (const void *)buf, len);
    if (sndbytes < 0) {
        lpc_parcel_write_int(reply, sndbytes);
        return false;
    }
    lpc_parcel_write_int(reply, sndbytes);
    return true;    
}

static bool remote_read(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock;
    void *buf;
    size_t len;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_sequence_buf(data, (void **)&buf, (size_t *)&len);
    int recvbytes = lwip_read(sock, buf, len);
    if (recvbytes < 0) {
        lpc_parcel_write_int(reply, recvbytes);
        return false;
    }
    lpc_parcel_write_int(reply, recvbytes);
    lpc_parcel_write_sequence(reply, buf, recvbytes);
    return true;    
}

static bool remote_fcntl(lpc_parcel_t data, lpc_parcel_t reply)
{
    int sock; lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    int cmd;
    int val;
    lpc_parcel_read_int(data, (uint32_t *)&cmd);
    lpc_parcel_read_int(data, (uint32_t *)&val);
    
    int retval = lwip_fcntl(sock, cmd, val);
    if (retval < 0) {
        lpc_parcel_write_int(reply, retval);
        return false;    
    }
    lpc_parcel_write_int(reply, retval);
    return true;    
}

static lpc_remote_handler_t net_remote_table[] = {
    remote_socket,
    remote_bind,
    remote_connect,
    remote_listen,
    remote_accept,
    remote_send,
    remote_recv,
    remote_close,
    remote_sendto,
    remote_recvfrom,
    remote_ioctl,
    remote_shutdown,
    remote_getpeername,
    remote_getsockname,
    remote_getsockopt,
    remote_setsockopt,
    remote_read,
    remote_write,
    remote_fcntl,
};

bool netserv_echo_main(uint32_t code, lpc_parcel_t data, lpc_parcel_t reply)
{
    //printf("netserv tid=%d do echo\n", pthread_self()); // 查看是哪个线程在处理
    if (code >= FIRST_CALL_CODE && code < NETCALL_LAST_CALL)
        return net_remote_table[code - 1](data, reply);
    return false;
}
