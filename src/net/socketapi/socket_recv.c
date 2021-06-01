#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <string.h>
#include <xbook/fd.h>
#include <xbook/safety.h>

#ifndef CONFIG_NETREMOTE
#include <lwip/sockets.h>
#endif

static int do_recv(int sock, void *buf, int len, int flags)
{
    #ifdef CONFIG_NETREMOTE
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
    int retval = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&retval);
    if (retval > 0) {
        lpc_parcel_read_sequence(parcel, buf, NULL);
    }
    lpc_parcel_put(parcel);
    return retval;
    #else
    return lwip_recv(sock, buf, len, flags);
    #endif
}

static int recv_large(int sock, void *buf, int len, int flags)
{
    char *_mbuf = mem_alloc(FSIF_RW_CHUNK_SIZE);
    if (_mbuf == NULL) {
        return -ENOMEM;
    }
    int total = 0;
    char *p = (char *)buf;
    size_t chunk = len % FSIF_RW_CHUNK_SIZE;
    while (len > 0) {
        int rd = do_recv(sock, _mbuf, chunk, flags);
        if (rd < 0) {
            errprint("[net] recv_large: sock %d do read failed!", sock);
            total = -EIO;
            break;
        }
        if (mem_copy_to_user(p, _mbuf, chunk) < 0) {
            errprint("[net] recv_large: copy buf %p to user failed!", p);
            total = -EINVAL;
            break;
        }
        // dbgprintln("[fs] sys_write: chunk %d wr %d\n", chunk, wr);
        p += chunk;
        total += rd;
        len -= chunk;
        chunk = FSIF_RW_CHUNK_SIZE;
    }
    mem_free(_mbuf);
    return total;
}

int sys_socket_recv(int fd, void *buf, int len, int flags)
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
        return recv_large(sock, buf, len, flags);
    } else {
        char _buf[FSIF_RW_BUF_SIZE] = {0};
        int retval = do_recv(sock, _buf, len, flags);
        if (retval < 0) {
            errprint("%s: call service sock %d failed!\n", __func__, sock);
        } else {
            if (mem_copy_to_user(buf, _buf, retval) < 0) {
                errprint("[net] sys_socket_recv: copy buf %p to user failed!", buf);
                return -EINVAL;
            }
        }
        return retval;
    }
    
}

