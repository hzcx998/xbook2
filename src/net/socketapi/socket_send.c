#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <xbook/debug.h>
#include <xbook/safety.h>

#ifndef CONFIG_NETREMOTE
#include <lwip/sockets.h>
#endif

static int do_send(int sock, const void *buf, int len, int flags)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_send(sock, buf, len, flags);
    #endif
}

static int send_large(int sock, const void *buf, int len, int flags)
{
    char *_mbuf = mem_alloc(FSIF_RW_CHUNK_SIZE);
    if (_mbuf == NULL) {
        return -ENOMEM;
    }
    int total = 0;
    char *p = (char *)buf;
    size_t chunk = len % FSIF_RW_CHUNK_SIZE;
    while (len > 0) {
        if (mem_copy_from_user(_mbuf, p, chunk) < 0) {
            errprint("[net] send_large: copy buf %p from user failed!", p);
            total = -EINVAL;
            break;
        }
        int wr = do_send(sock, _mbuf, chunk, flags);
        if (wr < 0) {
            errprint("[net] send_large: sock %d do send failed!", sock);
            total = -EIO;
            break;
        }
        // dbgprintln("[fs] sys_write: chunk %d wr %d\n", chunk, wr);
        p += chunk;
        total += wr;
        len -= chunk;
        chunk = FSIF_RW_CHUNK_SIZE;
    }
    mem_free(_mbuf);
    return total;
}

int sys_socket_send(int fd, const void *buf, int len, int flags)
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
    if (len > FSIF_RW_BUF_SIZE) {
        return send_large(sock, buf, len, flags);
    } else {
        char _buf[FSIF_RW_BUF_SIZE] = {0};
        if (mem_copy_from_user(_buf, (void *)buf, len) < 0) {
            errprint("%s: copy buf %p from user failed!", __func__, buf);
            return -EINVAL;
        }
        int retval = do_send(sock, _buf, len, flags);
        if (retval < 0) {
            errprint("%s: do_send sock %d failed!\n", __func__, sock);
        }
        return retval;
    }
}

