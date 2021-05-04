#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <xbook/fd.h>

static int do_setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen)
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

int sys_socket_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    if (fd < 0 || !optval || !optlen)
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
    int retval = do_setsockopt(sock, level, optname, optval, optlen);
    if (retval < 0) {
        errprint("%s: call service sock %d failed!\n", __func__, sock);
    }
    return retval;
}

