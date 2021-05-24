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
    char *ma = malloc(32);
    char *mb = malloc(128);
    char *mc = malloc(4096);
    

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
