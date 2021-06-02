#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <xbook/spinlock.h>
#include <sys/lpc.h>
#include <xbook/socketcache.h>
#include <xbook/file.h>
#include <xbook/safety.h>

#ifndef CONFIG_NETREMOTE
#include <lwip/sockets.h>
#endif

int netif_incref(int sock)
{
    if (sock < 0)
        return -EINVAL;
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("netif incref: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    #ifdef DEBUG_NETIF
    infoprint("netif incref: sock %d.\n", sock);            
    #endif
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
    #ifdef DEBUG_NETIF
    infoprint("netif decref: sock %d.\n", sock);            
    #endif
    return socket_cache_dec(socache);
}

static int do_socket(int domain, int type, int protocol)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_socket(domain, type, protocol);
    #endif
}

int do_socket_close(int sock)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_close(sock);
    #endif
}

int sys_socket(int domain, int type, int protocol)
{
    /* TODO: 检测参数 */
    #ifdef DEBUG_NETIF
    dbgprint("call sys_socket: domain=%d type=%d protocol=%d\n", domain, type, protocol);
    #endif
    int sock = do_socket(domain, type, protocol);
    #ifdef DEBUG_NETIF
    dbgprint("do socket: get socket=%d\n", sock);
    #endif
    if (sock < 0) {
        errprint("%s: get socket=%d error!\n", __func__, sock);
        return -ENOMEM;
    }
    socket_cache_t *socache = socket_cache_create(sock);
    if (!socache) {
        errprint("%s: create socket cache for sock %d error!\n", __func__, sock);        
        do_socket_close(sock);
        return -ENOMEM;
    }
    int fd = local_fd_install(sock, FILE_FD_SOCKET);
    if (fd < 0) {
        socket_cache_destroy(socache);
        do_socket_close(sock);
        return -ENOMEM;
    }
    return fd;
}

/**
 * FIXME: 当进程退出时，如果sock还处于等待中，或者sock处于执行中，那么
 * 强制退出时，关闭sock可能会失败，比如正在recv中，此时调用close肯定有问题。
 * 所以，需要再close时，产生一个信号之类的给lwip core，让它结束当前sock的处理，
 * 转向close操作。
 */
int netif_close(int sock)
{
    #ifdef DEBUG_NETIF
    dbgprint("netif close: sock %d\n", sock);
    #endif
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
    int retval = do_socket_close(sock);
    if (retval < 0) {
        errprint("netif close: do close sock %d failed!\n", sock);
        return -1;
    }
    socket_cache_destroy(socache);
    #ifdef DEBUG_NETIF
    infoprint("netif close: do close sock %d real.\n", sock);
    #endif
    return 0;
}

static int do_read(int sock, void *buf, size_t len)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_read(sock, buf, len);
    #endif
}

static int read_large(int sock, void *buffer, size_t nbytes)
{
    char *_mbuf = mem_alloc(FSIF_RW_CHUNK_SIZE);
    if (_mbuf == NULL) {
        return -ENOMEM;
    }
    int total = 0;
    char *p = (char *)buffer;
    size_t chunk = nbytes % FSIF_RW_CHUNK_SIZE;
    while (nbytes > 0) {
        int rd = do_read(sock, _mbuf, chunk);
        if (rd < 0) {
            errprint("[net] read_large: sock %d do read failed!\n", sock);
            total = -EIO;
            break;
        }
        if (mem_copy_to_user(p, _mbuf, chunk) < 0) {
            errprint("[net] read_large: copy buf %p to user failed!\n", p);
            total = -EINVAL;
            break;
        }
        // dbgprintln("[fs] sys_write: chunk %d wr %d\n", chunk, wr);
        p += chunk;
        total += rd;
        nbytes -= chunk;
        chunk = FSIF_RW_CHUNK_SIZE;
    }
    mem_free(_mbuf);
    return total;
}

int netif_read(int sock, void *buffer, size_t nbytes)
{
    if (sock < 0 || !buffer || !nbytes)
        return -EINVAL;
    socket_cache_t *socache = socket_cache_find(sock);
    if (!socache) {
        errprint("[net] netif read: find socket cache for sock %d error!\n", sock);            
        return -ESRCH;
    }
    if (atomic_get(&socache->reference) <= 0) {
        noteprint("[net] netif read: socket %d reference %d error!\n",
            sock, atomic_get(&socache->reference));            
        return -EPERM;
    }

    if (nbytes > FSIF_RW_BUF_SIZE) {
        return read_large(sock, buffer, nbytes);
    } else {
        char _buf[FSIF_RW_BUF_SIZE] = {0};
        int rd = do_read(sock, _buf, nbytes);
        if (rd > 0) {
            if (mem_copy_to_user(buffer, _buf, rd) < 0) {
                errprint("[net] netif_read: copy buf %p to user failed!\n", buffer);
                return -EINVAL;
            }
        }
        return rd;
    }
}

static int do_write(int sock, const void *buf, size_t len)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_write(sock, buf, len);
    #endif
}

static int write_large(int sock, void *buffer, size_t nbytes)
{
    char *_mbuf = mem_alloc(FSIF_RW_CHUNK_SIZE);
    if (_mbuf == NULL) {
        return -ENOMEM;
    }
    int total = 0;
    char *p = (char *)buffer;
    size_t chunk = nbytes % FSIF_RW_CHUNK_SIZE;
    while (nbytes > 0) {
        if (mem_copy_from_user(_mbuf, p, chunk) < 0) {
            errprint("[net] write_large: copy buf %p from user failed!\n", p);
            total = -EINVAL;
            break;
        }
        int wr = do_write(sock, _mbuf, chunk);
        if (wr < 0) {
            errprint("[net] write_large: sock %d do write failed!\n", sock);
            total = -EIO;
            break;
        }
        p += chunk;
        total += wr;
        nbytes -= chunk;
        chunk = FSIF_RW_CHUNK_SIZE;
    }
    mem_free(_mbuf);
    return total;
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

    if (nbytes > FSIF_RW_BUF_SIZE) {
        return write_large(sock, buffer, nbytes);
    } else {
        char _buf[FSIF_RW_BUF_SIZE] = {0};
        if (mem_copy_from_user(_buf, buffer, nbytes) < 0) {
            errprint("[net] netif_write: copy buf %p from user failed!\n", buffer);
            return -EINVAL;
        }
        //keprintln("buf: %s", _buf);
        return do_write(sock, _buf, nbytes);
    }
}

static int do_ioctl(int sock, int request, void *arg)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_ioctl(sock, request, arg);
    #endif
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
    char __arg[32];
    if (mem_copy_from_user(__arg, (void *)arg, 32) < 0) {
        errprint("%s: copy arg from sock %d error!\n", __func__, sock);            
        return -EINVAL;
    }
    int retval = do_ioctl(sock, request, __arg);
    if (retval < 0) {
        errprint("%s: do ioctl sock %d request %d failed!\n", __func__, sock, request);
    }
    return retval;   
}

static int do_fcntl(int sock, int cmd, int val)
{
    #ifdef CONFIG_NETREMOTE
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
    #else
    return lwip_fcntl(sock, cmd, val);
    #endif
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