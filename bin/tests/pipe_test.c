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
#define STDOUT 1
/*
 * 成功测试时的输出：
 * "  Write to pipe successfully."
 */
static int fd[2];

int pipe_test2(int argc, char *argv[])
{
    TEST_START(__func__);
    int cpid;
    char buf[128] = {0};
    int ret = pipe(fd);
    assert(ret != -1);
    const char *data = "  Write to pipe successfully.\n";
    cpid = fork();
    printf("cpid: %d\n", cpid);
    if(cpid > 0){
	close(fd[1]);
	while(read(fd[0], buf, 1) > 0)
            write(STDOUT, buf, 1);
	write(STDOUT, "\n", 1);
	close(fd[0]);
	wait(NULL);
    }else{
	close(fd[0]);
	write(fd[1], data, strlen(data));
	close(fd[1]);
	exit(0);
    }
    TEST_END(__func__);
    return 0;
}