
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
    struct sockaddr *my_addr;
    int addrlen;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0)
        return false;
    lpc_parcel_read_sequence(data, (void **)&my_addr, (size_t *)&addrlen);
    int retval = lwip_bind(sock, my_addr, addrlen);
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
    struct sockaddr *serv_addr;
    int addrlen;
    lpc_parcel_read_int(data, (uint32_t *)&sock);
    if (sock < 0) {
        lpc_parcel_write_int(reply, -1);
        return false;    
    }
    lpc_parcel_read_sequence(data, (void **)&serv_addr, (size_t *)&addrlen);
    int retval = lwip_connect(sock, serv_addr, addrlen);
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
    lpc_parcel_read_sequence(data, (void **)&buf, (size_t *)&len);
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
    lpc_parcel_read_sequence(data, (void **)&buf, (size_t *)&len);
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

static lpc_remote_handler_t net_remote_table[] = {
    remote_socket,
    remote_bind,
    remote_connect,
    remote_listen,
    remote_accept,
    remote_send,
    remote_recv,
    remote_close,
    //remote_ioctl,
};

bool netserv_echo_main(uint32_t code, lpc_parcel_t data, lpc_parcel_t reply)
{
    if (code >= FIRST_CALL_CODE && code < NETCALL_LAST_CALL)
        return net_remote_table[code - 1](data, reply);
    return false;
}
