#include <pthread.h>
#include <srv/guisrv.h>
#include <sys/srvcall.h>
#include <sys/res.h>
#include <sys/ipc.h>
#include <sys/proc.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include <guisrv.h>
#include <console/console.h>
#include <console/if.h>

#define DEBUG_LOCAL 0

int guisrv_msgid = -1;
int guisrv_pipeid = -1;
int guisrv_pipe_redirect = -1;

static int __connect(srvarg_t *arg)
{
    if (guisrv_msgid >= 0 || guisrv_pipeid >= 0) {
        goto connect_error;
    }
    /* 创建一个消息队列，用来发送事件给客户端 */
    int msgid = res_open("guisrv-msg", RES_IPC | IPC_MSG | IPC_CREAT | IPC_EXCL, 0);
    if (msgid < 0) {
        printf("[%s] open msg queue failed!\n", SRV_NAME);
        goto connect_error;
    }
    
    /* 打开读取管道 */
    int pipeid = res_open("guisrv-pipe", RES_IPC | IPC_PIPE | IPC_READER | IPC_CREAT | IPC_EXCL, 0);
    if (pipeid < 0) {
        printf("[%s] open read pipe failed!\n", SRV_NAME);
        goto pipe_error;
    }
    
    /* 记录消息id */
    guisrv_msgid = msgid;
    guisrv_pipeid = pipeid;
    
    SETSRV_RETVAL(arg, 0);  /* 成功 */
    return 0;
pipe_error:
    res_close(msgid);
connect_error:
    SETSRV_RETVAL(arg, -1);  /* 失败 */
    return -1;
}

static int __close(srvarg_t *arg)
{
    if (guisrv_msgid < 0 || guisrv_pipeid < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    /* 删除消息队列 */
    res_ioctl(guisrv_msgid, IPC_DEL, 0);
    res_close(guisrv_msgid);
    guisrv_msgid = -1;

    /* 删除管道 */
    res_close(guisrv_pipeid);
    guisrv_pipeid = -1;

    SETSRV_RETVAL(arg, 0);
    return 0;
}

static int __clear(srvarg_t *arg)
{
    if (guisrv_msgid < 0 || guisrv_pipeid < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    screen.clear();

    SETSRV_RETVAL(arg, 0);
    return 0;
}

int guisrv_if_send_msg(xcons_msg_t *msg) 
{
    if (guisrv_msgid < 0) {
        return -1;
    }
    //printf("send data %x\n", msg->data);
    if (res_write(guisrv_msgid, IPC_NOWAIT, msg, sizeof(xcons_msg_t) - sizeof(long)) < 0)
        return -1;
    return 0;
}

int guisrv_if_recv_data(void *buf, int buflen) 
{
    if (guisrv_pipeid < 0) {
        return -1;
    }
    return res_read(guisrv_pipeid, IPC_NOWAIT, buf, buflen);
}

/* 调用表 */
guisrv_func_t guisrv_call_table[] = {
    __connect,
    __close,
    __clear,
};

/* 文件映射 */
struct file_map {
    char *path;     /* 路径 */
    char execute;   /* 是否需要执行 */
    const char **argv;
};

struct file_map file_map_table[] = {
    {"/bin/bosh", 1, NULL},
};

int guisrv_execute()
{
    struct file_map *fmap;
    int pid = 0;
    int i;
    for (i = 0; i < ARRAY_SIZE(file_map_table); i++) {
        fmap = &file_map_table[i];
        if (fmap->execute) {
            pid = fork();
            if (pid < 0) {
                printf("%s: %s: fork failed!\n", SRV_NAME, __func__);
                return -1;
            }
            if (!pid) { /* 子进程执行新进程 */
                if (execv(fmap->path, fmap->argv)) {
                    printf("%s: %s: execv failed!\n", SRV_NAME, __func__);
                    exit(-1);
                }
            }
        }
    }
    return 0;
}

void *guisrv_echo_thread(void *arg)
{
#if DEBUG_LOCAL == 1
    printf("[guisrv] ready bind service.\n");
#endif    
    /* 绑定成为服务调用 */
    if (srvcall_bind(SRV_GUI) == -1)  {
        printf("%s: bind srvcall failed, service stopped!\n", SRV_NAME);
        return (void *) -1;
    }

#if DEBUG_LOCAL == 1
    printf("[guisrv] bind service ok.\n");
#endif

    guisrv_execute();

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

        /* 2.处理服务 */
        callnum = GETSRV_DATA(&srvarg, 0, int);
#if DEBUG_LOCAL == 1
        printf("%s: srvcall seq=%d callnum %d.\n", SRV_NAME, seq, callnum);
#endif 
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
