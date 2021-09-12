#include "test.h"

#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_ether.h>

#define MAXLINE 4096
int socket_test(int argc, char *argv[])
{
    int sockfd; 
    char sendline[4096]; 
    struct sockaddr_in servaddr; 
    
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0); 
    }

    printf("socket ID: %d\n", sockfd); 

    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port =htons(8080); 
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.104");
    printf("connecting...\n"); 
    
    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    { 
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno); 
        exit(0); 
    } 
    printf("send msg to server: \n"); 
    strcpy(sendline, "helllo!\n");
    if( send(sockfd, sendline, strlen(sendline), 0) < 0) 
    { 
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
     exit(0); 
    } 
    memset(sendline, 0, 4096);
    if (recv(sockfd, sendline, 256, 0) < 0) {
        printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0); 
    }
    printf("recv: %s\n", sendline);
    close(sockfd); 
    exit(0);
    return 0;
}

int socket_test2(int argc, char *argv[])
{
    printf("socket2 test start!\n");
        
    int err;
    int my_socke = socket(AF_INET, SOCK_STREAM, 0);
    if (my_socke < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("create socket %d\n", my_socke);
    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_in));
    myaddr.sin_addr.s_addr = htonl(0);
    myaddr.sin_port = htons(8080);
    myaddr.sin_family = AF_INET;
    
    err = bind(my_socke, (struct sockaddr *) &myaddr, sizeof(struct sockaddr));
    if (err < 0) {
        printf("socket bind failed!\n");
        return -1;
    }

    err = listen(my_socke, 5);
    if (err < 0) {
        printf("socket listen failed!\n");
        return -1;
    }
    printf("listen %d done!\n", my_socke);
    
    int client_sock;
    int count = 0;
    while (count < 3) {
        client_sock = accept(my_socke, NULL, NULL);
        printf("accept %d done!\n", client_sock);
        if (client_sock >= 0) {
            char buf[512] = {0};
            memset(buf, 0, 512);
            recv(client_sock, buf, 512, 0);
            printf("recv done %s!\n", buf);
            
            send(client_sock, buf, strlen(buf), 0);
            printf("send done!\n");

            close(client_sock);
            count++;
        }
    }
    return 0;
}

#define SERVER_IP   "192.168.0.104"
#define BUF_LEN 512
int socket_test3(int argc, char *argv[])
{
    printf("socket3 test start!\n");
    /* udp连接 */
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        printf("create socket failed!\n");
        return -1;
    }
    printf("create socket %d\n", fd);

    struct sockaddr_in serv_addr;
    
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_addr.s_addr = IPADDR_ANY;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_len = sizeof(struct sockaddr_in);
    printf("binding socket %d\n", fd);
    if (bind(fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("bind socket %d failed!\n", fd);
        return -1;
    }
    printf("binding socket %d done\n", fd);
    
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_addr.s_addr = inet_addr("192.168.0.104");
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_len = sizeof(struct sockaddr_in);
    
    struct sockaddr src;
    socklen_t srclen;
    char *sndbuf = "hello! Test!\n";
    
    while (1) {
        char buf[BUF_LEN] = {0};
        recvfrom(fd, buf, BUF_LEN, 0, &src, &srclen);
        printf("recvfrom: %s\n", buf);
        printf("recvfrom: srclen %d\n", srclen);
        
        if (!strcmp(buf, "quit")) {
            break;
        }

        printf("sendto: %s\n", sndbuf);
        if (sendto(fd, sndbuf, strlen(sndbuf), 0, (struct sockaddr *)&src, srclen) < 0)
            fprintf(stderr, "sendto: error\n");
    }
    close(fd);
    return 0;
}

int socket_test4(int argc, char *argv[])
{
    printf("socket4 test start!\n");
    
    /* 地址转换函数测试 */
    printf("htonl: %x -> %x\n", 0x1234abef, htonl(0x1234abef));
    printf("htons: %x -> %x\n", 0x1234, htons(0x1234));
    printf("ntohl: %x -> %x\n", 0x1234abef, ntohl(0x1234abef));
    printf("ntohs: %x -> %x\n", 0x1234, ntohs(0x1234));
    
    struct in_addr inp;
    inp.s_addr = inet_addr("192.168.0.1");
    printf("inet_addr: %s -> %x\n", "192.168.0.1", inp.s_addr);
    printf("inet_ntoa: %x -> %s\n", inp.s_addr, inet_ntoa(inp));

    inet_aton("127.0.0.1", &inp);
    printf("inet_aton: %s -> %x\n", "127.0.0.1", inp.s_addr);
    printf("inet_ntoa: %x -> %s\n", inp.s_addr, inet_ntoa(inp));

    
    const char *ip = "127.0.0.1";
    struct sockaddr_in address;
    address.sin_port = htons(8080);//little to big
    inet_pton(AF_INET, ip, &address.sin_addr);
    printf("inet_pton: %s -> %x\n", ip, address.sin_addr);

    char dest[100] ;
    inet_ntop(AF_INET, &address.sin_addr,dest,100);
    printf("inet_ntop: %x -> %s\n", address.sin_addr, dest);
    return 0;
}


int socket_ifconfig0(int argc, char *argv[])
{
    int sfd, if_count, i;
    struct ifconf ifc;
    struct ifreq ifr[10];
    char ipaddr[INET_ADDRSTRLEN] = {'\0'};

    memset(&ifc, 0, sizeof(struct ifconf));

    sfd = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = 10 * sizeof(struct ifreq);
    ifc.ifc_buf = (char *)ifr;

    /* SIOCGIFCONF is IP specific. see netdevice(7) */
    ioctl(sfd, SIOCGIFCONF, (char *)&ifc);

    if_count = ifc.ifc_len / (sizeof(struct ifreq));
    for (i = 0; i < if_count; i++) {
        printf("Interface %s : ", ifr[i].ifr_name);    
        inet_ntop(AF_INET, 
        &(((struct sockaddr_in *)&(ifr[i].ifr_addr))->sin_addr),
        ipaddr, INET_ADDRSTRLEN);
        printf("%s\n", ipaddr);
    }
    close(sfd);
    exit(EXIT_SUCCESS);
    return 0;
}


static char *get_ipaddr(const char *dev)
{
    int sfd, saved_errno, ret;
    struct ifreq ifr;
    char *ipaddr;

    ipaddr = (char *)malloc(INET_ADDRSTRLEN);
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    sfd = socket(AF_INET, SOCK_DGRAM, 0);

    errno = saved_errno;
    ret = ioctl(sfd, SIOCGIFADDR, &ifr);
    if (ret == -1) {
        if (errno == 19) {
            fprintf(stderr, "Interface %s : No such device.\n", dev);
            exit(EXIT_FAILURE);
        }
        if (errno == 99) {
            fprintf(stderr, "Interface %s : No IPv4 address assigned.\n", dev);
            exit(EXIT_FAILURE);
        }
    }
    saved_errno = errno;

    inet_ntop(AF_INET, &(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ipaddr, INET_ADDRSTRLEN);
    
    close(sfd);
    return ipaddr;
}

int socket_ifconfig1(int argc, char *argv[])
{
    #if 0
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [network interface name]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    #endif

    char ifname[IFNAMSIZ] = {'\0'};
    strncpy(ifname, "lo", IFNAMSIZ-1);

    char *ip = get_ipaddr(ifname);

    printf("Interface %s : %s\n", ifname, ip);
    return 0;
}

static unsigned char *get_if_mac(const char *dev)
{
    int sfd, ret, saved_errno;
    unsigned char *mac_addr;
    struct ifreq ifr;

    mac_addr = (unsigned char *)malloc(ETH_ALEN);

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
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
        printf("Interface %s : A Loopback device.\n", dev);
        printf("MAC address is always 00:00:00:00:00:00\n");
        exit(EXIT_SUCCESS);
    }

    if (ifr.ifr_addr.sa_family != ARPHRD_ETHER) {
        fprintf(stderr, "Interface %s : Not an Ethernet device.\n", dev);
        exit(EXIT_FAILURE);
    }

    memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    return (unsigned char *)mac_addr;
}

int socket_ifconfig2(int argc, char *argv[])
{
    /*
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [network interface name]\n", argv[0]);
        exit(EXIT_FAILURE);
    }*/

    char ifname[IFNAMSIZ] = {'\0'};
    strncpy(ifname, "en0", IFNAMSIZ-1);

    unsigned char *mac = get_if_mac(ifname);
    
    printf("Interface %s : %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
    ifname, *mac, *(mac+1), *(mac+2), *(mac+3), *(mac+4), *(mac+5));

    return 0;
}


static short get_if_flags(int, char *);

int socket_ifconfig3(int argc, char *argv[])
{
    /*if (argc != 2) {
        fprintf(stderr, "Usage: %s [network interface name]\n", argv[0]);
        exit(EXIT_FAILURE);
    }*/

    int sfd;
    short flags;
    char ifname[IFNAMSIZ] = {'\0'};
    strncpy(ifname, "en0", IFNAMSIZ-1);

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    flags = get_if_flags(sfd, ifname);

    printf("Interface %s : ", ifname);
    if (flags &IFF_UP)
        printf("UP ");

    if (flags &IFF_RUNNING)
        printf("RUNNING ");

    if (flags &IFF_LOOPBACK)
        printf("LOOPBACK ");

    if (flags &IFF_BROADCAST)
        printf("BROADCAST ");

    if (flags &IFF_MULTICAST)
        printf("MULTICAST ");

    if (flags &IFF_PROMISC)
        printf("PROMISC");

#ifndef IFF_LOWER_UP
#define IFF_LOWER_UP 0x10000
    if (flags &IFF_LOWER_UP)
        printf("LOWER_UP");
#endif

    printf("\n");

    close(sfd);
    exit(EXIT_SUCCESS);
}

static short get_if_flags(int s, char *dev)
{
    int saved_errno, ret;
    short if_flags;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    saved_errno = errno;
    ret = ioctl(s, SIOCGIFFLAGS, &ifr);
    if (ret == -1 &&errno == 19) {
        fprintf(stderr, "Interface %s : No such device.\n", dev);
        exit(EXIT_FAILURE);
    }
    errno = saved_errno;
    if_flags = ifr.ifr_flags;

    return if_flags;
}


static void set_ipaddr(const char *, const char *);

int socket_ifconfig4(int argc, char *argv[])
{

    /*if (argc != 3) {
        fprintf(stderr, "Usage: %s [network interface name] [ip address]\n",
        argv[0]);
        exit(EXIT_FAILURE);
    }*/
    argv[1] = "en0";
    argv[2] = "192.168.0.106";

    char ifname[IFNAMSIZ] = {'\0'};
    strncpy(ifname, argv[1], IFNAMSIZ-1);
    char ipaddr[INET_ADDRSTRLEN] = {'\0'};
    strncpy(ipaddr, argv[2], INET_ADDRSTRLEN);

    set_ipaddr(ifname, ipaddr);

    printf("Interface %s : ip address is set to %s\n", ifname, ipaddr);
    
    return 0;
}

static void set_ipaddr(const char *dev, const char *ip)
{
    int sfd, saved_errno, ret;
    struct ifreq ifr;
    struct sockaddr_in sin;

    sfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(sin.sin_addr));

    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

    errno = saved_errno;
    ret = ioctl(sfd, SIOCSIFADDR, &ifr);
    if (ret == -1) {
        if (errno == 19) {
            fprintf(stderr, "Interface %s : No such device.\n", dev);
            exit(EXIT_FAILURE);
        }
        if (errno == 99) {
            fprintf(stderr, "Interface %s : No IPv4 address assigned.\n", dev);
            exit(EXIT_FAILURE);
        }
    }
    saved_errno = errno;

    close(sfd);
}

static void set_if_flags(int, struct ifreq*);

int socket_ifconfig5(int argc, char *argv[])
{
/*
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [network interface name]\n", argv[0]);
        exit(EXIT_FAILURE);
    }*/
    argv[1] = "en0";

    int sfd;
    short flags;
    struct ifreq ifr;

    char ifname[IFNAMSIZ] = {'\0'};
    strncpy(ifname, argv[1], IFNAMSIZ-1);

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    flags = get_if_flags(sfd, &ifr);

    ifr.ifr_flags = flags;

    /* set IFF_UP if cleared */
    if ((flags & IFF_UP)) {
        ifr.ifr_flags &= ~IFF_UP;
        set_if_flags(sfd, &ifr);
        printf("Interface %s : UP unset.\n", ifname);
    }

    flags = ifr.ifr_flags;

    /* clear IFF_PROMISC if set */
    if (flags & IFF_PROMISC) {
        ifr.ifr_flags &= ~IFF_PROMISC;
        set_if_flags(sfd, &ifr);
        printf("Interface %s : PROMISC cleared.\n", ifname);
    }

    close(sfd);

    exit(EXIT_SUCCESS);
}

static void set_if_flags(int s, struct ifreq *ifr)
{
    int ret, saved_errno;
    saved_errno = errno;
    ret = ioctl(s, SIOCSIFFLAGS, ifr);
    if (ret == -1) {
        fprintf(stderr, "Interface %s : %s\n", ifr->ifr_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    errno = saved_errno;
}


static void change_ifname(char *, char *);
static void shutdown_if_up(char *);

int socket_ifconfig6(int argc, char *argv[])
{
    /*if (argc != 3) {
        fprintf(stderr, "%s [old ifname] [new ifname]\n", argv[0]);
        exit(EXIT_FAILURE);
    }*/
    argv[1] = "en0";
    argv[2] = "ena";
    
    char old_ifname[IFNAMSIZ] = {'\0'};
    strncpy(old_ifname, argv[1], IFNAMSIZ);

    char new_ifname[IFNAMSIZ] = {'\0'};
    strncpy(new_ifname, argv[2], IFNAMSIZ);

    change_ifname(old_ifname, new_ifname);
    printf("Interface name %s has been changed to %s\n", old_ifname, new_ifname);

    return 0;
}

void change_ifname(char *old_dev, char *new_dev)
{
    int sfd, ret, saved_errno;
    struct ifreq ifr;

    shutdown_if_up(old_dev);

    sfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, old_dev, IFNAMSIZ);
    strncpy(ifr.ifr_newname, new_dev, IFNAMSIZ);

    saved_errno = errno;
    ret = ioctl(sfd, SIOCSIFNAME, &ifr);
    if (ret == -1) {
        fprintf(stderr, "Interface %s : %s\n", new_dev, strerror(errno));
        exit(EXIT_FAILURE);
    }
    errno = saved_errno;
}

static void shutdown_if_up(char *dev)
{
    int sfd, ret, saved_errno;
    short flags;
    struct ifreq ifr;

    sfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    saved_errno = errno;
    ret = ioctl(sfd, SIOCGIFFLAGS, &ifr);
    if (ret == -1) {
        fprintf(stderr, "Interface %s : %s\n", dev, strerror(errno));
        exit(EXIT_FAILURE);
    }
    errno = saved_errno;

    flags = ifr.ifr_flags;
    if (flags & IFF_UP) {
        ifr.ifr_flags &= ~IFF_UP;
        saved_errno = errno;
        ret = ioctl(sfd, SIOCSIFFLAGS, &ifr);
        if (ret == -1) {
            fprintf(stderr, "Interface %s : %s\n",dev, strerror(errno));
            exit(EXIT_FAILURE);
        }
        errno = saved_errno;
    }
}

int socket_ifconfig(int argc, char *argv[])
{
    //socket_ifconfig0(argc, argv);
    //socket_ifconfig1(argc, argv);
    //socket_ifconfig2(argc, argv);
    //socket_ifconfig3(argc, argv);
    //socket_ifconfig4(argc, argv);
    //socket_ifconfig5(argc, argv);
    socket_ifconfig6(argc, argv);

    return 0;
}
