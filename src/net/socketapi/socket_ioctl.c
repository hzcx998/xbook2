#include <errno.h>
#include <sys/socket.h>
#include <xbook/debug.h>
#include <xbook/socketcache.h>
#include <xbook/net.h>
#include <xbook/file.h>
#include <xbook/safety.h>
#include <xbook/sockioif.h>
#include <xbook/netif.h>
#include <xbook/if_ether.h>
#include <xbook/if_arp.h>
#include <sys/ioctl.h>

#ifndef CONFIG_NETREMOTE
#include <lwip/sockets.h>
#endif

static int do_ifconf(int sock, void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifconf ifc;
    if (mem_copy_from_user(&ifc, arg, sizeof(struct ifconf)) < 0) {
        return -EINVAL;
    }
    int netif_count = ifc.ifc_len / sizeof(struct ifreq);
    if (netif_count <= 0 || !ifc.ifc_buf)
        return -EINVAL;
    struct ifreq *user_ifreq = ifc.ifc_req;
    int netif_count_return = 0;
    struct ifreq ifreq;
    net_interface_t *first_netif = net_interface_first();
    net_interface_t *cur_netif = first_netif;
    do {
        memset(ifreq.ifr_name, 0, IFNAMSIZ);
        strcpy(ifreq.ifr_name, cur_netif->name);
        struct sockaddr_in sockaddr;
        sockaddr.sin_addr.s_addr = cur_netif->ip_addr.addr;
        memcpy(&ifreq.ifr_addr, &sockaddr, sizeof(ifreq.ifr_addr));
        /* copy ifreq to user buf */
        if (mem_copy_to_user(user_ifreq, &ifreq, sizeof(struct ifreq)) < 0) {
            return -EINVAL;
        }
        cur_netif = list_next_owner(cur_netif, list);
        user_ifreq++;
        --netif_count;
        netif_count_return++;
    } while (cur_netif->list.next != &first_netif->list && netif_count > 0);
    netif_count_return *= sizeof(struct ifreq);

    struct ifconf *__ifc = (struct ifconf *)arg;
    /* copy ifconf len to user */
    if (mem_copy_to_user(&__ifc->ifc_len, &netif_count_return, sizeof(netif_count_return)) < 0) {
        return -EINVAL;
    }
    return 0;
}

static int do_get_ifaddr(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    
    struct sockaddr_in *sockaddr = (struct sockaddr_in *)&ifreq.ifr_addr;
    if (sockaddr->sin_family == AF_INET) {   /* ipv4 */
        net_interface_t *netif = net_interface_find(ifreq.ifr_name);
        if (!netif) {
            return -ENODEV;
        }
        memcpy(&sockaddr->sin_addr, &netif->ip_addr, sizeof(netif->ip_addr));
        if (mem_copy_to_user(pifreq, &ifreq, sizeof(struct ifreq)) < 0)
            return -EINVAL;
    } else {    /* ipv6 or others not support */
        return -ENOSYS;
    }
    return 0;
}

static int do_set_ifaddr(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    
    struct sockaddr_in *sockaddr = (struct sockaddr_in *)&ifreq.ifr_addr;
    if (sockaddr->sin_family == AF_INET) {   /* ipv4 */
        net_interface_t *netif = net_interface_find(ifreq.ifr_name);
        if (!netif) {
            return -ENODEV;
        }
        net_interface_set_ip_addr(netif, (ip_addr_t *)&sockaddr->sin_addr);
    } else {    /* ipv6 or others not support */
        return -ENOSYS;
    }
    return 0;
}

static int do_get_ifhwaddr(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    
    struct sockaddr *sockaddr = (struct sockaddr *)&ifreq.ifr_hwaddr;
    net_interface_t *netif = net_interface_find(ifreq.ifr_name);
    if (!netif) {
        return -ENODEV;
    }

    if (netif->flags & IFF_LOOPBACK) {
        sockaddr->sa_family = ARPHRD_LOOPBACK;
    } else {
        sockaddr->sa_family = ARPHRD_ETHER;
        memcpy(sockaddr->sa_data, netif->hwaddr, ETH_ALEN);
    }
    if (mem_copy_to_user(pifreq, &ifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    return 0;
}

static int do_set_ifhwaddr(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    
    struct sockaddr *sockaddr = (struct sockaddr *)&ifreq.ifr_hwaddr;
    net_interface_t *netif = net_interface_find(ifreq.ifr_name);
    if (!netif) {
        return -ENODEV;
    }

    if (netif->flags & IFF_LOOPBACK) {
        errprint("%s: can not set mac addr for loopback device.\n");
        return -EPERM;
    }
    net_interface_set_hwaddr(netif, sockaddr->sa_data);
    return 0;
}

static int do_get_ifflags(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    net_interface_t *netif = net_interface_find(ifreq.ifr_name);
    if (!netif) {
        return -ENODEV;
    }
    ifreq.ifr_flags = netif->flags;
    if (mem_copy_to_user(pifreq, &ifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    return 0;
}

static int do_set_ifflags(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    net_interface_t *netif = net_interface_find(ifreq.ifr_name);
    if (!netif) {
        return -ENODEV;
    }
    net_interface_set_flags(netif, ifreq.ifr_flags);
    return 0;
}

static int do_set_ifname(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    net_interface_t *netif = net_interface_find(ifreq.ifr_name);
    if (!netif) {
        return -ENODEV;
    }
    memset(netif->name, 0, IFNAMSIZ);
    strcpy(netif->name, ifreq.ifr_newname);
    return 0;
}


static int do_get_ifbrdaddr(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    
    struct sockaddr_in *sockaddr = (struct sockaddr_in *)&ifreq.ifr_broadaddr;
    if (sockaddr->sin_family == AF_INET) {   /* ipv4 */
        net_interface_t *netif = net_interface_find(ifreq.ifr_name);
        if (!netif) {
            return -ENODEV;
        }
        memcpy(&sockaddr->sin_addr, &netif->broad_addr, sizeof(netif->broad_addr));
        if (mem_copy_to_user(pifreq, &ifreq, sizeof(struct ifreq)) < 0)
            return -EINVAL;
    } else {    /* ipv6 or others not support */
        return -ENOSYS;
    }
    return 0;
}

static int do_set_ifbrdaddr(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    
    struct sockaddr_in *sockaddr = (struct sockaddr_in *)&ifreq.ifr_broadaddr;
    if (sockaddr->sin_family == AF_INET) {   /* ipv4 */
        net_interface_t *netif = net_interface_find(ifreq.ifr_name);
        if (!netif) {
            return -ENODEV;
        }
        net_interface_set_broad_addr(netif, (ip_addr_t *)&sockaddr->sin_addr);
    } else {    /* ipv6 or others not support */
        return -ENOSYS;
    }
    return 0;
}

static int do_get_ifnetmask(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    
    struct sockaddr_in *sockaddr = (struct sockaddr_in *)&ifreq.ifr_netmask;
    if (sockaddr->sin_family == AF_INET) {   /* ipv4 */
        net_interface_t *netif = net_interface_find(ifreq.ifr_name);
        if (!netif) {
            return -ENODEV;
        }
        memcpy(&sockaddr->sin_addr, &netif->netmask, sizeof(netif->netmask));
        if (mem_copy_to_user(pifreq, &ifreq, sizeof(struct ifreq)) < 0)
            return -EINVAL;
    } else {    /* ipv6 or others not support */
        return -ENOSYS;
    }
    return 0;
}

static int do_set_ifnetmask(void *arg)
{
    if (!arg)
        return -EINVAL;
    struct ifreq *pifreq = (struct ifreq *)arg;
    struct ifreq ifreq;
    if (mem_copy_from_user(&ifreq, pifreq, sizeof(struct ifreq)) < 0)
        return -EINVAL;
    
    struct sockaddr_in *sockaddr = (struct sockaddr_in *)&ifreq.ifr_netmask;
    if (sockaddr->sin_family == AF_INET) {   /* ipv4 */
        net_interface_t *netif = net_interface_find(ifreq.ifr_name);
        if (!netif) {
            return -ENODEV;
        }
        net_interface_set_netmask(netif, (ip_addr_t *)&sockaddr->sin_addr);
    } else {    /* ipv6 or others not support */
        return -ENOSYS;
    }
    return 0;
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

    switch (request) {
    case SIOCGPGRP:
    case SIOCSPGRP:
    case SIOCSARP:
    case SIOCGARP:
    case SIOCDARP:
    case SIOCADDRT:
    case SIOCDELRT:
        return -ENOSYS;
    case SIOCGIFCONF:
        return do_ifconf(sock, arg);
    case SIOCGIFADDR:
        return do_get_ifaddr(arg);
    case SIOCSIFADDR:
        return do_set_ifaddr(arg);
    case SIOCGIFHWADDR:
        return do_get_ifhwaddr(arg);
    case SIOCSIFHWADDR:
        return do_set_ifhwaddr(arg);
    case SIOCGIFFLAGS:
        return do_get_ifflags(arg);
    case SIOCSIFFLAGS:
        return do_set_ifflags(arg);
    case SIOCSIFNAME:
        return do_set_ifname(arg);
    case SIOCGIFBRDADDR:
        return do_get_ifbrdaddr(arg);
    case SIOCSIFBRDADDR:
        return do_set_ifbrdaddr(arg);
    case SIOCGIFNETMASK:
        return do_get_ifnetmask(arg);
    case SIOCSIFNETMASK:
        return do_set_ifnetmask(arg);
    default:    /* 默认状态，发送给lwip进行处理 */
        {
            char __arg[32];
            if (mem_copy_from_user(__arg, (void *)arg, 32) < 0) {
                errprint("%s: copy arg from sock %d error!\n", __func__, sock);            
                return -EINVAL;
            }
            int lwup_ret = lwip_ioctl(sock, request, arg);
            if (lwup_ret >= 0) {
                if (mem_copy_to_user((void *)arg, __arg, 32) < 0) {
                    errprint("%s: copy arg to sock %d error!\n", __func__, sock);            
                    return -EINVAL;
                }
            }
            return lwup_ret;
        }
        break;
    }
    return -NO_SYS;
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
    int retval = do_ioctl(sock, request, arg);
    if (retval < 0) {
        errprint("%s: do ioctl sock %d request %d failed with errno %d!\n", __func__, sock, request, retval);
    }
    return retval;   
}
