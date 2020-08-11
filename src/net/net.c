#include <xbook/netcard.h>
#include <xbook/net.h>
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

extern err_t ethernetif_init(struct netif *netif);
extern void ethernetif_input(struct netif *netif);

struct netif rtl8139_netif;
void
httpserver_init();
void socket_examples_init(void);

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
    IP4_ADDR(&ipaddr, 192,168,0,105);
    IP4_ADDR(&gateway, 192,168,0,1);
    IP4_ADDR(&netmask, 255,255,0, 0);
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
    netif_add(&rtl8139_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, ethernet_input);
#else    
    netif_add(&rtl8139_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, tcpip_input);
#endif    
    netif_set_default(&rtl8139_netif);
    netif_set_up(&rtl8139_netif);
#if CONFIG_LEVEL == 2
    printk("[%s] %s: dhcp start.\n", SRV_NAME, __func__);
    dhcp_start(&rtl8139_netif);
    printk("[%s] %s: dhcp done.\n", SRV_NAME, __func__);
#endif
}

/**
 * netin:
 * 网络输入线程，从网卡读取数据包，如果有数据就返回，没有数据就阻塞。
 * 这样可以减轻CPU负担。
 */
void netin_kthread(void *arg) 
{
    printk("[NETIN]: init start.\n");
#if 1    
    lwip_init_task();

    httpserver_init();
    //socket_examples_init();
    while(1) {
        /* 检测输入，如果没有收到数据就会阻塞。 */
        ethernetif_input(&rtl8139_netif);
		//todo: add your own user code here
	}
#endif
}

/* 网络初始化 */
void init_net(void)
{
    printk("[NET]: init start.\n");
    if (init_netcard_driver() < 0)
        pr_err("init netcard driver failed!\n");

    /* 打开一个线程来读取网络数据包 */
    task_t * netin = kthread_start("netin", TASK_PRIO_RT, netin_kthread, NULL);
    if (netin == NULL) {
        pr_err("[NET]: start kthread netin failed!\n");
    }
}

#if LWIP_NETCONN

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG         LWIP_DBG_OFF
#endif

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_index_html[] = "<html><head><title>Congrats!</title></head><body><h1>Welcome to LwIP 1.4.1 HTTP server!</h1> \
                                                        <center><p>This is a test page based on netconn API.</center></body></html>";

/** Serve one HTTP connection accepted in the http thread */
static void
httpserver_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  
  /* Read the data from the port, blocking if nothing yet there. 
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);

  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);

    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' ) {
      /* Send the HTML header 
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */
      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_COPY);
      
      /* Send our HTML page */
      netconn_write(conn, http_index_html, sizeof(http_index_html)-1, NETCONN_COPY);

      mdelay(10);
    }
  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);
  
  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

/** The main function, never returns! */
static void
httpserver_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);
  
  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);
  LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);

  err = netconn_bind(conn, NULL, 80);

  /* Bind to port 80 (HTTP) with default IP address */
  if (err != ERR_OK) {
      pr_err("netconn_bind error %d!\n", err);
      spin("netconn_bind");
  }
  
  /* Put the connection into LISTEN state */
  if (netconn_listen(conn) != ERR_OK) {
      pr_err("netconn_listen error!\n");
      spin("netconn_listen");
  }
  
  do {
    err = netconn_accept(conn, &newconn);
    if (err == ERR_OK) {
      
      httpserver_serve(newconn);
      netconn_delete(newconn);
    }
  } while(err == ERR_OK);
  LWIP_DEBUGF(HTTPD_DEBUG,
    ("http_server_netconn_thread: netconn_accept received error %d, shutting down",
    err));
  netconn_close(conn);
  netconn_delete(conn);
}

/** Initialize the HTTP server (start its thread) */
void
httpserver_init()
{
  sys_thread_new("http_server_netconn", httpserver_thread, NULL, DEFAULT_THREAD_STACKSIZE, TASK_PRIO_USER);
}
#endif


#if LWIP_SOCKET

#include "lwip/sockets.h"
#include "lwip/sys.h"

#include <string.h>
#include <stdio.h>

#ifndef SOCK_TARGET_HOST
#define SOCK_TARGET_HOST  "192.168.0.104"
#endif

#ifndef SOCK_TARGET_PORT
#define SOCK_TARGET_PORT  8080
#endif
char zrxbuf[1024];

/** This is an example function that tests
    blocking-connect and nonblocking--recv-write . */
static void socket_nonblocking(void *arg)
{
  int s;
  int ret;
  u32_t opt;
  struct sockaddr_in addr;
  int err;

  LWIP_UNUSED_ARG(arg);
  /* set up address to connect to */
  memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = PP_HTONS(SOCK_TARGET_PORT);
  addr.sin_addr.s_addr = inet_addr(SOCK_TARGET_HOST);
  
  /* create the socket */
  //s = socket(AF_INET, SOCK_STREAM, 0);
  //LWIP_ASSERT("s >= 0", s >= 0);

  /* connect */
  do
  {
      s = lwip_socket(AF_INET, SOCK_STREAM, 0);
      LWIP_ASSERT("s >= 0", s >= 0);
      ret = lwip_connect(s, (struct sockaddr*)&addr, sizeof(addr));
	  printk("socket connect result [%d]\n", ret);
	  if(ret != 0)
      {
         lwip_close(s);
	  }
  }while(ret != 0);
  
  /* should have an error: "inprogress" */
  if(ret != 0)
  {
     ret = lwip_close(s);
     while(1)
     {
         printk("socket connect error\n");
		 sys_msleep(1000);
     }
  }
  
  /* nonblocking */
  opt = 1;
  ret = lwip_ioctl(s, FIONBIO, &opt);
  LWIP_ASSERT("ret == 0", ret == 0);

  /* write should fail, too */
  while(1)
  {
        ret = lwip_read(s, zrxbuf, 1024);
        if (ret > 0) {
        /* should return 0: closed */
        printk("socket recv a data\n");        
		}
        printk("socket recv [%d]\n", ret);
		
        ret = lwip_write(s, "test", 4);
        if(ret>0)
        {
            printk("socket send %d data\n",ret);
		}
		else
		{
            ret = lwip_close(s);
			printk("socket closed %d\n",ret);
			while(1) sys_msleep(1000);
		}

		sys_msleep(1000);
		
  }
}

/** This is an example function that tests
    the recv function (timeout etc.). */
char rxbuf[1024];
char sndbuf[64];
static void socket_timeoutrecv(void *arg)
{
  int s;
  int ret;
  int opt;
  struct sockaddr_in addr;
  size_t len;

  LWIP_UNUSED_ARG(arg);
  /* set up address to connect to */
  memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = PP_HTONS(SOCK_TARGET_PORT);
  addr.sin_addr.s_addr = inet_addr(SOCK_TARGET_HOST);

  /* first try blocking: */

  /* create the socket */
  //s = socket(AF_INET, SOCK_STREAM, 0);
  //LWIP_ASSERT("s >= 0", s >= 0);

  /* connect */
  do
  {
      s = lwip_socket(AF_INET, SOCK_STREAM, 0);
      LWIP_ASSERT("s >= 0", s >= 0);
      ret = lwip_connect(s, (struct sockaddr*)&addr, sizeof(addr));
	  printk("socket connect result [%d]\n", ret);
	  if(ret != 0)
      {
         lwip_close(s);
	  }
  }while(ret != 0);
  /* should succeed */
  if(ret != 0)
  {
     printk("socket connect error %d\n", ret);
     ret = lwip_close(s);
     while(1) sys_msleep(1000);
  }

  /* set recv timeout (100 ms) */
  opt = 100;
  ret = lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(int));

  while(1)
  {
      len = 0;
      ret = lwip_recv(s, zrxbuf, 1024, 0);
      if (ret > 0) {
          /* should return 0: closed */
          //printk("socket recv a data\n"); 
          len = ret;
      }
      printk("read [%d] data\n", ret); 

	  len = sprintf(sndbuf,"Client:I receive [%d] data\n", len);
      ret = lwip_send(s, sndbuf, len, 0);
      if(ret>0)
      {
          printk("socket send %d data\n",ret);
	  }
	  else
	  {
          ret = lwip_close(s);
	      printk("socket closed %d\n",ret);
		  while(1) sys_msleep(1000);
	  }

	  //sys_msleep(1000);
		
  }
}

void socket_examples_init(void)
{
  //sys_thread_new("socket_nonblocking", socket_nonblocking, NULL, 0, TCPIP_THREAD_PRIO+2);
  sys_thread_new("socket_timeoutrecv", socket_timeoutrecv, NULL, 0, TCPIP_THREAD_PRIO);
}
#endif