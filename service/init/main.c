#include <sys/res.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    /* 打开tty，用来进行基础地输入输出 */
    int tty0 = res_open("tty0", RES_DEV, 0);
    if (tty0 < 0) {
        return -1;
    }
    int tty1 = res_open("tty0", RES_DEV, 0);
    if (tty1 < 0) {
        res_close(tty0);
        return -1;
    }
    
    int tty2 = res_open("tty0", RES_DEV, 0);
    if (tty2 < 0) {
        res_close(tty1);
        res_close(tty0);
        return -1;
    }
    int fd;
    fd = open("/res/test.txt", O_RDWR, 0);
    fd = open("/res/test.txt", O_RDWR, 0);
    fd = open("/res/test.txt", O_RDWR, 0);
    fd = 0;
    //res_ioctl(RES_STDINNO, TTYIO_CLEAR, 0);
    printf("start 'init' service, in user mode now.\n");
    /* 创建一个子进程 */
    int pid = fork();
    if (pid < 0) {
        printf("[INIT]: fork process error! stop service.\n");
        res_close(tty2);
        res_close(tty1);
        res_close(tty0);
        return -1;
    }
    if (pid > 0) {
        /* 父进程就相当于init进程，功能很简单，就只等待子进程退出 */
        while (1) { /* 等待一个子进程结束 */
            int status = 0;
            int _pid;
            _pid = waitpid(-1, &status, 0);    /* wait any child exit */
            if (_pid > 1) {
                printf("[INIT]: process[%d] exit with status %d.\n", _pid, status);
            }
        }
    }
    /* 配置环境变量 */
    char *const envp[3] = {"/bin", "/sbin", NULL}; 
    /* 执行shell */
    exit(execve("guisrv", NULL, envp));
    return 0;
}
