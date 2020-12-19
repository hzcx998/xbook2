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
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    lpc_parcel_put(parcel);
    return num;
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
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    lpc_parcel_put(parcel);
    return num;
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
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    lpc_parcel_put(parcel);
    return num;

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
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    lpc_parcel_put(parcel);
    return num;
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
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    if (num >= 0) { // accept ok
        struct sockaddr *addr_;
        socklen_t addrlen_;
        lpc_parcel_read_sequence(parcel, (void **)&addr_, (size_t *)&addrlen_);
        memcpy(addr, addr_, addrlen_);
        *addrlen = addrlen_;
    }
    lpc_parcel_put(parcel);
    return num;
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
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    lpc_parcel_put(parcel);
    return num;
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
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    if (num > 0) {
        void *recvbuf;
        size_t len_;
        lpc_parcel_read_sequence(parcel, &recvbuf, &len_);
        memcpy(buf, recvbuf, len_);
    }
    lpc_parcel_put(parcel);
    return num;
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
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    lpc_parcel_put(parcel);
    return num;
}
