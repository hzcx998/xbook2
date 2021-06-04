#ifndef _XBOOK_NETIF_H
#define _XBOOK_NETIF_H

#include <xbook/list.h>
#include <sys/socket.h>

#ifdef CONFIG_NET
#include <stddef.h>
#include <stdint.h>
#include <types.h>

#define DEBUG_NETIF
int netif_close(int sock);
int netif_incref(int sock);
int netif_decref(int sock);
int netif_read(int sock, void *buffer, size_t nbytes);
int netif_write(int sock, void *buffer, size_t nbytes);
int netif_ioctl(int sock, int request, void *arg);
int netif_fcntl(int sock, int cmd, long val);
int do_socket_close(int sock);
#endif

#define DEFAULT_NETIF_NAME  "en0"
#define LOOP_NETIF_NAME     "lo"
#define LOOP_MTU            65536

/** must be the maximum of all used hardware address lengths
    across all types of interfaces in use */
#define NETIF_HWADDR_LEN 6U
#define NETIF_NAME_LEN  16

/* 网络接口 */
typedef struct {
    list_t list;
    /** IP address configuration in network byte order */
    ip_addr_t ip_addr;
    ip_addr_t broad_addr;
    ip_addr_t netmask;
    ip_addr_t gateway;

    uint32_t mtu;
    /** number of bytes used in hwaddr */
    uint8_t hwaddr_len;
    /** link level hardware address of this interface */
    uint8_t hwaddr[NETIF_HWADDR_LEN];
    uint32_t flags;
    char name[NETIF_NAME_LEN];
} net_interface_t;


int network_interface_init();
void network_interface_input();

static inline net_interface_t *net_interface_create()
{
    net_interface_t *netif = mem_alloc(sizeof(net_interface_t));
    if (netif)
        memset(netif, 0, sizeof(net_interface_t));
    return netif;
}
#define net_interface_destroy(netif) mem_free(netif)

int net_interface_add(net_interface_t *netif);
int net_interface_del(net_interface_t *netif); 
net_interface_t *net_interface_find(char *name); 
net_interface_t *net_interface_first();

#endif  /* _XBOOK_NETIF_H */