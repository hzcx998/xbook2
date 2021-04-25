
#include "socket_udp.h"

#include "lwip/opt.h"

#if 1//LWIP_SOCKET

#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include <string.h>
#include <stdio.h>

static void socket_udp(void *arg)
{
    int err;
  	socklen_t s;
	struct sockaddr_in local,remote;
	uint32_t ret,strlen;
    LWIP_UNUSED_ARG(arg);
	
	memset(&local, 0, sizeof(struct sockaddr_in));
	local.sin_len = sizeof(local);
	local.sin_family = AF_INET;
	local.sin_port = htons(8080);	
	local.sin_addr.s_addr = htons(INADDR_ANY);
	
	//�½�һ��NETCONN����,��Ϊ�����PCB�飬����socket���
	s = lwip_socket(AF_INET, SOCK_DGRAM, 0);
	LWIP_ASSERT("s >= 0", s >= 0);
	
	err = lwip_bind(s, (struct sockaddr*)&local, sizeof(struct sockaddr_in));
    LWIP_ASSERT("ret == 0", err == 0);
	
	printf("socket udp server init ok socket = %d\n", s);
    while (1) 
    {
       uint8_t RecvBuf[10],SendBuf[50];
       socklen_t len=sizeof(struct sockaddr_in);
       ret = lwip_recvfrom(s, (void *)RecvBuf, 20, 0, (struct sockaddr*)&remote, &len);
       
       if(ret>=3&&RecvBuf[0]=='G'&&RecvBuf[1]=='E'&&RecvBuf[2]=='T'){
          strlen = sprintf((void *)SendBuf,"Current OSTime: [%d] ms\n", sys_now());
          if(strlen>0)
             lwip_sendto(s,SendBuf,strlen,0,(struct sockaddr*)&remote,len);
       }
    }	

}

void socket_examples_init(void)
{

  sys_thread_new("socket_udp_thread", socket_udp, NULL, 0, TCPIP_THREAD_PRIO + 1);
 /* sys_thread_new("sockex_nonblocking_connect", sockex_nonblocking_connect, NULL, 0, 0);
  sys_thread_new("sockex_nonblocking_connect", sockex_nonblocking_connect, NULL, 0, 0);
  sys_thread_new("sockex_nonblocking_connect", sockex_nonblocking_connect, NULL, 0, 0);
  sys_thread_new("sockex_nonblocking_connect", sockex_nonblocking_connect, NULL, 0, 0);*/
 // sys_thread_new("sockex_testrecv", sockex_testrecv, NULL, 0, 0);
  /*sys_thread_new("sockex_testtwoselects", sockex_testtwoselects, NULL, 0, 0);*/
}

#endif /* LWIP_SOCKETS */
