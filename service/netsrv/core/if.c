#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/res.h>
#include <sys/ipc.h>
#include <sys/proc.h>
#include <sys/srvcall.h>
#include <core/netsrv.h>
#include <srv/netsrv.h>

#include <lwip/sockets.h>

#define DEBUG_LOCAL 0

#define SRVBUF_256      256
#define SRVBUF_SOCKADDR sizeof(struct sockaddr)
#define SRVBUF_32K      32768

unsigned char *srvbuf256;
unsigned char *srvbuf_sockaddr;
unsigned char *srvbuf32k;

static int __nullfunc(srvarg_t *arg)
{
    SETSRV_RETVAL(arg, 0);
    return 0;
}

static int __socket(srvarg_t *arg)
{
    int domain      = GETSRV_DATA(arg, 1, int);
    int type        = GETSRV_DATA(arg, 2, int);
    int protocol    = GETSRV_DATA(arg, 3, int);

    int socket_id = lwip_socket(domain, type, protocol);

    if (socket_id == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;    
    }
    SETSRV_RETVAL(arg, socket_id);
    return 0;
}

static int __bind(srvarg_t *arg)
{
    int socket_id   = GETSRV_DATA(arg, 1, int);
    int addrlen     = GETSRV_SIZE(arg, 2);
    
    struct sockaddr my_addr;
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 2, &my_addr);
        SETSRV_SIZE(arg, 2, addrlen);
        if (srvcall_fetch(SRV_NET, arg))
            return -1;
    }

    int ret = lwip_bind(socket_id, &my_addr, addrlen);
    if (ret == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;    
    }
    SETSRV_RETVAL(arg, ret);
    return 0;
}

static int __connect(srvarg_t *arg)
{
    int socket_id   = GETSRV_DATA(arg, 1, int);
    int addrlen     = GETSRV_SIZE(arg, 2);
    
    struct sockaddr serv_addr;
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 2, &serv_addr);
        SETSRV_SIZE(arg, 2, addrlen);
        if (srvcall_fetch(SRV_NET, arg))
            return -1;
    }

    int ret = lwip_connect(socket_id, &serv_addr, addrlen);
    if (ret == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;    
    }
    SETSRV_RETVAL(arg, ret);
    return 0;
}

static int __listen(srvarg_t *arg)
{
    int socket_id   = GETSRV_DATA(arg, 1, int);
    int backlog     = GETSRV_DATA(arg, 2, int);
    
    int ret = lwip_listen(socket_id, backlog);
    if (ret == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;    
    }
    SETSRV_RETVAL(arg, ret);
    return 0;
}

static int __accept(srvarg_t *arg)
{
    int socket_id   = GETSRV_DATA(arg, 1, int);

    socklen_t *len = (socklen_t *) srvbuf256;
    *len = 0;
    *len   = GETSRV_DATA(arg, 4, socklen_t);
    struct sockaddr *client_addr = (struct sockaddr *) srvbuf32k;    /* 客户端地址 */
    memset(client_addr, 0, sizeof(*client_addr));
    int ret = lwip_accept(socket_id, client_addr, len);
    if (ret == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;    
    }
    /* 回传参数值 */
    SETSRV_DATA(arg, 2, client_addr);
    SETSRV_SIZE(arg, 2, sizeof(struct sockaddr));
    SETSRV_DATA(arg, 3, len);
    SETSRV_SIZE(arg, 3, sizeof(socklen_t));
    SETSRV_RETVAL(arg, ret);
    return 0;
}

static int __send(srvarg_t *arg)
{
    int socket_id   = GETSRV_DATA(arg, 1, int);
    int len         = GETSRV_SIZE(arg, 2);
    int flags       = GETSRV_DATA(arg, 3, int);
    void *buf       = (void *) srvbuf32k;

    /* 最大一次性操作32kb数据 */
    len = MIN(len, SRVBUF_32K);
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 2, buf);
        SETSRV_SIZE(arg, 2, len);
        if (srvcall_fetch(SRV_NET, arg))
            return -1;
    }
    int sndbytes = lwip_send(socket_id, buf, len, flags);
    if (sndbytes == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, sndbytes);
    return 0;
}


static int __recv(srvarg_t *arg)
{
    int socket_id   = GETSRV_DATA(arg, 1, int);
    int len         = GETSRV_SIZE(arg, 2);
    int flags       = GETSRV_DATA(arg, 3, int);
    void *buf       = (void *) srvbuf32k;

    /* 最大一次性操作32kb数据 */
    len = MIN(len, SRVBUF_32K);
    
    int recvbytes = lwip_recv(socket_id, buf, len, flags);
    if (recvbytes == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    /* 回传缓冲区 */
    SETSRV_DATA(arg, 2, buf);
    SETSRV_SIZE(arg, 2, recvbytes);
    SETSRV_RETVAL(arg, recvbytes);
    return 0;
}


static int __sendto(srvarg_t *arg)
{
    int socket_id   = GETSRV_DATA(arg, 1, int);
    int len         = GETSRV_SIZE(arg, 2);
    int flags       = GETSRV_DATA(arg, 3, int);
    void *buf       = (void *) srvbuf32k;
    struct sockaddr *to = (struct sockaddr *) srvbuf256;
    memset(to, 0, sizeof(struct sockaddr));

    int tolen       = GETSRV_SIZE(arg, 4);
    tolen           = MIN(tolen, sizeof(struct sockaddr));

    /* 最大一次性操作32kb数据 */
    len = MIN(len, SRVBUF_32K);
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 2, buf);
        SETSRV_SIZE(arg, 2, len);
        SETSRV_DATA(arg, 4, to);
        SETSRV_SIZE(arg, 4, tolen);
        if (srvcall_fetch(SRV_NET, arg))
            return -1;
    }

    srvprint("arg: %d %x %d %d %x %d\n", socket_id, buf, len, flags, to, tolen);


    int sndbytes = lwip_sendto(socket_id, buf, len, flags, to, tolen);
    if (sndbytes == -1) {
        srvprint("sendto failed!\n");
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, sndbytes);
    return 0;
}


static int __recvfrom(srvarg_t *arg)
{
    int socket_id   = GETSRV_DATA(arg, 1, int);
    int len         = GETSRV_SIZE(arg, 2);
    int flags       = GETSRV_DATA(arg, 3, int);
    void *buf       = (void *) srvbuf32k;
    /* 最大一次性操作32kb数据 */
    len = MIN(len, SRVBUF_32K);
    
    socklen_t *fromlen = (socklen_t *) srvbuf256;
    *fromlen = 0;
    *fromlen   = GETSRV_DATA(arg, 6, int);

    struct sockaddr *client_addr = (struct sockaddr *) srvbuf_sockaddr;    /* 客户端地址 */
    memset(client_addr, 0, sizeof(*client_addr));

    int recvbytes = lwip_recvfrom(socket_id, buf, len, flags, client_addr, fromlen);
    if (recvbytes == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    /* 回传缓冲区 */
    SETSRV_DATA(arg, 2, buf);
    SETSRV_SIZE(arg, 2, recvbytes);
    SETSRV_DATA(arg, 4, client_addr);
    SETSRV_SIZE(arg, 4, sizeof(*client_addr));
    SETSRV_DATA(arg, 5, fromlen);
    SETSRV_SIZE(arg, 5, sizeof(socklen_t));
    SETSRV_RETVAL(arg, recvbytes);
    return 0;
}

static int __close(srvarg_t *arg)
{
    int socket_id      = GETSRV_DATA(arg, 1, int);
    
    if (lwip_close(socket_id) < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;    
    }
    SETSRV_RETVAL(arg, 0);
    return 0;
}


/* 调用表 */
srvcall_func_t netsrv_call_table[] = {
    __socket,
    __bind,
    __connect,
    __listen,
    __accept,
    __send,
    __recv,
    __close,
    __sendto,
    __recvfrom,
};

void *netsrv_if_thread(void *arg)
{
    /* 绑定成为服务调用 */
    if (srvcall_bind(SRV_NET) == -1)  {
        srvprint("bind srvcall failed, service stopped!\n");
        return (void *) -1;
    }
#if DEBUG_LOCAL == 1
    srvprint("srvice bind success.\n");
#endif

    int seq;
    srvarg_t srvarg;
    int callnum;
    while (1)
    {
        memset(&srvarg, 0, sizeof(srvarg_t));
        /* 1.监听服务 */
        if (srvcall_listen(SRV_NET, &srvarg)) {  
            continue;
        }

        /* 2.处理服务 */
        callnum = GETSRV_DATA(&srvarg, 0, int);
#if DEBUG_LOCAL == 1
        srvprint("srvcall seq=%d callnum %d.\n", seq, callnum);
#endif 
        if (callnum >= 0 && callnum < NETSRV_CALL_NR) {
            netsrv_call_table[callnum](&srvarg);
        }
        seq++;

        /* 3.应答服务 */
        srvcall_ack(SRV_NET, &srvarg);
    }
    pthread_exit((void *) -1);
    return NULL;
}

int init_netsrv_if()
{
    
    srvbuf_sockaddr = malloc(SRVBUF_SOCKADDR);
    if (srvbuf_sockaddr == NULL) {
        return -1;
    }
    memset(srvbuf_sockaddr, 0, SRVBUF_SOCKADDR);

    srvbuf256 = malloc(SRVBUF_256);
    if (srvbuf256 == NULL) {
        return -1;
    }
    memset(srvbuf256, 0, SRVBUF_256);
    srvbuf32k = malloc(SRVBUF_32K);
    if (srvbuf32k == NULL) {
        return -1;
    }
    memset(srvbuf32k, 0, SRVBUF_32K);

    /* 开一个线程来接收服务 */
    pthread_t thread;
    int retval = pthread_create(&thread, NULL, netsrv_if_thread, NULL);
    if (retval == -1) 
        return -1;

    return 0;
}
