#include <xbook/netif.h>
#include <xbook/file.h>
#include <xbook/fd.h>

#ifndef CONFIG_NETREMOTE
#include <lwip/sockets.h>
#endif

#define NUM_SOCKETS MEMP_NUM_NETCONN

int netif_select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout)
{
    #ifdef DEBUG_SELECT
    dbgprint("netif select start\n");
    #endif
    /* 将fd转换为sock套接字资源 */
    fd_set __readfds, __writefds, __exceptfds;
    FD_ZERO(&__readfds);
    FD_ZERO(&__writefds);
    FD_ZERO(&__exceptfds);
    int n = min(NUM_SOCKETS, maxfdp);
    int i;
    for (i = 0; i < n; i++) {
        if (readfds) {
            if (FD_ISSET(i, readfds)) {
                file_fd_t *ffd = fd_local_to_file(i);
                /* 将句柄转换为fd_set */
                if (ffd->handle >= 0 && ffd->handle < n) {
                    FD_SET(ffd->handle, &__readfds);
                }
            }
        }
        if (writefds) {
            if (FD_ISSET(i, writefds)) {
                file_fd_t *ffd = fd_local_to_file(i);
                if (ffd->handle >= 0 && ffd->handle < n) {
                    FD_SET(ffd->handle, &__writefds);
                }
            }
        }
        if (exceptfds) {
            if (FD_ISSET(i, exceptfds)) {
                file_fd_t *ffd = fd_local_to_file(i);
                if (ffd->handle >= 0 && ffd->handle < n) {
                    FD_SET(ffd->handle, &__exceptfds);
                }
            }
        }
    }
    /* 转换完成 */
    #ifdef DEBUG_SELECT
    dbgprint("trasmit from fd to sock start\n");
    fd_set_dump(&__readfds, n);
    fd_set_dump(&__writefds, n);
    fd_set_dump(&__exceptfds, n);
    #endif
    /* 执行select */
    int ret = lwip_select(n, 
            readfds == NULL ? NULL:&__readfds,
            writefds == NULL ? NULL:&__writefds,
            exceptfds == NULL ? NULL:&__exceptfds,
            timeout);
    if (ret < 0) {
        errprint("lwip select failed\n");
        return ret;
    }
    #ifdef DEBUG_SELECT
    dbgprint("trasmit from sock to fd start\n");
    #endif
    /* 将sock转换为fd */
    for (i = 0; i < n; i++) {
        if (readfds) {
            if (FD_ISSET(i, readfds)) {
                file_fd_t *ffd = fd_local_to_file(i);
                if (ffd->handle >= 0 && ffd->handle < n) {
                    /* 如果fd对应的sock不为1，那么就将fd置0 */
                    if (!FD_ISSET(ffd->handle, &__readfds)) {
                        FD_CLR(i, readfds);
                    }
                }
            }
        }
        if (writefds) {
            if (FD_ISSET(i, writefds)) {
                file_fd_t *ffd = fd_local_to_file(i);
                if (ffd->handle >= 0 && ffd->handle < n) {
                    if (!FD_ISSET(ffd->handle, &__writefds)) {
                        FD_CLR(i, writefds);
                    }
                }
            }
        }
        if (exceptfds) {
            if (FD_ISSET(i, exceptfds)) {
                file_fd_t *ffd = fd_local_to_file(i);
                if (ffd->handle >= 0 && ffd->handle < n) {
                    if (!FD_ISSET(ffd->handle, &__exceptfds)) {
                        FD_CLR(i, exceptfds);
                    }
                }
            }
        }
    }
    #ifdef DEBUG_SELECT
    fd_set_dump(readfds, n);
    fd_set_dump(writefds, n);
    fd_set_dump(exceptfds, n);
    dbgprint("netif select done\n");
    #endif
    return ret;
}