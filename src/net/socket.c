#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <xbook/spinlock.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/file.h>

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

    fsal_file_t *fp = fsal_file_alloc();
    if (fp == NULL) {
        errprint("%s: alloc file struct failed!\n", __func__);
        return -ENOMEM;
    }

    dbgprint("call sys_socket: domain=%d type=%d protocol=%d\n", domain, type, protocol);
    int sock = do_socket(domain, type, protocol);
    dbgprint("do socket: get socket=%d\n", sock);
    if (sock < 0) {
        errprint("sys socket: get socket=%d error!\n", sock);
        fsal_file_free(fp);
        return -ENOMEM;
    }
    socket_cache_t *socache = socket_cache_create(sock);
    if (!socache) {
        errprint("sys socket: create socket cache for sock %d error!\n", sock);        
        fsal_file_free(fp);
        do_close(sock);
        return -ENOMEM;
    }

    int fd = local_fd_install(sock, FILE_FD_SOCKET);
    if (fd < 0) {
        fsal_file_free(fp);
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

static int do_read(int sock, void *buf, size_t len)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -ENOMEM;
    }
    memset(buf, 0, len);
    lpc_parcel_write_int(parcel, sock); 
    lpc_parcel_write_sequence(parcel, buf, len); // 写入缓冲区，预留位置
    if (lpc_call(LPC_ID_NET, NETCALL_read, parcel, parcel) < 0) {
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

int netif_read(int sock, void *buffer, size_t nbytes)
{
    if (sock < 0 || !buffer || !nbytes)
        return -EINVAL;
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("netif read: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    if (atomic_get(&socache->reference) <= 0) {
        noteprint("netif read: socket %d reference %d error!\n",
            sock, atomic_get(&socache->reference));            
        return -EPERM;
    }
    int retval = do_read(sock, buffer, nbytes);
    if (retval < 0) {
        errprint("netif read: do read sock %d nbytes %d failed!\n", sock, nbytes);
    }
    return retval;
}

static int do_write(int sock, const void *buf, size_t len)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -ENOMEM;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_sequence(parcel, (void *)buf, len);
    if (lpc_call(LPC_ID_NET, NETCALL_write, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int netif_write(int sock, void *buffer, size_t nbytes)
{
    if (sock < 0 || !buffer || !nbytes)
        return -EINVAL;
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("netif write: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    if (atomic_get(&socache->reference) <= 0) {
        noteprint("netif write: socket %d reference %d error!\n",
            sock, atomic_get(&socache->reference));            
        return -EPERM;
    }
    int retval = do_write(sock, buffer, nbytes);
    if (retval < 0) {
        errprint("netif write: do write sock %d nbytes %d failed!\n", sock, nbytes);
    }
    return retval;
}

static int do_ioctl(int sock, int request, void *arg)
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

int netif_ioctl(int sock, int request, void *arg)
{
    /* arg 可以为NULL */
    if (sock < 0)
        return -EINVAL;
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("netif ioctl: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    if (atomic_get(&socache->reference) <= 0) {
        noteprint("netif ioctl: socket %d reference %d error!\n",
            sock, atomic_get(&socache->reference));            
        return -EPERM;
    }
    int retval = do_ioctl(sock, request, arg);
    if (retval < 0) {
        errprint("netif ioctl: do ioctl sock %d request %d failed!\n", sock, request);
    }
    return retval;   
}

static int do_fcntl(int sock, int cmd, int val)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, sock);
    lpc_parcel_write_int(parcel, cmd);
    lpc_parcel_write_int(parcel, val);
    if (lpc_call(LPC_ID_NET, NETCALL_fcntl, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    lpc_parcel_put(parcel);
    return retval;
}

int netif_fcntl(int sock, int cmd, long val)
{
    if (sock < 0)
        return -EINVAL;
    infoprint("netif fcntl: sock %d enter\n", sock);            
    
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("netif fcntl: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    if (atomic_get(&socache->reference) <= 0) {
        noteprint("netif fcntl: socket %d reference %d error!\n",
            sock, atomic_get(&socache->reference));            
        return -EPERM;
    }
    int retval = do_fcntl(sock, cmd, val);
    if (retval < 0) {
        errprint("netif fcntl: do fcntl sock %d request %d failed!\n", sock, cmd);
    }
    return retval;   
}

