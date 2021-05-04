#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <xbook/fd.h>

static int do_accept(int sock, struct sockaddr *addr, socklen_t *addrlen)
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
        if (addr)
            lpc_parcel_read_sequence(parcel, (void *)addr, (size_t *)addrlen);
    }
    lpc_parcel_put(parcel);
    return retval;
}

int sys_socket_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    if (fd < 0)
        return -EINVAL;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        errprint("%s: fd %d err!\n", __func__, fd);
        return -EINVAL;
    }
    int sock = ffd->handle;
    
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("%s: find socket cache for sock %d error!\n", __func__, sock);            
        return -ESRCH;
    }
    if (atomic_get(&socache->reference) <= 0) {
        noteprint("%s: socket %d reference %d error!\n",
            __func__, sock, atomic_get(&socache->reference));            
        return -EPERM;
    }
    keprint("socket accept sock %d\n" endl, sock);
    
    int new_sock = do_accept(sock, addr, addrlen);
    if (new_sock < 0) {
        errprint("%s: call service sock %d failed!\n", __func__, sock);
        return new_sock;
    }
    /* 如果返回值为正，那么创建一个套接字cache，并安装到fd中 */
    socket_cache_t *new_socache = socket_cache_create(new_sock);
    if (!new_socache) {
        errprint("sys socket accept: create socket cache for sock %d error!\n", new_sock);        
        do_socket_close(new_sock);
        return -ENOMEM;
    }
    int new_fd = local_fd_install(new_sock, FILE_FD_SOCKET);
    if (new_fd < 0) {
        errprint("sys socket accept: create fd for sock %d error!" endl, new_sock);        
        socket_cache_destroy(new_socache);
        do_socket_close(new_sock);
        return -ENOMEM;
    }
    keprint("socket accept fd %d socket %d ok" endl, new_fd, new_sock);
    return new_fd;
}

