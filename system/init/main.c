#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

// #define _HAS_LOGIN

int main(int argc, char *argv[])
{
    /* 打开tty，用来进行基础地输入输出 */
    int tty0 = opendev("tty0", O_RDONLY);
    if (tty0 < 0) {
        return -1;
    }
    int tty1 = opendev("tty0", O_WRONLY);
    if (tty1 < 0) {
        close(tty0);
        return -1;
    }
    
    int tty2 = opendev("tty0", O_WRONLY);
    if (tty2 < 0) {
        close(tty1);
        close(tty0);
        return -1;
    }

    /* 创建一个子进程 */
    int pid = fork();
    if (pid < 0) {
        printf("[INIT]: fork process error! stop service.\n");
        close(tty2);
        close(tty1);
        close(tty0);
        return -1;
    }
    if (pid > 0) {
        /* 父进程就相当于init进程，功能很简单，就只等待子进程退出 */
        while (1) {
            int status = 0;
            int _pid;
            _pid = waitpid(-1, &status, 0);    /* wait any child exit */
            if (_pid > 1) {
                printf("[INIT]: process[%d] exit with status %d.\n", _pid, status);
            }
        }
    }
    #ifdef _HAS_LOGIN
    pid = getpid();
    ioctl(tty0, TTYIO_HOLDER, &pid);
    char *_argv[3] = {"-s", "/sbin/sh", NULL};
    exit(execv("/sbin/login", _argv));
    #else
    exit(execv("/sbin/sh", NULL));
    #endif
    return 0;
}
