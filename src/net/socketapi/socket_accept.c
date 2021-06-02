#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <xbook/safety.h>
#include <xbook/netif.h>

#ifndef CONFIG_NETREMOTE
#include <lwip/sockets.h>
#endif

static int do_accept(int sock, struct sockaddr *addr, socklen_t *addrlen)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_accept(sock, addr, addrlen);
    #endif
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
    #ifdef DEBUG_NETIF
    keprint("socket accept sock %d\n" endl, sock);
    #endif
    struct sockaddr __addr;
    socklen_t __addrlen = 0;
    int new_sock = do_accept(sock, &__addr, &__addrlen);
    if (new_sock < 0) {
        errprint("%s: call service sock %d failed!\n", __func__, sock);
        do_socket_close(new_sock);
        return new_sock;
    }
    if (addrlen) { 
        if (mem_copy_to_user(addrlen, &__addrlen, sizeof(socklen_t)) < 0) {
            errprint("%s: copy addr on sock %d error!\n", __func__, sock);   
            do_socket_close(new_sock);         
            return -EINVAL;
        }
    }
    if (addr) {
        if (mem_copy_to_user(addr, &__addr, sizeof(struct sockaddr)) < 0) {
            errprint("%s: copy addrlen on sock %d error!\n", __func__, sock);        
            do_socket_close(new_sock);         
            return -EINVAL;
        }
    }
    /* 如果返回值为正，那么创建一个套接字cache，并安装到fd中 */
    socket_cache_t *new_socache = socket_cache_create(new_sock);
    if (!new_socache) {
        errprint("%s: create socket cache for sock %d error!\n", __func__, new_sock);        
        do_socket_close(new_sock);
        return -ENOMEM;
    }
    int new_fd = local_fd_install(new_sock, FILE_FD_SOCKET);
    if (new_fd < 0) {
        errprint("%s: create fd for sock %d error!" endl, __func__, new_sock);        
        socket_cache_destroy(new_socache);
        do_socket_close(new_sock);
        return -ENOMEM;
    }
    #ifdef DEBUG_NETIF
    keprint("socket accept fd %d socket %d ok" endl, new_fd, new_sock);
    #endif
    return new_fd;
}

