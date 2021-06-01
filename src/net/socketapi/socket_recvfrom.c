#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <xbook/safety.h>
#include <string.h>

#ifndef CONFIG_NETREMOTE
#include <lwip/sockets.h>
#endif

static int do_recvfrom(int sock, void *buf, int len, unsigned int flags,
    struct sockaddr *from, socklen_t *fromlen)
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
    #else
    return lwip_recvfrom(sock, buf, len, flags, from, fromlen);
    #endif
}

static int recvfrom_large(int sock, void *buf, int len, int flags,
    struct sockaddr *from, socklen_t *fromlen)
{
    char *_mbuf = mem_alloc(FSIF_RW_CHUNK_SIZE);
    if (_mbuf == NULL) {
        return -ENOMEM;
    }
    int total = 0;
    char *p = (char *)buf;
    size_t chunk = len % FSIF_RW_CHUNK_SIZE;
    while (len > 0) {
        int rd = do_recvfrom(sock, _mbuf, chunk, flags, from, fromlen);
        if (rd < 0) {
            errprint("[net] recvfrom_large: sock %d do read failed!", sock);
            total = -EIO;
            break;
        }
        if (mem_copy_to_user(p, _mbuf, chunk) < 0) {
            errprint("[net] recvfrom_large: copy buf %p to user failed!", p);
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
    struct sockaddr __from;
    socklen_t __fromlen;
    int retval;
    if (len > FSIF_RW_BUF_SIZE) {
        retval = recvfrom_large(sock, buf, len, flags, &__from, &__fromlen);
    } else {
        char _buf[FSIF_RW_BUF_SIZE] = {0};
        retval = do_recvfrom(sock, _buf, len, flags, &__from, &__fromlen);
        if (retval < 0) {
            errprint("%s: call service sock %d failed!\n", __func__, sock);
        } else {
            if (mem_copy_to_user(buf, _buf, retval) < 0) {
                errprint("%s: copy buf %p to user failed!", __func__, buf);
                return -EINVAL;
            }
        }
    }

    if (fromlen) { 
        if (mem_copy_to_user(fromlen, &__fromlen, sizeof(socklen_t)) < 0) {
            errprint("%s: copy fromlen on sock %d error!\n", __func__, sock);         
            return -EINVAL;
        }
    }
    if (from) {
        if (mem_copy_to_user(from, &__from, sizeof(struct sockaddr)) < 0) {
            errprint("%s: copy from on sock %d error!\n", __func__, sock);            
            return -EINVAL;
        }
    }
    #ifdef DEBUG_NETIF
    dbgprint("%s: from: %d-%d len:%d\n", __func__, __from.sa_len, __from.sa_family, __fromlen);
    #endif
    return retval;
}

