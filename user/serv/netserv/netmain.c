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
#include <unistd.h>
#include <stdio.h>
#include <netserv.h>
#include <pthread.h>
#include <sys/lpc.h>
#include <net.client.h>
#include <time.h>

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
    IP4_ADDR(&gateway, 192,168,0,104);
    IP4_ADDR(&netmask, 255,255,0, 0);
#if NO_SYS == 1
    netif_add(&lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, ethernet_input);
#else    
    netif_add(&lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, tcpip_input);
#endif    
    netif_set_default(&lwip_netif);
    netif_set_up(&lwip_netif);
}

bool remote_socket(lpc_parcel_t data, lpc_parcel_t reply)
{
    int domain; lpc_parcel_read_int(data, (uint32_t *)&domain);
    int type; lpc_parcel_read_int(data, (uint32_t *)&type);
    int protocol; lpc_parcel_read_int(data, (uint32_t *)&protocol);
    int socket_id = lwip_socket(domain, type, protocol);
    if (socket_id < 0) {
        lpc_parcel_write_int(reply, -EPERM);
        return false;    
    }
    lpc_parcel_write_int(reply, socket_id);
    return true;    
}

static lpc_remote_handler_t net_remote_table[] = {
    remote_socket
};

bool netserv_echo_main(uint32_t code, lpc_parcel_t data, lpc_parcel_t reply)
{
    if (code >= FIRST_CALL_CODE && code < NETCALL_LAST_CALL)
        return net_remote_table[code - 1](data, reply);
    return false;
}

void *netserv_thread(void *arg)
{
    lpc_echo(LPC_ID_NET, netserv_echo_main);
    return NULL;
}
extern void client();
void network_init(void)
{
    printf("network start.\n");
    if (netcard_manager_init() < 0) {
        printf("init netcard driver failed!\n");
        abort();
        return;
    }
    lwip_init_task();
    httpserver_init();

    pthread_t thread;
    pthread_create(&thread, NULL, netserv_thread, NULL);
    pid_t pid = fork();
    if (pid > 0) {
        while(1) {
            ethernetif_input(&lwip_netif);
            //todo: add your own user code here
            //printf("get netpack\n");
            sched_yeild();
        }
    } else {
       client();
    }
}

