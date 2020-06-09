#include <srv/guisrv.h>
#include <pthread/pthread.h>
#include <guisrv.h>
#include <stdio.h>
#include <string.h>
#if 1
#define DEBUG_LOCAL 1

int do_open_display(srvarg_t *arg)
{
    printf("[guisrv] open display\n");
    arg->retval = 0;
    return 0;
}

int do_close_display(srvarg_t *arg)
{
    arg->retval = 0;
    return 0;
}

/* 调用表 */
guisrv_func_t guisrv_call_table[] = {
    do_open_display,
    do_close_display,
};

void *guisrv_echo_thread(void *arg)
{
    printf("[guisrv] ready bind service.\n");
    
    /* 绑定成为服务调用 */
    if (srvcall_bind(SRV_GUI) == -1)  {
        printf("%s: bind srvcall failed, service stopped!\n", SRV_NAME);
        return (void *) -1;
    }

    printf("[guisrv] bind service ok.\n");
    
    int seq;
    srvarg_t srvarg;
    int callnum;
    while (1)
    {
        memset(&srvarg, 0, sizeof(srvarg_t));
        /* 1.监听服务 */
        if (srvcall_listen(SRV_GUI, &srvarg)) {  
            continue;
        }

#if DEBUG_LOCAL == 1
        printf("%s: srvcall seq=%d.\n", SRV_NAME, seq);
#endif 
        /* 2.处理服务 */
        callnum = GETSRV_DATA(&srvarg, 0, int);
        if (callnum >= 0 && callnum < GUISRV_CALL_NR) {
            guisrv_call_table[callnum](&srvarg);
        }

        seq++;
        /* 3.应答服务 */
        srvcall_ack(SRV_GUI, &srvarg);   

    }
    pthread_exit((void *) -1);
    return NULL;
}

int init_guisrv_interface()
{
    /* 开一个线程来接收服务 */
    pthread_t thread_echo;
    int retval = pthread_create(&thread_echo, NULL, guisrv_echo_thread, NULL);
    if (retval == -1) 
        return -1;

    return 0;
}
#endif