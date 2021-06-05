#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_ether.h>

static char *get_ipaddr(int sfd, const char *dev)
{
    int saved_errno = errno, ret;
    struct ifreq ifr;
    static char ipaddr[32] = {0};
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    errno = saved_errno;
    ret = ioctl(sfd, SIOCGIFADDR, &ifr);
    if (ret == -1) {
        if (errno == ENODEV) {
            fprintf(stderr, "Interface %s : No such device.\n", dev);
            exit(EXIT_FAILURE);
        }
    }
    saved_errno = errno;

    inet_ntop(AF_INET, &(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ipaddr, INET_ADDRSTRLEN);
    return ipaddr;
}

static char *get_netmask(int sfd, const char *dev)
{
    int saved_errno = errno, ret;
    struct ifreq ifr;
    static char ipaddr[32] = {0};
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    errno = saved_errno;
    ret = ioctl(sfd, SIOCGIFNETMASK, &ifr);
    if (ret == -1) {
        if (errno == ENODEV) {
            fprintf(stderr, "Interface %s : No such device.\n", dev);
            exit(EXIT_FAILURE);
        }
    }
    saved_errno = errno;

    inet_ntop(AF_INET, &(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr), ipaddr, INET_ADDRSTRLEN);
    return ipaddr;
}

static char *get_broadcast(int sfd, const char *dev)
{
    int saved_errno = errno, ret;
    struct ifreq ifr;
    static char ipaddr[32] = {0};
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    errno = saved_errno;
    ret = ioctl(sfd, SIOCGIFBRDADDR, &ifr);
    if (ret == -1) {
        if (errno == ENODEV) {
            fprintf(stderr, "Interface %s : No such device.\n", dev);
            exit(EXIT_FAILURE);
        }
    }
    saved_errno = errno;

    inet_ntop(AF_INET, &(((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr), ipaddr, INET_ADDRSTRLEN);
    return ipaddr;
}

static unsigned char *get_if_mac(int sfd, const char *dev)
{
    int ret, saved_errno;
    static unsigned char mac_addr[ETH_ALEN] = {0};
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    saved_errno = errno;
    ret = ioctl(sfd, SIOCGIFHWADDR, &ifr);
    if (ret == -1 && errno == 19) {
        fprintf(stderr, "Interface %s : No such device.\n", dev);
        exit(EXIT_FAILURE);
    }
    errno = saved_errno;

    if (ifr.ifr_addr.sa_family == ARPHRD_LOOPBACK) {
        //printf("Interface %s : A Loopback device.\n", dev);
        //printf("MAC address is always 00:00:00:00:00:00\n");
        memset(mac_addr, 0, ETH_ALEN);
        return (unsigned char *)mac_addr;
    }

    if (ifr.ifr_addr.sa_family != ARPHRD_ETHER) {
        fprintf(stderr, "Interface %s : Not an Ethernet device.\n", dev);
        exit(EXIT_FAILURE);
    }

    memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    return (unsigned char *)mac_addr;
}

static short get_if_flags(int s, char *dev)
{
    int saved_errno = errno, ret;
    short if_flags;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    saved_errno = errno;
    ret = ioctl(s, SIOCGIFFLAGS, &ifr);
    if (ret == -1 &&errno == ENODEV) {
        fprintf(stderr, "Interface %s : No such device.\n", dev);
        exit(EXIT_FAILURE);
    }
    errno = saved_errno;
    if_flags = ifr.ifr_flags;

    return if_flags;
}

static int get_if_mtu(int s, char *dev)
{
    int saved_errno = errno, ret;
    int if_mtu;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    saved_errno = errno;
    ret = ioctl(s, SIOCGIFMTU, &ifr);
    if (ret == -1 &&errno == ENODEV) {
        fprintf(stderr, "Interface %s : No such device.\n", dev);
        exit(EXIT_FAILURE);
    }
    errno = saved_errno;
    if_mtu = ifr.ifr_mtu;

    return if_mtu;
}

static void show_flags(int flags)
{
    if (flags & IFF_UP) {
        printf("UP ");
    }
    if (flags & IFF_BROADCAST) {
        printf("BROADCAST ");
    }
    if (flags & IFF_DEBUG) {
        printf("DEBUG ");
    }
    if (flags & IFF_LOOPBACK) {
        printf("LOOPBACK ");
    }
    if (flags & IFF_POINTOPOINT) {
        printf("POINTOPOINT ");
    }
    if (flags & IFF_NOTRAILERS) {
        printf("NOTRAILERS ");
    }
    if (flags & IFF_RUNNING) {
        printf("RUNNING ");
    }
    if (flags & IFF_NOARP) {
        printf("NOARP ");
    }
    if (flags & IFF_ALLMULTI) {
        printf("ALLMULTI ");
    }
    if (flags & IFF_MASTER) {
        printf("MASTER ");
    }
    if (flags & IFF_SLAVE) {
        printf("SLAVE ");
    }
    if (flags & IFF_MULTICAST) {
        printf("MULTICAST ");
    }
    if (flags & IFF_PORTSEL) {
        printf("PORTSEL ");
    }
    if (flags & IFF_AUTOMEDIA) {
        printf("AUTOMEDIA ");
    }
}

static void show_netif(char *dev)
{
    int sfd;
    short flags;
    char ifname[IFNAMSIZ] = {'\0'};
    strncpy(ifname, dev, IFNAMSIZ-1);

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    flags = get_if_flags(sfd, ifname);
    char *ipaddr = get_ipaddr(sfd, ifname);

    char *netif_type = NULL;
    if (!strcmp(ipaddr, "127.0.0.1"))
        netif_type = "Loopback";
    else
        netif_type = "Ethernet";

    unsigned char *hwaddr = get_if_mac(sfd, ifname);
    char *brdaddr = get_broadcast(sfd, ifname);
    char *netmask = get_netmask(sfd, ifname);
    int if_mtu = get_if_mtu(sfd, ifname);

    printf("%s Link encap: %s\n", ifname, netif_type);
    printf("        Mac address:");
    int i;
    for (i = 0; i < ETH_ALEN; i++) {
        if (i == ETH_ALEN - 1)
            printf("%X\n", hwaddr[i]);
        else
            printf("%X:", hwaddr[i]);
    }
    printf("        Inet address:%s Broadcast:%s Netmask:%s\n", ipaddr, brdaddr, netmask);
    printf("        ");
    show_flags(flags);
    printf("MTU:%d\n", if_mtu);
    close(sfd);
}

void show_all_netif()
{
    int sfd, if_count, i;
    struct ifconf ifc;
    struct ifreq ifr[10];
    memset(&ifc, 0, sizeof(struct ifconf));
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    ifc.ifc_len = 10 * sizeof(struct ifreq);
    ifc.ifc_buf = (char *)ifr;
    /* SIOCGIFCONF is IP specific. see netdevice(7) */
    ioctl(sfd, SIOCGIFCONF, (char *)&ifc);
    if_count = ifc.ifc_len / (sizeof(struct ifreq));
    for (i = 0; i < if_count; i++) {
        //printf("Interface %s : ", ifr[i].ifr_name);    
        show_netif(ifr[i].ifr_name);
    }
    close(sfd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	if(argc == 2){
        // list arg
        show_netif(argv[1]);
	}else{
        // list all
        show_all_netif();
	}
    exit(EXIT_SUCCESS);
}
