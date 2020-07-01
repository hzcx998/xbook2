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


#define DEBUG_LOCAL 1

#define SRVBUF_256      256
#define SRVBUF_32K      32768

unsigned char *srvbuf256;
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
    srvprint("do lwip_socket.\n");

    int socket_id = lwip_socket(domain, type, protocol);
    srvprint("do lwip_socket done.\n");

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
    socklen_t len;    /* 客户端地址结构长度 */

    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        /* 获取传递来的长度 */
        SETSRV_DATA(arg, 3, &len);
        SETSRV_SIZE(arg, 3, sizeof(socklen_t));
        if (srvcall_fetch(SRV_NET, arg))
            return -1;
    }
    struct sockaddr client_addr;    /* 客户端地址 */
    srvprint("get socklen %d\n", len);
    
    int ret = lwip_accept(socket_id, &client_addr, &len);
    if (ret == -1) {
        SETSRV_RETVAL(arg, -1);
        return -1;    
    }
    struct sockaddr_in *addr = (struct sockaddr_in *) &client_addr;
    srvprint("accept ok, client ip %x port %d, addr len %d.\n", addr->sin_addr, addr->sin_port, len);

    /* 回传参数值 */
    SETSRV_DATA(arg, 2, &client_addr);
    SETSRV_SIZE(arg, 2, sizeof(struct sockaddr));
    SETSRV_DATA(arg, 3, &len);
    SETSRV_SIZE(arg, 3, sizeof(socklen_t));
    SETSRV_RETVAL(arg, ret);
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
    __nullfunc,
    __nullfunc,
    __close,
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
