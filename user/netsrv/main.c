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
#include <ping.h>
#include <loopback.h>

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

#if 0 /* udp test */


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

#if 0 /* http */
#define HTTP_PORT 80
const unsigned char htmldata[] = "	\
        <html>	\
        <head><title> A LwIP WebServer !!</title></head> \
	    <center><p>A WebServer Based on LwIP v1.4.1!</center>\
	    </html>";

static void echo_client_init(void);
static void http_server_init(void);

//@@code for a echo client
//@@client will connect to server first,then send anything received out to server 
//@@client will never close connection actively


static void echo_client_conn_err(void *arg, err_t err)
{
    printf("connect error! closed by core!!\n");
	printf("try to connect to server again!!\n");
	echo_client_init();
}
static err_t echo_client_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
	/* We call this function to tell the LwIp that we have processed the data */
	/* This lets the stack advertise a larger window, so more data can be received*/
	tcp_recved(pcb, p->tot_len);
    tcp_write(pcb, p->payload, p->len, 1);
    pbuf_free(p);

  } 
  else if (err == ERR_OK) 
  {
    /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
    /* We free the allocated memory and we close the connection */
	tcp_close(pcb);
	echo_client_init();
    return ERR_OK;
  }
  return ERR_OK;
}
static err_t echo_client_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	printf("echo client send data OK! sent len = [%d]\n", len);
    return ERR_OK;
}
static err_t echo_client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    char GREETING[] = "Hi, I am a new Client!\n";

	tcp_recv(pcb, echo_client_recv);
	tcp_sent(pcb, echo_client_sent);
    
	
	 
    tcp_write(pcb, GREETING, sizeof(GREETING), 1); 
    return ERR_OK;
}

static void echo_client_init(void)
{
  struct tcp_pcb *pcb = NULL;	            		
  struct ip_addr server_ip;
  /* Create a new TCP control block  */
  pcb = tcp_new();
  IP4_ADDR(&server_ip, 192,168,0,104);
  tcp_connect(pcb, &server_ip, 21, echo_client_connected); 
  tcp_err(pcb, echo_client_conn_err); 										
}

//@@code for a http server
//@@send HTTP data to any connected client, and then close connection
static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  char *data = NULL;
  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
	/* We call this function to tell the LwIp that we have processed the data */
	/* This lets the stack advertise a larger window, so more data can be received*/
	tcp_recved(pcb, p->tot_len);

    data =  p->payload;
	if(p->len >=3 && data[0] == 'G'&& data[1] == 'E'&& data[2] == 'T')
	{
        tcp_write(pcb, htmldata, sizeof(htmldata), 1);
    }
	else
	{
	    printf("Request error\n");
	}
     pbuf_free(p);
	 tcp_close(pcb);
  } 
  else if (err == ERR_OK) 
  {
    /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
    /* We free the allocated memory and we close the connection */
    return tcp_close(pcb);
  }
  return ERR_OK;
}

/**
  * @brief  This function when the Telnet connection is established
  * @param  arg  user supplied argument 
  * @param  pcb	 the tcp_pcb which accepted the connection
  * @param  err	 error value
  * @retval ERR_OK
  */

static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{     

  tcp_recv(pcb, http_recv);
  return ERR_OK;
}
/**
  * @brief  Initialize the http application  
  * @param  None 
  * @retval None 
  */
 
static void http_server_init(void)
{
  struct tcp_pcb *pcb = NULL;	            		
  
  /* Create a new TCP control block  */
  pcb = tcp_new();	                		 	

  /* Assign to the new pcb a local IP address and a port number */
  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  tcp_bind(pcb, IP_ADDR_ANY, HTTP_PORT);       


  /* Set the connection to the LISTEN state */
  pcb = tcp_listen(pcb);				

  /* Specify the function to be called when a connection is established */	
  tcp_accept(pcb, http_accept);   
										
}

void http_lwip_demo(void *pdata)
{
	//init LwIP
	lwip_init_task();

	//setup echo server
 	echo_client_init();
    http_server_init();
	//for periodic handle
	while(1)
	{
        ethernetif_input(&rtl8139_netif);
        sys_check_timeouts();
		
		//todo: add your own user code here
	}
}

#endif 

int main(int argc, char *argv[])
{
    printf("netsrv: start.\n");
    //init LwIP
	lwip_init_task();
    //telnet_server_init();
	//setup echo server
    //tcpserver_init();

    ping_init();
    int i = 0;
	//for periodic handle
	while(1)
	{
        i++;
        if (i > 0xffff) {
            ping_send_now();
            i = 0;
        }

        ethernetif_input(&rtl8139_netif);
        sys_check_timeouts();
		
		//todo: add your own user code here
	}
    return 0;
}