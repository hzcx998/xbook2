#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <xbook/net.h>
#include <xbook/fd.h>
#include <sys/lpc.h>

static do_socket(int domain, int type, int protocol)
{
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
}

int sys_socket(int domain, int type, int protocol)
{
    /* TODO: 检测参数 */

    dbgprint("call sys_socket: domain=%d type=%d protocol=%d\n", domain, type, protocol);
    int sock = do_socket(domain, type, protocol);
    dbgprint("do socket: get socket=%d\n", sock);
    if (sock < 0) {
        return -ENOMEM;
    }
    /* TODO: 添加一个结构体来处理套接字引用计数并，增加sock引用计数 */

    int fd = local_fd_install(sock, FILE_FD_SOCKET);
    return fd;
}


static int do_close(int sock)
{
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
}

int netif_close(int sock)
{
    dbgprint("netif close: sock %d\n", sock);

    /* TODO: 检测引用计数，小于等于0才进行关闭 */

    int retval = do_close(sock);
    if (retval < 0) {
        dbgprint("netif close: do close sock %d failed!\n", sock);
        return -1;
    }
    return 0;
}
