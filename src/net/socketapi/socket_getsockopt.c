#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <xbook/safety.h>

#ifndef CONFIG_NETREMOTE
#include <lwip/sockets.h>
#endif

static int do_getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_getsockopt(sock, level, optname, optval, optlen);
    #endif
}

int sys_socket_getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
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
    char __optval[32];
    socklen_t __optlen;
    int retval = do_getsockopt(sock, level, optname, __optval, &__optlen);
    if (retval < 0) {
        errprint("%s: call service sock %d failed!\n", __func__, sock);
    }
    if (optlen) { 
        if (mem_copy_to_user(optlen, &__optlen, sizeof(socklen_t)) < 0) {
            errprint("%s: copy arg on sock %d error!\n", __func__, sock);         
            return -EINVAL;
        }
    }
    if (optval) {
        if (mem_copy_to_user(optval, __optval, __optlen) < 0) {
            errprint("%s: copy arg on sock %d error!\n", __func__, sock);            
            return -EINVAL;
        }
    }
    return retval;
}

