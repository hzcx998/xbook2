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
#include <core/netsrv.h>
#include <lwip/dhcp.h>
#include <lwipopts.h>

#include <drivers/netcard.h>

extern err_t ethernetif_init(struct netif *netif);
extern void ethernetif_input(struct netif *netif);
void tcpecho_init(void);

void
httpserver_init();
void
udpecho_init(void);
void socket_examples_init2(void);
void chargen_init(void);
void dns_netconn_init();
void dhcp_netconn_init();

void socket_examples_init(void);

struct netif rtl8139_netif;

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
    IP4_ADDR(&netmask, 255,255,255, 0);
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
    printf("[%s] %s: dhcp start.\n", SRV_NAME, __func__);
    dhcp_start(&rtl8139_netif);
    printf("[%s] %s: dhcp done.\n", SRV_NAME, __func__);
    
#endif
}

/*
Net Service:
+------------------------+
| 网络服务接口            |
| 网络服务环境            |
\                        /
+------------------------+
| LWIP                   |
\                        /
+------------------------+
| 驱动                   |
+------------------------+
*/


int lwip_init_done();

int main(int argc, char *argv[])
{
    /* 初始化驱动 */
    if (init_netcard_driver() < 0) {
        srvprint("init netcard failed!\n");
        return -1;
    }
#if 0
    int ret = drv_netcard.open(0);
    if (ret < 0) {
        srvprint("open netcard 0 failed!\n");
        return -1;
    }
    while (1)
    {
        drv_netcard.write(0, "test!", 5);
        sleep(1);
    }
#endif    
    //printf("%s: started.\n", SRV_NAME);
    //init LwIP
	lwip_init_task();

#if NO_SYS == 0
    //tcpecho_init();
#endif    
    //dhcp_netconn_init();

    //socket_examples_init();
    //httpserver_init();
    //udpecho_init();
    //socket_examples_init();
    //chargen_init();
    // setup echo server
 	//echo_client_init();
    //http_server_init();
	
    //ping_init();
	while(1)
	{
        /* 检测输入，并进行超时检测 */
        ethernetif_input(&rtl8139_netif);
#if NO_SYS == 1
        sys_check_timeouts();
#endif
		//todo: add your own user code here
	}
    return 0;
}

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"
/*-----------------------------------------------------------------------------------*/
static void 
tcpecho_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* Create a new connection identifier. */
  conn = netconn_new(NETCONN_TCP);
  if (conn == NULL)
    printf("netconn new failed!\n");    
  /* Bind connection to well known port number 7. */
  err = netconn_bind(conn, NULL, 7);
  if (err != ERR_OK)
    printf("netconn bind failed!\n");   
  /* Tell connection to go into listening mode. */
  err = netconn_listen(conn);
  if (err != ERR_OK)
    printf("netconn listen failed!\n");  
  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (err == ERR_OK) {
      struct netbuf *buf;
      void *data;
      u16_t len;
      
      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
        /*printf("Recved\n");*/
        do {
             netbuf_data(buf, &data, &len);
             err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 1
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
      }
      /*printf("Got EOF, looping\n");*/ 
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
tcpecho_init(void)
{
  sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, TCPIP_THREAD_PRIO + 1);
}

/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */

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
      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
      
      /* Send our HTML page */
      netconn_write(conn, http_index_html, sizeof(http_index_html)-1, NETCONN_NOCOPY);
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
  
  /* Bind to port 80 (HTTP) with default IP address */
  netconn_bind(conn, NULL, 80);
  
  /* Put the connection into LISTEN state */
  netconn_listen(conn);
  
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
  sys_thread_new("http_server_netconn", httpserver_thread, NULL, DEFAULT_THREAD_STACKSIZE, TCPIP_THREAD_PRIO + 1);
}

#endif /* LWIP_NETCONN*/


#if LWIP_NETCONN

#include "lwip/api.h"
#include "lwip/sys.h"

/*-----------------------------------------------------------------------------------*/
//static char buffer[4096];
static void
udpecho_thread(void *arg)
{
  static struct netconn *conn;
  static struct netbuf *buf;
  static ip_addr_t *addr;
  static unsigned short port;

  err_t err;
  LWIP_UNUSED_ARG(arg);

  conn = netconn_new(NETCONN_UDP);
  LWIP_ASSERT("con != NULL", conn != NULL);
  netconn_bind(conn, NULL, 7);

  while (1) {
    err = netconn_recv(conn, &buf);
    if (err == ERR_OK) {
      addr = netbuf_fromaddr(buf);
      port = netbuf_fromport(buf);

	  err = netconn_send(conn, buf);
      if(err != ERR_OK) {
          LWIP_DEBUGF(LWIP_DBG_ON, ("netconn_send failed: %d\n", (int)err));
      }
      
      netbuf_delete(buf);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
void
udpecho_init(void)
{
  sys_thread_new("udpecho_thread", udpecho_thread, NULL, DEFAULT_THREAD_STACKSIZE, TCPIP_THREAD_PRIO+1);
}

#endif /* LWIP_NETCONN */

#if LWIP_SOCKET

#include "lwip/sockets.h"
#include "lwip/sys.h"

#include <string.h>
#include <stdio.h>

#ifndef SOCK_TARGET_HOST
#define SOCK_TARGET_HOST  "10.253.190.124"
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
      s = socket(AF_INET, SOCK_STREAM, 0);
      LWIP_ASSERT("s >= 0", s >= 0);
      ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
	  printf("socket connect result [%d]\n", ret);
	  if(ret != 0)
      {
         close(s);
	  }
  }while(ret != 0);
  
  /* should have an error: "inprogress" */
  if(ret != 0)
  {
     ret = close(s);
     while(1)
     {
         printf("socket connect error\n");
		 sys_msleep(1000);
     }
  }
  
  /* nonblocking */
  opt = 1;
  ret = ioctlsocket(s, FIONBIO, &opt);
  LWIP_ASSERT("ret == 0", ret == 0);

  /* write should fail, too */
  while(1)
  {
        ret = read(s, zrxbuf, 1024);
        if (ret > 0) {
        /* should return 0: closed */
        printf("socket recv a data\n");        
		}
        printf("socket recv [%d]\n", ret);
		
        ret = write(s, "test", 4);
        if(ret>0)
        {
            printf("socket send %d data\n",ret);
		}
		else
		{
            ret = close(s);
			printf("socket closed %d\n",ret);
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
      s = socket(AF_INET, SOCK_STREAM, 0);
      LWIP_ASSERT("s >= 0", s >= 0);
      ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
	  printf("socket connect result [%d]\n", ret);
	  if(ret != 0)
      {
         close(s);
	  }
  }while(ret != 0);
  /* should succeed */
  if(ret != 0)
  {
     printf("socket connect error %d\n", ret);
     ret = close(s);
     while(1) sys_msleep(1000);
  }

  /* set recv timeout (100 ms) */
  opt = 100;
  ret = lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(int));

  while(1)
  {
      len = 0;
      ret = read(s, zrxbuf, 1024);
      if (ret > 0) {
          /* should return 0: closed */
          //printf("socket recv a data\n"); 
          len = ret;
      }
      printf("read [%d] data\n", ret); 

	  len = sprintf(sndbuf,"Client:I receive [%d] data\n", len);
      ret = write(s, sndbuf, len);
      if(ret>0)
      {
          printf("socket send %d data\n",ret);
	  }
	  else
	  {
          ret = close(s);
	      printf("socket closed %d\n",ret);
		  while(1) sys_msleep(1000);
	  }

	  //sys_msleep(1000);
		
  }
}

void socket_examples_init2(void)
{
  //sys_thread_new("socket_nonblocking", socket_nonblocking, NULL, 0, TCPIP_THREAD_PRIO+2);
  sys_thread_new("socket_timeoutrecv", socket_timeoutrecv, NULL, 0, TCPIP_THREAD_PRIO+1);
}

#endif /* LWIP_SOCKETS */

#if LWIP_SOCKET

#define MAX_SERV                 5         /* Maximum number of chargen services. Don't need too many */
#define CHARGEN_THREAD_NAME      "chargen"
#define CHARGEN_PRIORITY         254       /* Really low priority */
#define CHARGEN_THREAD_STACKSIZE 0
#define SEND_SIZE TCP_SNDLOWAT             /* If we only send this much, then when select
                                              says we can send, we know we won't block */
struct charcb 
{
    struct charcb *next;
    int socket;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    char nextchar;
};

static struct charcb *charcb_list = 0;
static int do_read(struct charcb *p_charcb);
static void close_chargen(struct charcb *p_charcb);

/**************************************************************
 * void chargen_thread(void *arg)
 *
 * chargen task. This server will wait for connections on well
 * known TCP port number: 19. For every connection, the server will
 * write as much data as possible to the tcp port.
 **************************************************************/
static void chargen_thread(void *arg)
{
    int listenfd;
    struct sockaddr_in chargen_saddr;
    fd_set readset;
    fd_set writeset;
    int i, maxfdp1;
    struct charcb *p_charcb;
    LWIP_UNUSED_ARG(arg);

    /* First acquire our socket for listening for connections */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    LWIP_ASSERT("chargen_thread(): Socket create failed.", listenfd >= 0);
    memset(&chargen_saddr, 0, sizeof(chargen_saddr));
    chargen_saddr.sin_family = AF_INET;
    chargen_saddr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
    chargen_saddr.sin_port = htons(19);     /* Chargen server port */

    if (bind(listenfd, (struct sockaddr *) &chargen_saddr, sizeof(chargen_saddr)) == -1)
        LWIP_ASSERT("chargen_thread(): Socket bind failed.", 0);

    /* Put socket into listening mode */
    if (listen(listenfd, MAX_SERV) == -1)
        LWIP_ASSERT("chargen_thread(): Listen failed.", 0);

    
    /* Wait forever for network input: This could be connections or data */
    for (;;)
    {
        maxfdp1 = listenfd+1;

        /* Determine what sockets need to be in readset */
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_SET(listenfd, &readset);
        for (p_charcb = charcb_list; p_charcb; p_charcb = p_charcb->next)
        {
                if (maxfdp1 < p_charcb->socket + 1)
                    maxfdp1 = p_charcb->socket + 1;
                FD_SET(p_charcb->socket, &readset);
                FD_SET(p_charcb->socket, &writeset);
        }

        /* Wait for data or a new connection */
        i = select(maxfdp1, &readset, &writeset, 0, 0);
        
        if (i == 0)
            continue;
        /* At least one descriptor is ready */
        if (FD_ISSET(listenfd, &readset))
        {
            /* We have a new connection request!!! */
            /* Lets create a new control block */
            p_charcb = (struct charcb *)mem_malloc(sizeof(struct charcb));
            if (p_charcb)
            {
                p_charcb->socket = accept(listenfd,
                                        (struct sockaddr *) &p_charcb->cliaddr,
                                        &p_charcb->clilen);
                if (p_charcb->socket < 0)
                    mem_free(p_charcb);
                else
                {
                    /* Keep this tecb in our list */
                    p_charcb->next = charcb_list;
                    charcb_list = p_charcb;
                    p_charcb->nextchar = 0x21;
                }
            } else {
                /* No memory to accept connection. Just accept and then close */
                int sock;
                struct sockaddr cliaddr;
                socklen_t clilen;

                sock = accept(listenfd, &cliaddr, &clilen);
                if (sock >= 0)
                    close(sock);
            }
        }
        /* Go through list of connected clients and process data */
        for (p_charcb = charcb_list; p_charcb; p_charcb = p_charcb->next)
        {
            if (FD_ISSET(p_charcb->socket, &readset))
            {
                /* This socket is ready for reading. This could be because someone typed
                 * some characters or it could be because the socket is now closed. Try reading
                 * some data to see. */
                if (do_read(p_charcb) < 0)
                    break;
            }
            if (FD_ISSET(p_charcb->socket, &writeset))
            {
                char line[80];
                char setchar = p_charcb->nextchar;

                for( i = 0; i < 59; i++)
                {
                    line[i] = setchar;
                    if (++setchar == 0x7f)
                        setchar = 0x21;
                }
                line[i] = 0;
                strcat(line, "\n\r");
                if (write(p_charcb->socket, line, strlen(line)) < 0)
                {
                    close_chargen(p_charcb);
                    break;
                }
                if (++p_charcb->nextchar == 0x7f)
                    p_charcb->nextchar = 0x21;
            }
            
        }
    }
    
    
}

/**************************************************************
 * void close_chargen(struct charcb *p_charcb)
 *
 * Close the socket and remove this charcb from the list.
 **************************************************************/
static void close_chargen(struct charcb *p_charcb)
{
    struct charcb *p_search_charcb;

        /* Either an error or tcp connection closed on other
         * end. Close here */
        close(p_charcb->socket);
        /* Free charcb */
        if (charcb_list == p_charcb)
            charcb_list = p_charcb->next;
        else
            for (p_search_charcb = charcb_list; p_search_charcb; p_search_charcb = p_search_charcb->next)
            {
                if (p_search_charcb->next == p_charcb)
                {
                    p_search_charcb->next = p_charcb->next;
                    break;
                }
            }
        mem_free(p_charcb);
}


/**************************************************************
 * void do_read(struct charcb *p_charcb)
 *
 * Socket definitely is ready for reading. Read a buffer from the socket and
 * discard the data.  If no data is read, then the socket is closed and the
 * charcb is removed from the list and freed.
 **************************************************************/
static int do_read(struct charcb *p_charcb)
{
    char buffer[80];
    int readcount;

    /* Read some data */
    readcount = read(p_charcb->socket, &buffer, 80);
    if (readcount <= 0)
    {
        close_chargen(p_charcb);
        return -1;
    }
	printf("recv data len = %d\n", readcount);
    return 0;
}


/**************************************************************
 * void chargen_init(void)
 *
 * This function initializes the chargen service. This function
 * may only be called either before or after tasking has started.
 **************************************************************/
void chargen_init(void)
{
    sys_thread_new( CHARGEN_THREAD_NAME, chargen_thread, 0, CHARGEN_THREAD_STACKSIZE, TCPIP_THREAD_PRIO+1);
    
}

#endif /* LWIP_SOCKET */