#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/init.h>
#include <netif/etharp.h>
#include <lwip/timers.h>
#include <lwip/udp.h>
#include <lwip/tcpip.h>

#include <tftpserver.h>
#include <ping.h>
#include <loopback.h>
#include <pthread.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/srvcall.h>
#include <time.h>
#include <errno.h>
#include <sys/res.h>
#include <unistd.h>
#include <math.h>
#include <http.h>
#include <netsrv.h>

#if 1
extern err_t ethernetif_init(struct netif *netif);

struct netif rtl8139_netif;

void lwip_init_task(void)
{
    struct ip_addr ipaddr, netmask, gateway;
    lwip_init();

    IP4_ADDR(&gateway, 192,168,0,1);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&ipaddr, 192,168,0,105);
    
    netif_add(&rtl8139_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, ethernet_input);
    netif_set_default(&rtl8139_netif);
    netif_set_up(&rtl8139_netif);
}
#endif

int main(int argc, char *argv[])
{
    printf("%s: started.\n", SRV_NAME);
    http_lwip_demo(NULL);
    return 0;
}
