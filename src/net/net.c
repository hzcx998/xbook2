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

#include <xbook/net.h>
#include <xbook/netif.h>
#include <xbook/task.h>
#include <xbook/schedule.h>
#include <xbook/fsal.h>

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


extern void httpserver_init();
/**
 * netin:
 * 网络输入线程，从网卡读取数据包，如果有数据就返回，没有数据就阻塞。
 * 这样可以减轻CPU负担。
 */
void netin_kthread(void *arg) 
{
    infoprint("[net] starting...\n");
    lwip_init_task();
    httpserver_init();
    while(1) {
        /* 检测输入，如果没有收到数据就会阻塞。 */
        ethernetif_input(&lwip_netif);
		//todo: add your own user code here
        task_yield();
	}
}

void network_init(void)
{
    if (netcard_manager_init() < 0) {
        warnprint("[net] init netcard manager driver failed!\n");
        return;
    }
    
    /* 打开一个线程来读取网络数据包 */
    task_t * netin = kern_thread_start("netin", TASK_PRIO_LEVEL_NORMAL, netin_kthread, NULL);
    if (netin == NULL) {
        errprint("[net] start kthread netin failed!\n");
    }
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
