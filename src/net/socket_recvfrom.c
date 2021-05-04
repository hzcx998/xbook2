#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <string.h>

static int do_recvfrom(int sock, void *buf, int len, unsigned int flags,
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

int sys_socket_recvfrom(int fd, void *buf, int len, unsigned int flags,
    struct sockaddr *from, socklen_t *fromlen)
{
    if (fd < 0 || !buf || !len)
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
    int retval = do_recvfrom(sock, buf, len, flags, from, fromlen);
    if (retval < 0) {
        errprint("%s: call service sock %d failed!\n", __func__, sock);
    }
    return retval;
}

