#include <xbook/net.h>
#include <xbook/fsal.h>
#include <errno.h>

int netif_read(int sock, void *buf, size_t bytes)
{
    return -ENOSYS;
}

int netif_write(int sock, void *buf, size_t bytes)
{
    return -ENOSYS;
}

/* 网络接口的抽象层 */
fsal_t netif_fsal = {
    .name       = "netif",
    .list       = LIST_HEAD_INIT(netif_fsal.list),
    .subtable   = NULL,
    .close      = netif_close,
    .read       = netif_read,
    .write      = netif_write,
};
