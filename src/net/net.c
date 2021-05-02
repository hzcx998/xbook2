#include <xbook/net.h>
#include <xbook/fsal.h>
#include <errno.h>

/* 网络接口的抽象层 */
fsal_t netif_fsal = {
    .name       = "netif",
    .list       = LIST_HEAD_INIT(netif_fsal.list),
    .subtable   = NULL,
    .close      = netif_close,
    .read       = netif_read,
    .write      = netif_write,
    .ioctl      = netif_ioctl,
    .fcntl      = netif_fcntl,
    .incref     = netif_incref,
    .decref     = netif_decref,
};
