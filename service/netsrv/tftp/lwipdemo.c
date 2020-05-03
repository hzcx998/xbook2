
#include <tftpserver.h>
#if TFTP_SERVER_ON == 1
#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/init.h>
#include <netif/etharp.h>
#include <lwip/timers.h>
#include <lwip/udp.h>
#include <lwip/pbuf.h>
#include <stdio.h>	

//extern functions
extern err_t ethernetif_init(struct netif *netif);
extern void ethernetif_input(struct netif *netif);

//global data
struct netif rtl8139_netif;

static void lwip_init_task(void)
{
	struct ip_addr ipaddr, netmask, gateway;

    lwip_init();
	IP4_ADDR(&gateway, 192,168,0,1);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&ipaddr, 192,168,0,105);

    netif_add(&rtl8139_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init,ethernet_input);
	netif_set_default(&rtl8139_netif);
	netif_set_up(&rtl8139_netif);
	
}

void tftp_server_demo(void *pdata)
{
    printf("tftp server starting...\n");
	//init LwIP
	lwip_init_task();

	//setup tftp server
 	tftp_server_init();
	//for periodic handle
	while(1)
	{
		ethernetif_input(&rtl8139_netif);
		
		//process LwIP timeout
		sys_check_timeouts();
		
		//todo: add your own user code here

	}
}

#endif