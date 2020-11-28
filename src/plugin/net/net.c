#include <plugin/net.h>
#include <xbook/clock.h>
#include <xbook/task.h>
#include <xbook/schedule.h>

#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/init.h>
#include <netif/etharp.h>
#include <lwip/timers.h>
#include <lwip/udp.h>
#include <lwip/tcpip.h>
#include <lwip/dhcp.h>

/* lwip interface */
extern err_t ethernetif_init(struct netif *netif);
extern void ethernetif_input(struct netif *netif);

struct netif lwip_netif;
void httpserver_init();

/**
 * 0： 仅虚拟机和主机通信
 * 1： 虚拟机和主机以及外网通信
 * 2： DHCP动态分配ip
*/
#define CONFIG_LEVEL 0

void lwip_init_task(void)
{
    struct ip_addr ipaddr, netmask, gateway;
#if NO_SYS == 1
    lwip_init();
#else
    tcpip_init(NULL, NULL);
#endif
    #if CONFIG_LEVEL == 0
    //IP4_ADDR(&ipaddr, 172,17,1,1);
    #if 0
    IP4_ADDR(&ipaddr, 169,254,146,177);
    IP4_ADDR(&gateway, 169,254,146,176);
    IP4_ADDR(&netmask, 255,255,0, 0);
    #else
    IP4_ADDR(&ipaddr, 192,168,0,105);
    IP4_ADDR(&gateway, 192,168,0,104);
    IP4_ADDR(&netmask, 255,255,255, 0);

    #endif

    sys_dns_setserver(1, "192.168.0.104");

    #elif CONFIG_LEVEL == 1
    IP4_ADDR(&gateway, 169,254,177,48);
    IP4_ADDR(&netmask, 255,255,0,0);
    IP4_ADDR(&ipaddr, 169,254,177,105);

    #elif CONFIG_LEVEL == 2
    IP4_ADDR(&gateway, 0,0,0,0);
    IP4_ADDR(&netmask, 0,0,0,0);
    IP4_ADDR(&ipaddr, 0,0,0,0);
    #endif
#if NO_SYS == 1
    netif_add(&lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, ethernet_input);
#else    
    netif_add(&lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, tcpip_input);
#endif    
    netif_set_default(&lwip_netif);
    netif_set_up(&lwip_netif);
#if CONFIG_LEVEL == 2
    printk("[%s] %s: dhcp start.\n", "net", __func__);
    err_t err = dhcp_start(&lwip_netif);
    
    printk("[%s] %s: dhcp done err=%d.\n", "net", __func__, err);
#endif
}

void netin_kthread(void *arg) 
{
    lwip_init_task();
    while(1) {
        ethernetif_input(&lwip_netif);
		//todo: add your own user code here
        task_yeild();
	}
}

void network_init(void)
{
    printk("network start.\n");
    if (netcard_manager_init() < 0) {
        pr_err("init netcard driver failed!\n");
        return;
    }
    task_t * netin = kern_thread_start("netin", TASK_PRIO_LEVEL_NORMAL, netin_kthread, NULL);
    if (netin == NULL) {
        pr_err("[NET]: start kthread netin failed!\n");
    }
}