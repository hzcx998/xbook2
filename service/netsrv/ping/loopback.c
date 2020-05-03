/**
  ******************************************************************************
  * @file    loopback.c 
  * @author  Forrest
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A hello world example based on a Telnet connection
  *          The application works as a server which wait for the client request
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 
/* Includes ------------------------------------------------------------------*/
#include "loopback.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{

  char *data_recv = NULL;
  int index = 0;
  struct pbuf *q = NULL;

  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
	/* We call this function to tell the LwIp that we have processed the data */
	/* This lets the stack advertise a larger window, so more data can be received*/
    data_recv = (char *)mem_calloc(p->tot_len + 1, 1);
	if(NULL != data_recv)
	{
        q = p;
		while(q != NULL)
		{
		    memcpy(&data_recv[index],q->payload,q->len);
		    index += q->len;
		    q = q->next;
		}

		printf("[Server]Get MSG: %s", data_recv);
        mem_free(data_recv);
		data_recv = NULL;
	}
	else
	{
        printf("[Server]mem_calloc error, len = %u\r\n", p->tot_len);
	}
	
	tcp_recved(pcb, p->tot_len);
    pbuf_free(p);
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
err_t server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{     
  
  /* Tell LwIP to associate this structure with this connection. */
 // tcp_arg(pcb, mem_calloc(sizeof(struct name), 1));	
  
  /* Configure LwIP to use our call back functions. */
 // tcp_err(pcb, HelloWorld_conn_err);
 // tcp_setprio(pcb, TCP_PRIO_MIN);
  tcp_recv(pcb, server_recv);
 // tcp_poll(pcb, http_poll, 10);
 //  tcp_sent(pcb, http_sent);
  return ERR_OK;
}

/**
  * @brief  Initialize the hello application  
  * @param  None 
  * @retval None 
  */
 
void tcpserver_init(void)
{
  struct tcp_pcb *pcb;	            		
  
  /* Create a new TCP control block  */
  pcb = tcp_new();	                		 	

  /* Assign to the new pcb a local IP address and a port number */
  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  tcp_bind(pcb, IP_ADDR_ANY, 6060);       


  /* Set the connection to the LISTEN state */
  pcb = tcp_listen(pcb);				

  /* Specify the function to be called when a connection is established */	
  tcp_accept(pcb, server_accept);   
										
}

//unsigned char loopdata[]="Loop Interface Test!!\n";
err_t loopclient_poll(void *arg, struct tcp_pcb *tpcb)
{
    char loopdata[100] = {0,};
	static unsigned int count = 0;
    int len = 0;

	len = sprintf(loopdata, "Loop Interface Test, loop count = %u", count++);
	
    tcp_write(tpcb, loopdata, len, TCP_WRITE_FLAG_COPY);
	return ERR_OK;
}

err_t loopclient_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{

//  char *rq;

  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
	/* We call this function to tell the LwIp that we have processed the data */
	/* This lets the stack advertise a larger window, so more data can be received*/
	tcp_recved(pcb, p->tot_len);
	printf("[Client]recive data, len = %d\r\n", p->tot_len);
    pbuf_free(p);
  } 
  else if (err == ERR_OK) 
  {
    return tcp_close(pcb);
  }
  
  return ERR_OK;
}


err_t loopclient_connect(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  tcp_recv(tpcb, loopclient_recv);
  tcp_poll(tpcb, loopclient_poll, 10);
  return ERR_OK;
}

void loopclient_init(void)
{
  struct tcp_pcb *pcb = NULL;
  struct ip_addr ipaddr;
  //IP4_ADDR(&ipaddr, 192, 168, 1, 37); 
  IP4_ADDR(&ipaddr, 127, 0, 0, 1);
  /* Create a new TCP control block  */
  pcb = tcp_new();	                		 	

  /* Assign to the new pcb a local IP address and a port number */
  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  //tcp_bind(pcb, IP_ADDR_ANY, 7);       

  tcp_connect(pcb,&ipaddr,6060,loopclient_connect);
}

