#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

void t()
{
    char b;
    int c[30];
    while (1) {
        
    }
}

/* 栈溢出测试 */
int fib(int n)
{
    char buf[4096] = {0};
    if (n < 2)
        return 1;
    return fib(n - 1) + fib(n - 2);
}

int main()
{
    #if 0
    int a = 0;
    char buf[32] = {0};
    int c = a + 10;
    t();
    while (1) {
        
    }
    #endif
    #if 1
    /* 打开tty，用来进行基础地输入输出 */
    int tty0 = open("/dev/tty0", O_RDONLY);
    if (tty0 < 0) {
        while (1)
        {
            /* code */
        }
        
        return -1;
    }
    int tty1 = open("/dev/tty0", O_WRONLY);
    if (tty1 < 0) {
        while (1)
        {
            /* code */
        }
        close(tty0);
        return -1;
    }
    int tty2 = dup(tty1);
    //printf("[INIT]: start.\n");
    char *str = "[INIT]: start.\n";
    write(tty1, str, strlen(str));
    
    /*====内存测试===*/
    brk(NULL);
    char *mbuf = sbrk(1024);
    memset(mbuf, 0, 1024);

    char *v = (char *) 0x00001000;
    *v = 0x10;

    printf("hello, world!\n");
    int n = fib(10);
    printf("fib:%d\n", n);
    
    char *mbuf0 = malloc(32);
    char *mbuf1 = malloc(512);
    char *mbuf2 = malloc(4096*2);
    printf("malloc:%p %p %p\n", mbuf0, mbuf1, mbuf2);
    free(mbuf0);
    free(mbuf1);
    free(mbuf2);
    printf("test done!\n");
    
    while (1)
    {
        /* code */
    }
    
    // printf("hello, world!\n");
    pid_t pid = fork();
    if (pid > 0) {
        write(tty1, "I am parent\n", 12);
        int state;
        waitpid(pid, &state, 0);
        write(tty1, "Wait done\n", 9);
        while (1)
        {
            /* code */
        }
    } else {
        write(tty1, "I am child\n", 11);
        sleep(3);
        write(tty1, "sleep done\n", 11);
        _exit(0);
    }
    pid = fork();
    if (pid > 0) {
        write(tty1, "I am parent\n", 12);    
    } else {
        write(tty1, "I am child\n", 11);    
    }
    #endif
    while (1)
    {
        /* code */
    }
    
    return 0;
}
