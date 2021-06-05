#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/init.h>
#include <lwip/tcpip.h>
#include <lwip/dhcp.h>
#include <lwip/sockets.h>

#include <xbook/net.h>
#include <xbook/netif.h>
#include <xbook/debug.h>
#include <xbook/fsal.h>
#include <xbook/sockioif.h>

/* lwip interface */
extern err_t ethernetif_init(struct netif *netif);
extern void ethernetif_input(struct netif *netif);
extern void httpserver_init();

/* lwip的网络接口 */
static struct netif lwip_netif;

/* 网络接口链表头 */
LIST_HEAD(netif_list_head);
DEFINE_SPIN_LOCK(netif_spin_lock);

#define CONFIG_LEVEL 0

void lwip_init_task(void)
{
    struct ip_addr ipaddr, netmask, gateway;
#if NO_SYS == 1
    lwip_init();
#else
    tcpip_init(NULL, NULL);
#endif

#if LWIP_DHCP == 1
    IP4_ADDR(&ipaddr, 0,0,0,0);
    IP4_ADDR(&gateway, 0,0,0,0);
    IP4_ADDR(&netmask, 0,0,0,0);
#else
    IP4_ADDR(&ipaddr, 192,168,0,105);
    IP4_ADDR(&gateway, 192,168,0,1);
    IP4_ADDR(&netmask, 255,255,0, 0);
#endif

#if NO_SYS == 1
    netif_add(&lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, ethernet_input);
#else    
    netif_add(&lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, tcpip_input);
#endif    
    netif_set_default(&lwip_netif);
    netif_set_up(&lwip_netif);

#if LWIP_DHCP == 1
    dhcp_start(&lwip_netif);
#endif
}

int net_interface_add(net_interface_t *netif)
{
    if (!netif)
        return -1;
    spin_lock(&netif_spin_lock);
    list_add(&netif->list, &netif_list_head);
    spin_unlock(&netif_spin_lock);
    return 0;
}

int net_interface_del(net_interface_t *netif)
{
    if (!netif)
        return -1;
    spin_lock(&netif_spin_lock);
    list_del(&netif->list);
    spin_unlock(&netif_spin_lock);
    return 0;
}

net_interface_t *net_interface_find(char *name)
{
    net_interface_t *netif;
    spin_lock(&netif_spin_lock);
    list_for_each_owner (netif, &netif_list_head, list) {
        if (!strcmp(netif->name, name)) {
            spin_unlock(&netif_spin_lock);
            return netif;
        }
    }
    spin_unlock(&netif_spin_lock);
    return NULL;
}

net_interface_t *net_interface_first()
{
    return list_first_owner_or_null(&netif_list_head, net_interface_t, list);
}

void net_interface_set_up(net_interface_t *netif)
{
    if (netif->extension) {
    #if defined(NETIF_TYPE_LWIP)
        netif_set_up((struct netif *)netif->extension);
    #endif
    }
}

void net_interface_set_down(net_interface_t *netif)
{
    if (netif->extension) {
    #if defined(NETIF_TYPE_LWIP)
        netif_set_down((struct netif *)netif->extension);
    #endif
    }
}

void net_interface_set_ip_addr(net_interface_t *netif, ip_addr_t *addr)
{
    /* copy ip to kernel */
    memcpy(&netif->ip_addr, &addr, sizeof(netif->ip_addr));
    if (netif->extension) {
    #if defined(NETIF_TYPE_LWIP)
        net_interface_set_down(netif);
        struct netif *real_netif = (struct netif *)netif->extension;
        memcpy(&real_netif->ip_addr, addr, sizeof(addr));
        net_interface_set_up(netif);
    #endif
    }
}

void net_interface_set_broad_addr(net_interface_t *netif, ip_addr_t *addr)
{
    /* copy ip to kernel */
    memcpy(&netif->broad_addr, &addr, sizeof(netif->broad_addr));
    if (netif->extension) {
    #if defined(NETIF_TYPE_LWIP)
        /* FIXME: copy broad addr */
    #endif
    }
}

void net_interface_set_netmask(net_interface_t *netif, ip_addr_t *addr)
{
    /* copy ip to kernel */
    memcpy(&netif->netmask, &addr, sizeof(netif->netmask));
    if (netif->extension) {
    #if defined(NETIF_TYPE_LWIP)
        net_interface_set_down(netif);
        struct netif *real_netif = (struct netif *)netif->extension;
        memcpy(&real_netif->netmask, addr, sizeof(addr));
        net_interface_set_up(netif);
    #endif
    }
}

void net_interface_set_flags(net_interface_t *netif, int flags)
{
    netif->flags = flags;
    if (netif->extension) {
    #if defined(NETIF_TYPE_LWIP)
        struct netif *real_netif = (struct netif *)netif->extension;
        
        if (netif->flags & IFF_UP)
            net_interface_set_up(netif);
        else
            net_interface_set_down(netif);

        if (netif->flags & IFF_BROADCAST)
            real_netif->flags |= NETIF_FLAG_BROADCAST;
        else
            real_netif->flags &= ~NETIF_FLAG_BROADCAST;

        if (netif->flags & IFF_POINTOPOINT)
            real_netif->flags |= NETIF_FLAG_POINTTOPOINT;
        else
            real_netif->flags &= ~NETIF_FLAG_POINTTOPOINT;

        if (netif->flags & IFF_NOARP)
            real_netif->flags &= ~NETIF_FLAG_ETHARP;
        else
            real_netif->flags |= NETIF_FLAG_ETHARP;

        if (netif->flags & IFF_NOARP)
            real_netif->flags &= ~NETIF_FLAG_ETHARP;
        else
            real_netif->flags |= NETIF_FLAG_ETHARP;
        
        if (netif->flags & IFF_RUNNING)
            real_netif->flags |= NETIF_FLAG_LINK_UP;
        else
            real_netif->flags &= ~NETIF_FLAG_LINK_UP;

    #endif
    }
}

void net_interface_set_hwaddr(net_interface_t *netif, char *hwaddr)
{
    memcpy(netif->hwaddr, hwaddr, netif->hwaddr_len);
    if (netif->extension) {
    #if defined(NETIF_TYPE_LWIP)
        /* FIXME: set mac addr in netcard */
    #endif
    }
}

void net_interface_dump()
{
    net_interface_t *netif;
    spin_lock(&netif_spin_lock);
    list_for_each_owner (netif, &netif_list_head, list) {
        dbgprint("====NETIF====\n");
        dbgprint("name: %s\n", netif->name);
        dbgprint("ip addr: %s\n", ipaddr_ntoa(&netif->ip_addr));
        dbgprint("broad addr: %s\n", ipaddr_ntoa(&netif->broad_addr));
        dbgprint("netmask: %s\n", ipaddr_ntoa(&netif->netmask));
        dbgprint("gateway: %s\n", ipaddr_ntoa(&netif->gateway));
        dbgprint("mtu: %d\n", netif->mtu);
        dbgprint("hardware len: %d\n", netif->hwaddr_len);
        dbgprint("hardware: ");
        int i;
        for (i = 0; i < netif->hwaddr_len; i++) {
            if (i == netif->hwaddr_len - 1)
                dbgprint("%x", netif->hwaddr[i]);
            else
                dbgprint("%x:", netif->hwaddr[i]);
        }
        dbgprint("\nflags: %x: <", netif->flags);
        if (netif->flags & IFF_UP) {
            dbgprint("UP,");
        }
        if (netif->flags & IFF_BROADCAST) {
            dbgprint("BROADCAST,");
        }
        if (netif->flags & IFF_DEBUG) {
            dbgprint("DEBUG,");
        }
        if (netif->flags & IFF_LOOPBACK) {
            dbgprint("LOOPBACK,");
        }
        if (netif->flags & IFF_POINTOPOINT) {
            dbgprint("POINTOPOINT,");
        }
        if (netif->flags & IFF_NOTRAILERS) {
            dbgprint("NOTRAILERS,");
        }
        if (netif->flags & IFF_RUNNING) {
            dbgprint("RUNNING,");
        }
        if (netif->flags & IFF_NOARP) {
            dbgprint("NOARP,");
        }
        if (netif->flags & IFF_ALLMULTI) {
            dbgprint("ALLMULTI,");
        }
        if (netif->flags & IFF_MASTER) {
            dbgprint("MASTER,");
        }
        if (netif->flags & IFF_SLAVE) {
            dbgprint("SLAVE,");
        }
        if (netif->flags & IFF_MULTICAST) {
            dbgprint("MULTICAST,");
        }
        if (netif->flags & IFF_PORTSEL) {
            dbgprint("PORTSEL,");
        }
        if (netif->flags & IFF_AUTOMEDIA) {
            dbgprint("AUTOMEDIA,");
        }
        dbgprint(">\n");

    }
    spin_unlock(&netif_spin_lock);
}

/**
 * 初始化网络接口
 */
int network_interface_init()
{
    lwip_init_task();
    /* 初始化接口 */
    list_init(&netif_list_head);
    spinlock_init(&netif_spin_lock);

    /* 创建第一个网络接口 */
    net_interface_t *netif = net_interface_create(&lwip_netif);
    assert(netif != NULL);
    /* 根据lwip_netif的flags进行设置 */
    netif->flags = 0;
    if (lwip_netif.flags & NETIF_FLAG_UP)
        netif->flags |= IFF_UP;
    if (lwip_netif.flags & NETIF_FLAG_BROADCAST)
        netif->flags |= IFF_BROADCAST;
    if (lwip_netif.flags & NETIF_FLAG_POINTTOPOINT)
        netif->flags |= IFF_POINTOPOINT;
    if (!(lwip_netif.flags & NETIF_FLAG_ETHARP))
        netif->flags |= IFF_NOARP;
    if (lwip_netif.flags & NETIF_FLAG_LINK_UP)
        netif->flags |= IFF_RUNNING;

    netif->ip_addr = lwip_netif.ip_addr;
    ipaddr_aton("255.255.255.255", &netif->broad_addr);
    netif->netmask = lwip_netif.netmask;
    netif->gateway = lwip_netif.gw;
    netif->mtu = lwip_netif.mtu;
    netif->hwaddr_len = lwip_netif.hwaddr_len;
    memcpy(netif->hwaddr, lwip_netif.hwaddr, netif->hwaddr_len);
    strcpy(netif->name, DEFAULT_NETIF_NAME);
    net_interface_add(netif);

    /* 创建loop网络接口 */
    net_interface_t *loopif = net_interface_create(NULL);
    assert(loopif != NULL);
    loopif->flags = IFF_RUNNING | IFF_UP | IFF_LOOPBACK;
    ipaddr_aton("127.0.0.1", &loopif->ip_addr);
    ipaddr_aton("255.0.0.0", &loopif->netmask);
    ipaddr_aton("0.0.0.0", &loopif->broad_addr);
    ipaddr_aton("127.0.0.1", &loopif->gateway);
    loopif->mtu = LOOP_MTU;
    loopif->hwaddr_len = NETIF_HWADDR_LEN;
    memset(loopif->hwaddr, 0, loopif->hwaddr_len);
    strcpy(loopif->name, LOOP_NETIF_NAME);
    net_interface_add(loopif);
    net_interface_dump();

    httpserver_init();
    return 0;
}

void network_interface_input()
{
    ethernetif_input(&lwip_netif);
}

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
    .select     = netif_select,
};