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

#include <tftpserver.h>

#if 1 /* udp test */
extern err_t ethernetif_init(struct netif *netif);

extern void ethernetif_input(struct netif *netif);

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

void udp_demo_callback_new(void *arg, struct udp_pcb *upcb, struct pbuf *p, \
    struct ip_addr *addr, u16_t port)
{
    struct pbuf *q = NULL;
    const char *reply = "This is reply!\n";
    if (arg) {
        printf("%s", (char *)arg);
    }
    pbuf_free(p);
    q = pbuf_alloc(PBUF_TRANSPORT, strlen(reply)+1, PBUF_RAM);
    if (!q) {
        printf("out of PBUF_RAM\n");
        return;
    } 
    memset(q->payload, 0, q->len);
    memcpy(q->payload, reply, strlen(reply));
    udp_sendto(upcb, q,addr, port);
    pbuf_free(q);
}
static char *st_buffer="We get a data\n";
void udp_demo_init()
{
    struct udp_pcb *upcb;
    upcb = udp_new();

    udp_bind(upcb, IP_ADDR_ANY, 60000);
    udp_recv(upcb, udp_demo_callback_new, (void *)st_buffer);
}

void udp_demo_test()
{
    lwip_init_task();
    udp_demo_init();
    while (1)
    {
        ethernetif_input(&rtl8139_netif);
        sys_check_timeouts();
    }
}
#endif

int main(int argc, char *argv[])
{
    printf("netsrv: start.\n");
    
    udp_demo_test();
    return 0;
}