#include "test.h"

int pipe_test(int argc, char *argv[])
{
    printf("----pipe test----\n");
    int fd[2];
    int ret = pipe(fd);
    if (ret < 0) {
        printf("create pipe failed!\n");
        return -1;
    }
    int pid = fork();
    if (pid == -1)
        return -1; 
    /* 父进程读取子进程传递的数据 */

    if (pid > 0) {
        
        printf("I am parent %d, my child %d.\n", getpid(), pid);
        close(fd[1]);
        /* 读取数据 */
        char buf[32];
        memset(buf, 0, 32);
        read(fd[0], buf, 32);
        printf("parent read: %s\n", buf);

        close(fd[0]);
    } else {
        
        printf("I am child %d, my parent %d.\n", getpid(), getppid());
        close(fd[0]);
        write(fd[1], "hello, pipe!\n", strlen("hello, pipe!\n"));
        printf("child write done!\n");
        close(fd[1]);
    }
    return 0;
}