#include <netsocket.h>
#include <net.client.h>
#include <sys/lpc.h>
#include <string.h>

int net_socket(int domain, int type, int protocol)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, domain);
    lpc_parcel_write_int(parcel, type);
    lpc_parcel_write_int(parcel, protocol);
    if (lpc_call(LPC_ID_NET, NETCALL_socket, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int net_bind(int sock, struct sockaddr *my_addr, int addrlen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_sequence(parcel, my_addr, addrlen);
    if (lpc_call(LPC_ID_NET, NETCALL_bind, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int net_connect(int sock, struct sockaddr *serv_addr, int addrlen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_sequence(parcel, serv_addr, addrlen);
    if (lpc_call(LPC_ID_NET, NETCALL_connect, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;

}

int net_listen(int sock, int backlog)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_int(parcel, backlog);
    if (lpc_call(LPC_ID_NET, NETCALL_listen, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int net_accept(int sock, struct sockaddr *addr, socklen_t *addrlen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    if (lpc_call(LPC_ID_NET, NETCALL_accept, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    if (retval >= 0) { // accept ok
        lpc_parcel_read_sequence(parcel, (void *)&addr, (size_t *)addrlen);
    }
    lpc_parcel_put(parcel);
    return retval;
}

int net_send(int sock, const void *buf, int len, int flags)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_sequence(parcel, (void *)buf, len);
    lpc_parcel_write_int(parcel, flags);
    if (lpc_call(LPC_ID_NET, NETCALL_send, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int net_sendto(int sock, const void *buf, int len, unsigned int flags,
    const struct sockaddr *to, socklen_t tolen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_sequence(parcel, (void *)buf, len);
    lpc_parcel_write_int(parcel, flags);
    lpc_parcel_write_sequence(parcel, (void *)to, tolen);
    if (lpc_call(LPC_ID_NET, NETCALL_sendto, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int net_recv(int sock, void *buf, int len, int flags)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    memset(buf, 0, len);
    lpc_parcel_write_int(parcel, sock); 
    lpc_parcel_write_sequence(parcel, buf, len); // 写入缓冲区，预留位置
    lpc_parcel_write_int(parcel, flags);
    if (lpc_call(LPC_ID_NET, NETCALL_recv, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    if (retval > 0) {
        lpc_parcel_read_sequence(parcel, buf, NULL);
    }
    lpc_parcel_put(parcel);
    return retval;
}

int net_recvfrom(int sock, void *buf, int len, unsigned int flags,
    struct sockaddr *from, socklen_t *fromlen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    memset(buf, 0, len);
    lpc_parcel_write_int(parcel, sock); 
    lpc_parcel_write_sequence(parcel, buf, len); // 写入缓冲区，预留位置
    lpc_parcel_write_int(parcel, flags);
    if (lpc_call(LPC_ID_NET, NETCALL_recvfrom, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    if (retval > 0) {
        lpc_parcel_read_sequence(parcel, buf, NULL);
        lpc_parcel_read_sequence(parcel, (void *)from, (size_t *)fromlen);
    }
    lpc_parcel_put(parcel);
    return retval;
}

int net_close(int sock)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    if (lpc_call(LPC_ID_NET, NETCALL_close, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int net_ioctl(int sock, int request, void *arg)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_int(parcel, request);
    lpc_parcel_write_sequence(parcel, arg, sizeof(void *));
    if (lpc_call(LPC_ID_NET, NETCALL_ioctl, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_read_sequence(parcel, arg, NULL);
    lpc_parcel_put(parcel);
    return retval;
}

int net_shutdown(int sock, int how)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_int(parcel, how);
    if (lpc_call(LPC_ID_NET, NETCALL_shutdown, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int net_getpeername(int sock, struct sockaddr *serv_addr, socklen_t *addrlen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    if (lpc_call(LPC_ID_NET, NETCALL_getpeername, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    if (retval >= 0) {
        lpc_parcel_read_sequence(parcel, (void *)serv_addr, (size_t *)addrlen);    
    }
    lpc_parcel_put(parcel);
    return retval;
}

int net_getsockname(int sock, struct sockaddr *my_addr, socklen_t *addrlen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    if (lpc_call(LPC_ID_NET, NETCALL_getsockname, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    if (retval >= 0) {
        lpc_parcel_read_sequence(parcel, (void *)my_addr, (size_t *)addrlen);    
    }
    lpc_parcel_put(parcel);
    return retval;
}

int net_getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_int(parcel, level);
    lpc_parcel_write_int(parcel, optname);
    if (lpc_call(LPC_ID_NET, NETCALL_getsockopt, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    if (retval >= 0) {
        lpc_parcel_read_sequence(parcel, optval, (size_t *)optlen);    
    }
    lpc_parcel_put(parcel);
    return retval;
}

int net_setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_int(parcel, level);
    lpc_parcel_write_int(parcel, optname);
    lpc_parcel_write_sequence(parcel, (void *)optval, optlen);
    if (lpc_call(LPC_ID_NET, NETCALL_setsockopt, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}
