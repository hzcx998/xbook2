#include <xbook/net.h>
#include <xbook/fsal.h>
#include <xbook/safety.h>
#include <string.h>
#include <errno.h>
#if 0
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
#endif
char __hostname[HOSTNAME_MAX_LEN];

int sys_sethostname(const char *name, size_t len)
{
    if (!name)
        return -EFAULT;
    if (!len)
        return -EINVAL;
    if (len >= HOSTNAME_MAX_LEN)
        return -ENAMETOOLONG;
    if (mem_copy_from_user(__hostname, (char *)name, len) < 0)
        return -EFAULT;
    return 0;
}

int sys_gethostname(char *name, size_t len)
{
    if (!name)
        return -EFAULT;
    if (!len)
        return -EINVAL;
    if (len >= HOSTNAME_MAX_LEN)
        return -ENAMETOOLONG;
    return mem_copy_to_user(name, __hostname, len);
}

void net_init()
{
    /* 设置主机名 */
    strcpy(__hostname, HOSTNAME_DEFAULT);
}