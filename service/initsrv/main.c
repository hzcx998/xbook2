#include <sys/res.h>
#include <sys/vmm.h>
#include <sys/proc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/trigger.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>

#define TTY_NAME    "tty0"

/* 当前服务的名字 */
#define SRV_NAME        "initsrv"

/* 要加载的文件服务名字 */
#define SUBSRV_NAME    "filesrv"

#define srvprint(...) \
        printf("[%s] %s: %s: ", SRV_NAME, __FILE__, __func__); printf(__VA_ARGS__)

/**
 * initsrv - 初始化服务
 * 
 * 启动其它服务进程，以及等待所有子进程。
 * 启动的服务可以通过配置服务列表来选择。
 */
int main(int argc, char *argv[])
{
    /* 打开tty，用来进行基础地输入输出 */
    res_open(TTY_NAME, RES_DEV, 0);
    res_open(TTY_NAME, RES_DEV, 0);
    res_open(TTY_NAME, RES_DEV, 0);
    res_ioctl(RES_STDINNO, TTYIO_CLEAR, 0);
    srvprint("service start, a new world is coming.\n");
    
    /* 创建一个子进程 */
    int pid = fork();
    if (pid < 0) {
        srvprint("fork process error! stop service.\n");
        return -1;
    }
    if (pid > 0) {
        /* 父进程 */
        while (1) { /* 等待一个子进程结束 */
            int status = 0;
            int _pid;
            _pid = waitpid(-1, &status, 0);    /* wait any child exit */
            if (_pid > 1) {
                srvprint(" process[%d] exit with status %x.\n", _pid, status);
            }
        }
    }

    /* 子进程，需要启动文件服务 */
    exit(execraw(SUBSRV_NAME, NULL));
    return 0;
}
