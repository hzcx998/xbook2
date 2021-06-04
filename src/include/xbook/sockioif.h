#ifndef _XBOOK_SOCKIOIF_H
#define _XBOOK_SOCKIOIF_H

#include <sys/socket.h>

struct if_nameindex
{
    unsigned int if_index;  /* 1, 2, ... */
    char *if_name;  /* null terminated name: "eth0", ... */
};

/* Standard interface flags. */
enum
{
    IFF_UP = 0x1,       /* Interface is up.*/
# define IFF_UP    IFF_UP
    IFF_BROADCAST = 0x2,    /* Broadcast address valid.*/
# define IFF_BROADCAST    IFF_BROADCAST
    IFF_DEBUG = 0x4,       /* Turn on debugging.*/
# define IFF_DEBUG    IFF_DEBUG
    IFF_LOOPBACK = 0x8,       /* Is a loopback net.*/
# define IFF_LOOPBACK    IFF_LOOPBACK
    IFF_POINTOPOINT = 0x10,    /* Interface is point-to-point link.*/
# define IFF_POINTOPOINT IFF_POINTOPOINT
    IFF_NOTRAILERS = 0x20,    /* Avoid use of trailers.*/
# define IFF_NOTRAILERS    IFF_NOTRAILERS
    IFF_RUNNING = 0x40,       /* Resources allocated.*/
# define IFF_RUNNING    IFF_RUNNING
    IFF_NOARP = 0x80,       /* No address resolution protocol.*/
# define IFF_NOARP    IFF_NOARP
    IFF_PROMISC = 0x100,    /* Receive all packets.*/
# define IFF_PROMISC    IFF_PROMISC

    /* Not supported */
    IFF_ALLMULTI = 0x200,    /* Receive all multicast packets.*/
# define IFF_ALLMULTI    IFF_ALLMULTI

    IFF_MASTER = 0x400,       /* Master of a load balancer.*/
# define IFF_MASTER    IFF_MASTER
    IFF_SLAVE = 0x800,       /* Slave of a load balancer.*/
# define IFF_SLAVE    IFF_SLAVE

    IFF_MULTICAST = 0x1000,    /* Supports multicast.*/
# define IFF_MULTICAST    IFF_MULTICAST

    IFF_PORTSEL = 0x2000,    /* Can set media type.*/
# define IFF_PORTSEL    IFF_PORTSEL
    IFF_AUTOMEDIA = 0x4000,    /* Auto media select active.*/
# define IFF_AUTOMEDIA    IFF_AUTOMEDIA
    IFF_DYNAMIC = 0x8000    /* Dialup device with changing addresses.*/
# define IFF_DYNAMIC    IFF_DYNAMIC
};


/* The ifaddr structure contains information about one address of an
 interface.They are maintained by the different address families,
 are allocated and attached when an address is set, and are linked
 together so all addresses for an interface can be located.*/

struct ifaddr
{
    struct sockaddr ifa_addr;    /* Address of interface.*/
    union
    {
        struct sockaddr    ifu_broadaddr;
        struct sockaddr    ifu_dstaddr;
    } ifa_ifu;
    struct iface *ifa_ifp;    /* Back-pointer to interface.*/
    struct ifaddr *ifa_next;    /* Next address for interface.*/
};

# define ifa_broadaddr    ifa_ifu.ifu_broadaddr    /* broadcast address    */
# define ifa_dstaddr    ifa_ifu.ifu_dstaddr    /* other end of link    */

/* Device mapping structure. I'd just gone off and designed a
 beautiful scheme using only loadable modules with arguments for
 driver options and along come the PCMCIA people 8)

 Ah well. The get() side of this is good for WDSETUP, and it'll be
 handy for debugging things. The set side is fine for now and being
 very small might be worth keeping for clean configuration.*/

struct ifmap
{
    unsigned long int mem_start;
    unsigned long int mem_end;
    unsigned short int base_addr;
    unsigned char irq;
    unsigned char dma;
    unsigned char port;
    /* 3 bytes spare */
};

/* Interface request structure used for socket ioctl's. All interface 
   ioctl's must have parameter definitions which begin with ifr_name. 
   The remainder may be interface specific. */  
struct ifreq {  
#define IFHWADDRLEN 6  
#ifndef IFNAMSIZ
#define IFNAMSIZ    16
#endif
    union {
       char ifrn_name[IFNAMSIZ]; /* Interface name, e.g. "en0". */  
    } ifr_ifrn;  
    union {  
       struct sockaddr ifru_addr;  
       struct sockaddr ifru_dstaddr;  
       struct sockaddr ifru_broadaddr;  
       struct sockaddr ifru_netmask;  
       struct sockaddr ifru_hwaddr;  
       short int ifru_flags;  
       int ifru_ivalue;  
       int ifru_mtu;  
       struct ifmap ifru_map;  
       char ifru_slave[IFNAMSIZ]; /* Just fits the size */  
       char ifru_newname[IFNAMSIZ];  
       caddr_t ifru_data;  
    } ifr_ifru;  
};
# define ifr_name ifr_ifrn.ifrn_name /* interface name */  
# define ifr_hwaddr ifr_ifru.ifru_hwaddr /* MAC address */  
# define ifr_addr ifr_ifru.ifru_addr /* address */  
# define ifr_dstaddr ifr_ifru.ifru_dstaddr /* other end of p-p lnk */  
# define ifr_broadaddr ifr_ifru.ifru_broadaddr /* broadcast address */  
# define ifr_netmask ifr_ifru.ifru_netmask /* interface net mask */  
# define ifr_flags ifr_ifru.ifru_flags /* flags */  
# define ifr_metric ifr_ifru.ifru_ivalue /* metric */  
# define ifr_mtu ifr_ifru.ifru_mtu /* mtu */  
# define ifr_map ifr_ifru.ifru_map /* device map */  
# define ifr_slave ifr_ifru.ifru_slave /* slave device */  
# define ifr_data ifr_ifru.ifru_data /* for use by interface */  
# define ifr_ifindex ifr_ifru.ifru_ivalue /* interface index */  
# define ifr_bandwidth ifr_ifru.ifru_ivalue /* link bandwidth */  
# define ifr_qlen ifr_ifru.ifru_ivalue /* queue length */  
# define ifr_newname ifr_ifru.ifru_newname /* New name */  

struct ifconf {
	int ifc_len; /* size of buffer    */
	union {
		char *ifcu_buf; /* input from user->kernel*/
		struct ifreq *ifcu_req; /* return from kernel->user*/
	} ifc_ifcu;
};

#define ifc_buf ifc_ifcu.ifcu_buf   /* buffer address */
#define ifc_req ifc_ifcu.ifcu_req   /* array of structures returned */

#endif /* _XBOOK_SOCKIOIF_H */