#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/init.h>
#include <netif/etharp.h>
#include <lwip/timers.h>
#include <lwip/udp.h>
#include <lwip/tcpip.h>
#include <lwip/dhcp.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <netserv.h>
#include <pthread.h>
#include <sys/lpc.h>
#include <stdlib.h>

#include "socket_udp.h"

/* lwip interface */
extern err_t ethernetif_init(struct netif *netif);
extern void ethernetif_input(struct netif *netif);
extern void httpserver_init();

struct netif lwip_netif;

#define CONFIG_LEVEL 0

void lwip_init_task(void)
{
    struct ip_addr ipaddr, netmask, gateway;
#if NO_SYS == 1
    lwip_init();
#else
    tcpip_init(NULL, NULL);
#endif
    IP4_ADDR(&ipaddr, 192,168,0,105);
    IP4_ADDR(&gateway, 192,168,0,1);
    IP4_ADDR(&netmask, 255,255,0, 0);
#if NO_SYS == 1
    netif_add(&lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, ethernet_input);
#else    
    netif_add(&lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, tcpip_input);
#endif    
    netif_set_default(&lwip_netif);
    netif_set_up(&lwip_netif);
}

extern bool netserv_echo_main(uint32_t code, lpc_parcel_t data, lpc_parcel_t reply);

static void *netserv_thread(void *arg)
{
    printf("netserv thread: start:%d\n", pthread_self());
    lpc_echo_group(LPC_ID_NET, netserv_echo_main);
    return NULL;
}

void network_init(void)
{
    if (netcard_manager_init() < 0) {
        printf("netserv: init netcard driver failed!\n");
        abort();
        return;
    }
    lwip_init_task();
    httpserver_init();
    //socket_examples_init();

    /* 多个线程来检测端口是否有请求，然后并对请求作出应答 */
    pthread_t thread;
    pthread_create(&thread, NULL, netserv_thread, NULL);
    
    pthread_t thread1;
    pthread_create(&thread1, NULL, netserv_thread, NULL);
    
    while(1) {
        ethernetif_input(&lwip_netif);
        //todo: add your own user code here
        //printf("get netpack\n");
        sched_yield();
    }
}
