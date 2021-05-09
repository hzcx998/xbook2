/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define NO_SYS                     0
#define LWIP_SOCKET               (NO_SYS==0)
#define LWIP_NETCONN              (NO_SYS==0)

#define MEM_ALIGNMENT           4

/* use os's timeval */
#define LWIP_TIMEVAL_PRIVATE 0

#define LWIP_DNS    1

#define LWIP_DHCP    1

#define TCPIP_THREAD_PRIO 0    

/* 不打开lwip的socket, connect等宏 */
#define LWIP_COMPAT_SOCKETS 0

/* 使用系统的内存分配 */
#define MEM_LIBC_MALLOC 1

#define MEMP_NUM_NETCONN 10 //能够同时激活的超时连接数目(NO_SYS==0有效)

#define MEMP_NUM_NETBUF                 (MEMP_NUM_NETCONN)

#define MEMP_NUM_UDP_PCB 10

#define LWIP_TCPIP_CORE_LOCKING 1

#define LWIP_SO_RCVTIMEO 1

/* enable loopback */
#define LWIP_NETIF_LOOPBACK 1

#define LWIP_HAVE_LOOPIF 1

#define LWIP_LOOPBACK_MAX_PBUFS 4

#endif /* __LWIPOPTS_H__ */
