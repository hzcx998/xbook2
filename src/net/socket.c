#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <xbook/spinlock.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>

static int do_socket(int domain, int type, int protocol)
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

static int do_close(int sock)
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

int sys_socket(int domain, int type, int protocol)
{
    /* TODO: 检测参数 */

    dbgprint("call sys_socket: domain=%d type=%d protocol=%d\n", domain, type, protocol);
    int sock = do_socket(domain, type, protocol);
    dbgprint("do socket: get socket=%d\n", sock);
    if (sock < 0) {
        errprint("sys socket: get socket=%d error!\n", sock);
        return -ENOMEM;
    }
    socket_cache_t *socache = socket_cache_create(sock);
    if (!socache) {
        errprint("sys socket: create socket cache for sock %d error!\n", sock);        
        do_close(sock);
        return -ENOMEM;
    }

    int fd = local_fd_install(sock, FILE_FD_SOCKET);
    if (fd < 0) {
        socket_cache_destroy(socache);
        do_close(sock);
        return -ENOMEM;
    }
    return fd;
}

int netif_close(int sock)
{
    dbgprint("netif close: sock %d\n", sock);
    if (sock < 0)
        return -EINVAL;
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("netif close: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    if (socket_cache_dec(socache) < 0) {
        errprint("netif close: dec socket cache reference for sock %d error!\n", sock);            
        return -ESRCH;
    }
    if (atomic_get(&socache->reference) >= 1) {
        noteprint("netif close: socket %d reference %d not zero, not real close.\n",
            sock, atomic_get(&socache->reference));            
        return 0;
    }
    int retval = do_close(sock);
    if (retval < 0) {
        errprint("netif close: do close sock %d failed!\n", sock);
        return -1;
    }
    socket_cache_destroy(socache);
    infoprint("netif close: do close sock %d real.\n", sock);
    
    return 0;
}

int netif_incref(int sock)
{
    if (sock < 0)
        return -EINVAL;
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("netif incref: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    infoprint("netif incref: sock %d.\n", sock);            
    return socket_cache_inc(socache);
}

int netif_decref(int sock)
{
    if (sock < 0)
        return -EINVAL;
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("netif decref: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    infoprint("netif decref: sock %d.\n", sock);            
    return socket_cache_dec(socache);
}
