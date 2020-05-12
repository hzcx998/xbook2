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

/**
 * 系统调用是用户态陷入内核，让内核帮忙执行一些内容。
 * 服务调用是用户态转移到用户态，让服务进程帮忙执行一些内容。
 * srvcall, 传入参数，然后先陷入内核，然后寻找到对应的服务进程。
 * 查看服务进程的状态，如果可转移，那么就直接转移，不然，就等待。
 * 
 * 服务进程，先通过系统调用，进入到可服务状态，等待其它进程跳转服务。
 * 先查看就绪队列，如果有就从里面挑选一个出来，并执行。然后继续陷入
 * 内核挑选，直到就绪队列为空，然后就进入阻塞等待状态。
 * 
 * 服务进程：处于可转移状态，当第一个进程跳转的时候，将参数传递给它
 * 然后唤醒它。然后阻塞调用进程。
 * 处于不可转移状态，说明正在处理一个转移，那么，就需要把当前进程挂到
 * 服务进程的就绪队列，然后阻塞自己，等待服务进程处理完服务后并解除
 * 阻塞。
 * 
 * srvcall0(int, opt, a, b, c);
 * srvcall1(long, a, b, c);
 * 
 * srvlisten(srvmsg); 监听服务
 * ...处理数据...
 * 如果涉及到缓冲区数据操作，就把对应进程的地址
 * 映射到服务进程中，直接进行数据的读写。
 * ...处理完毕...
 * srvecho(srvmsg);   应答服务
 * 
 * fopen(string, arg)
 * 
 */



/* 当前服务的名字 */
#define SRV_NAME    "netsrv"

#if 1
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
#endif
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

#if 1 /* http */
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
        printf("%s: http serve send data.\n", SRV_NAME);
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

void *server_test(void *arg)
{
    srvarg_t srvarg;
    char *path = "0:/abc";
	while(1)
	{
        srvarg.data[0] = FILESRV_OPEN;
        srvarg.data[1] = (unsigned long) path;
        srvarg.size[0] = 0;
        srvarg.size[1] = strlen(path) + 1;
        srvcall(SRV_FS, &srvarg);
	}

    return NULL;
}

void *server_test2(void *arg)
{
    srvarg_t srvarg;
    char buf[32];
	while(1)
	{
        srvarg.data[0] = FILESRV_READ;
        srvarg.size[0] = 0;
        srvarg.data[1] = (unsigned long) buf;
        srvarg.size[1] = 32;
        srvarg.data[2] = (unsigned long) "hello read.";
        srvarg.size[2] = 32;
        srvarg.io |= 1 << 1;
        srvcall(SRV_FS, &srvarg);
        printf("%s: buf %s retval %d\n", __func__, buf, srvarg.retval);

	}
    return NULL;
}

void http_lwip_demo(void *pdata)
{
	//init LwIP
	lwip_init_task();

	//setup echo server
 	echo_client_init();
    http_server_init();
	//for periodic handle
    printf("%s: ready file .\n", SRV_NAME);

    int fd = open("1:/filsrv", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }
    printf("open file fd %d.\n", fd);

    fd = open("1:/filsrv2", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }
    printf("open file fd %d.\n", fd);

    if (close(fd)) {
        printf("close fd %d failed!\n", fd);
    }
    
    if (close(0)) {
        printf("close fd %d failed!\n", 0);
    }

    fd = open("1:/filsrv3", O_CREAT | O_RDWR);
    if (fd < 0) {
        printf("open file failed!\n");
    }

    char *test_buf = malloc(1025);
    memset(test_buf, 'a', 1025);
    
    int wb = write(fd, test_buf, 1025);
    printf("write bytes %d\n", wb);
    
    int pos = lseek(fd, -1024, SEEK_END);
    printf("seek %d\n", pos);

    memset(test_buf, 0, 1025);
    
    int rb = read(fd, test_buf, 1025);
    printf("rb bytes %d\n", rb);
    
    while (1)
    {
        /* code */
    }
    

    pthread_t thread1;
    pthread_create(&thread1, NULL, server_test, NULL);
    pthread_t thread2;
    pthread_create(&thread2, NULL, server_test2, NULL);
    
    srvarg_t srvarg;
    while (1)
    {
        srvarg.data[0] = FILESRV_CLOSE;
        srvarg.size[0] = 0;
        srvcall(SRV_FS, &srvarg);

    }

	while(1)
	{
        /* 检测输入，并进行超时检测 */
        //ethernetif_input(&rtl8139_netif);
        //sys_check_timeouts();
		
		//todo: add your own user code here

	}
}

#endif 

#include <ping.h>

int main(int argc, char *argv[])
{
    printf("%s: started.\n", SRV_NAME);
    http_lwip_demo(NULL);
    return 0;
}