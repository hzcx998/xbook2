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
#include <pthread.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

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


void child_delay(int t)
{
    int i, j;
    for (i = 0; i < 100 * t; i++)
        for (j = 0; j < 1000; j++);
}

int count2 = 0;

int ticks = 0;

/*
0: none
1: spin lock
2: mutex lock
*/
#define LOCK_TYPE 3

#if LOCK_TYPE == 1
pthread_spinlock_t spinlock;
#elif LOCK_TYPE == 2
pthread_mutex_t mutexlock;
#elif LOCK_TYPE == 3
pthread_cond_t cond;
pthread_mutex_t mutex;
int testx, testy;
#endif

void *thread_a(void *arg)
{
    while (1)
    {
        child_delay(1);
#if LOCK_TYPE == 1
        pthread_spin_lock(&spinlock);
#elif LOCK_TYPE == 2
        if (pthread_mutex_lock(&mutexlock)) {
            printf("thread_a: mutex failed!\n");
            
            continue;
        }
        /*if (pthread_mutex_trylock(&mutexlock))
            continue;*/
#endif
        
        count2 = 1;
        ticks++;
        child_delay(1);
        if (count2 != 1)
            printf("thread_a: !!!!!!!!!!!!\n");
        
        //printf("thread_a: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa: %d\n", count);


#if LOCK_TYPE == 1
        pthread_spin_unlock(&spinlock);
#elif LOCK_TYPE == 2
        pthread_mutex_unlock(&mutexlock);
#endif
        
        
    }
}

void *thread_b(void *arg)
{

    while (1)
    {
        child_delay(1);
#if LOCK_TYPE == 1
        pthread_spin_lock(&spinlock);
#elif LOCK_TYPE == 2
        if (pthread_mutex_lock(&mutexlock)) {
            printf("thread_b: mutex failed!\n");
           
            continue;
        }
        /* if (pthread_mutex_trylock(&mutexlock))
            continue;*/
#endif

        
        count2 = 2;
        
        ticks++;
        child_delay(1);
        if (count2 != 2)
            printf("thread_b: @@@@@@@@@@@@\n");
        //printf("thread_b: bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb: %d\n", count);
        

#if LOCK_TYPE == 1
        pthread_spin_unlock(&spinlock);
#elif LOCK_TYPE == 2
        pthread_mutex_unlock(&mutexlock);
#endif
        
        
    }
}

void *thread_c(void *arg)
{

    while (1)
    {
        child_delay(1);
#if LOCK_TYPE == 1
        pthread_spin_lock(&spinlock);
#elif LOCK_TYPE == 2
        if (pthread_mutex_lock(&mutexlock)) {
            printf("thread_c: mutex failed!\n");
           
            continue;
        }
        /* if (pthread_mutex_trylock(&mutexlock))
            continue;*/
#endif
        count2 = 2;
        
        ticks++;
        child_delay(1);
        if (count2 != 2)
            printf("thread_c: ^^^^^^^^^^^^\n");
        //printf("thread_b: bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb: %d\n", count);

#if LOCK_TYPE == 1
        pthread_spin_unlock(&spinlock);
#elif LOCK_TYPE == 2
        pthread_mutex_unlock(&mutexlock);
#endif
        
        
    }
}


int thread_test();

int main(int argc, char *argv[])
{
    
    printf("netsrv: start.\n");
#if 0    
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
#endif
#if LOCK_TYPE == 1
    pthread_spin_init(&spinlock, 0);
#elif LOCK_TYPE == 2
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_NORMAL);
    
    if (pthread_mutex_init(&mutexlock, &mattr)) {
        printf("pthread_mutex_init failed!\n");
    }
#endif
    struct timeval tv;

    gettimeofday(&tv, NULL);
    printf("time: %u.%u\n", tv.tv_sec, tv.tv_usec);
    
    int i = 0;
    while (i++ < 0xffffff);
    
    gettimeofday(&tv, NULL);
    printf("time: %u.%u\n", tv.tv_sec, tv.tv_usec);
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    printf("time: %u.%u\n", ts.tv_sec, ts.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("time: %u.%u\n", ts.tv_sec, ts.tv_nsec);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    printf("time: %u.%u\n", ts.tv_sec, ts.tv_nsec);
    
    thread_test();
#if 0
    pthread_t thread1, thread2, thread3;
    pthread_create(&thread1, NULL, thread_a, NULL);
    pthread_create(&thread2, NULL, thread_b, NULL);
    pthread_create(&thread3, NULL, thread_c, NULL);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
#endif    
    /*
    if (pthread_mutex_trylock(&mutexlock)) {
        printf("lock failed\n");
    }else{
        printf("lock succes\n");
    }
    if (pthread_mutex_trylock(&mutexlock)) {
        printf("lock failed\n");
    }else{
        printf("lock succes\n");
    }
    //加锁几次，同样也要释放几次
    pthread_mutex_unlock(&mutexlock);
    pthread_mutex_unlock(&mutexlock);
    */

    


#if LOCK_TYPE == 1
    pthread_spin_destroy(&spinlock);
#elif LOCK_TYPE == 2
    
    //销毁互斥锁属性和互斥锁
    pthread_mutexattr_destroy(&mattr);
    pthread_mutex_destroy(&mutexlock);
    
#endif
    printf("ready exit!\n");
    return 0;
}
#if 0
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;/*初始化互斥锁*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;/*初始化条件变量*/


void *thread1(void *);
void *thread2(void *);


int i=1;


int thread_test(void)
{
pthread_t t_a;
pthread_t t_b;
pthread_create(&t_a,NULL,thread1,(void *)NULL);/*创建进程t_a*/
pthread_create(&t_b,NULL,thread2,(void *)NULL); /*创建进程t_b*/
pthread_join(t_a, NULL);/*等待进程t_a结束*/
pthread_join(t_b, NULL);/*等待进程t_b结束*/
pthread_mutex_destroy(&mutex);
pthread_cond_destroy(&cond);
printf("test done!\n");
exit(0);
}
void *thread1(void *junk)
{
for(i=1;i<=6;i++)
{
printf("djh: Line: %d, i = %d\n", __LINE__, i);
pthread_mutex_lock(&mutex);/*锁住互斥量*/
printf("thread1: lock %d\n", __LINE__);
    if(i%3==0){
    printf("thread1:signal 1 %d\n", __LINE__);
    pthread_cond_signal(&cond);/*条件改变，发送信号，通知t_b进程*/
    printf("thread1:signal 2 %d\n", __LINE__);
    printf("%s will sleep 1s in Line: %d \n", __FUNCTION__, __LINE__);
    sleep(1);

    }
pthread_mutex_unlock(&mutex);/*解锁互斥量*/
printf("thread1: unlock %d\n", __LINE__);
printf("%s will sleep 1s in Line: %d \n\n", __FUNCTION__, __LINE__);
sleep(1);

}
}


void *thread2(void *junk)
{
while(i<6)
while(i<6)
{
printf("djh: Line: %d, i = %d\n", __LINE__, i);
pthread_mutex_lock(&mutex);
printf("thread2: lock %d\n", __LINE__);
if(i%3!=0){
printf("thread2: wait 1 %d\n", __LINE__);
pthread_cond_wait(&cond,&mutex);/*解锁mutex，并等待cond改变*/
printf("thread2: wait 2 %d\n", __LINE__);
}
pthread_mutex_unlock(&mutex);
printf("thread2: unlock %d\n", __LINE__);
printf("%s will sleep 1s in Line: %d \n\n", __FUNCTION__, __LINE__);
sleep(1);
}
}
#endif 

#if 0
int sequence=1;
int eat = 1;
pthread_mutex_t productor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t my_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t productor_mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t my_cond1 = PTHREAD_COND_INITIALIZER;

void *productor1()
{
    int my_sequence = 1;

    while(1)
    {
        pthread_mutex_lock(&productor_mutex);
        while(sequence!=my_sequence)
        {
            pthread_cond_wait (&my_cond, &productor_mutex);
        }


        pthread_mutex_lock(&productor_mutex1);
        while(eat!=1)
        {
            pthread_cond_wait (&my_cond1, &productor_mutex1);
        }
        printf("A ");

        eat=0;
        pthread_cond_broadcast (&my_cond1);
        pthread_mutex_unlock (&productor_mutex1);

        sequence=2;
        pthread_cond_broadcast(&my_cond);
        pthread_mutex_unlock (&productor_mutex);
    }

    return 0;
}

void *productor2()
{
    int my_sequence=2;

    while(1)
    {
        pthread_mutex_lock(&productor_mutex);
        while(sequence!=my_sequence)
        {
            pthread_cond_wait (&my_cond, &productor_mutex);
    }

        pthread_mutex_lock(&productor_mutex1);
        while(eat!=1)
        {
        pthread_cond_wait (&my_cond1, &productor_mutex1);
        }
        printf("B ");

        eat=0;
        pthread_cond_broadcast (&my_cond1);
        pthread_mutex_unlock (&productor_mutex1);

        sequence=1;
        pthread_cond_broadcast (&my_cond);
        pthread_mutex_unlock (&productor_mutex);
    }
    return 0;
}

void *consumer()
{
    long a=200;

    while(a--)
    {
        pthread_mutex_lock(&productor_mutex1);
        while(eat!=0)
        {
            pthread_cond_wait (&my_cond1, &productor_mutex1);
        }
        printf("C ");

        eat=1;
        pthread_cond_broadcast (&my_cond1);
        pthread_mutex_unlock (&productor_mutex1);
    }

    return 0;
}

int thread_test(void)
{
    pthread_t pth1=0,pth2=0,pth3=0;
    int err=-1;
    void *tret=NULL;

    err = pthread_create(&pth1,NULL,productor1,NULL);
    if(err)
    {
        printf("creat pthread productor failed!\n");
        exit(1);
    }
    err = pthread_create(&pth2,NULL,productor2,NULL);
    if(err)
    {
        printf("creat pthread productor failed!\n");
        exit(1);
    }

    err = pthread_create(&pth3,NULL,consumer,NULL);
    if(err)
    {
        printf("creat pthread consumer failed!\n");
        exit(1);
    }

    err = pthread_join(pth3,&tret);
    if(err)
        printf("can't join pthread consumer!\n");
    else
            printf("pthread consumer exit with %d!\n",(int)tret);

    pthread_cancel(pth1);
    pthread_cancel(pth2);
    printf("main will exit\n");

    exit(0);
}
#endif

#if 0
void * Proc(void * arg)
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);

    struct timespec to;

    int i = 0;
    pthread_mutex_lock(&mutex);
    
    while (i < 5) 
    {
        to.tv_sec = time(NULL) + 3;
        to.tv_nsec = 0;
        int err = pthread_cond_timedwait(&cond, &mutex, &to);
        if (err == ETIMEDOUT) 
        {
            printf("time out %d: dispatch something...\n",i);
	     i++;
        }
        printf("count %d retval:%d\n",i, err);
    }

     pthread_mutex_unlock(&mutex);


}
int thread_test()
{    
    pthread_t pid;
    int i=0;
    printf("create thread.../n");
    pthread_create(&pid,0,Proc,0);
    pthread_join(pid,NULL);
    sleep(1);
    printf("Succeed exit!/n");
}
#endif

#if 0

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
 
#define SENDSIGTIME 10
 
pthread_cond_t g_cond;
pthread_mutex_t g_mutex;
 
void thread1(void *arg)
{
    int inArg = (int)arg;
    int ret = 0;
    struct timeval now;
    struct timespec outtime;
 
    pthread_mutex_lock(&g_mutex);
 
    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + 5;
    outtime.tv_nsec = now.tv_usec * 1000;
 
    ret = pthread_cond_timedwait(&g_cond, &g_mutex, &outtime);
    //ret = pthread_cond_wait(&g_cond, &g_mutex);
    pthread_mutex_unlock(&g_mutex);
 
    printf("thread 1 ret: %d\n", ret);
 
}
 
int thread_test(void)
{
    pthread_t id1;
    int ret;
 
    pthread_cond_init(&g_cond, NULL);
    pthread_mutex_init(&g_mutex, NULL);
 
    ret = pthread_create(&id1, NULL, (void *)thread1, (void *)1);
    if (0 != ret)
    {
	printf("thread 1 create failed!\n");
	return 1;
    }
 
    printf("wait %ds send signal!\n", SENDSIGTIME);
    sleep(SENDSIGTIME);
    printf("signal sending....\n");
    pthread_mutex_lock(&g_mutex);
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
 
 
    pthread_join(id1, NULL);
 
    pthread_cond_destroy(&g_cond);
    pthread_mutex_destroy(&g_mutex);
 
    return 0;
}
#endif

#if 1

#define MAX_THREAD_NUM 5
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
void* thread_fun(void* arg)
{
    int index = *(int*)arg;
    struct timespec abstime;
    struct timeval now;
    abstime.tv_sec = now.tv_sec + 5;
    abstime.tv_nsec = now.tv_usec * 1000;
    printf("[%d]thread start up!\n", index);
    pthread_mutex_lock(&mutex);
    printf("[%d]thread wait...\n", index);
    pthread_cond_timedwait(&cond, &mutex, &abstime);
    printf("[%d]thread wake up\n", index);
    pthread_mutex_unlock(&mutex);
    pthread_exit(0);
}
 
int thread_test()
{
    pthread_t tid[MAX_THREAD_NUM];
    int i;
    for(i = 0; i < MAX_THREAD_NUM; i++)
    {
        pthread_create(&tid[i], NULL, thread_fun, &i);
        sleep(1);
    }
    sleep(3);
    pthread_cond_broadcast(&cond);
    for(i = 0; i < MAX_THREAD_NUM; ++i)
    {
        pthread_join(tid[i], NULL);
    }
    return 0;
}
#endif