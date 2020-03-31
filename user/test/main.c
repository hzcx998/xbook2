#include <sys/xbook.h>
#include <string.h>
#include <conio.h>

int func(int n)
{   
    if (n == 0 || n == 1)
        return 1; 
    else 
        return n * func(n - 1); 
}

int __strlen(char *s)
{
    int n = 0;
    while (*s) {
        n++;
        s++;
    }
    return n;
}

x_dev_t dev;
void log(char *str)
{
    x_write(dev, 0, str, strlen(str));
}

int main(int argc, char *argv[])
{
    /*nt i;
    char *p;
    for (i = 0; i < argc; i++) {
        p = (char *)argv[i];
        while (*p++);
    }*/
    dev = x_open("con0", 0);
     
    int i = 0;
    while (i < argc)
    {
        printf("\n-%s ", argv[i]);    
        i++;
    }
    //func(1000);
    //printf("hello, printf %d %s %x\n", 123, "haha", 456);
    log("hello, xbook!");
    strlen("hello, xlibc!|n");
    printf("nice to meet you!");

    unsigned long heap = x_heap(0);
    printf("heap addr:%x\n", heap);

    x_heap(heap + 4096);
    printf("heap addr:%x\n", heap);
    unsigned char *buf = (unsigned char *) heap;
    memset(buf, 0, 4096);

    heap = x_heap(0);
    x_heap(heap + 4096 * 10);
    printf("heap addr:%x\n", heap);
    buf = (unsigned char *) heap;
    memset(buf, 0, 4096 * 10);

    heap = x_heap(heap);
    printf("heap addr:%x\n", heap);

    heap = x_heap(0);
    printf("heap addr:%x\n", heap);

    x_heap(heap + 4096 * 100);
    printf("heap addr:%x\n", heap);
    buf = (unsigned char *) heap;
    memset(buf, 0, 4096 * 100);

    //x_exit(0);
    int pid = x_fork();
    /*if (pid > 0) {
        log("parent!");
        int status;
        int pid2 = x_wait(&status);
        
        //msleep(3000);
        log("parent~\n");
    } else {
        log("child!");
        exit(123);
        
        log("child~\n");
    }
    
    pid = x_fork();*/
    if (pid > 0) {
        log("parent!");
        int status;
        int pid2 = x_wait(&status);
        
        //msleep(3000);
        log("parent~\n");
        x_exit(0);
    } else {
        char *_argv[3] = {"bin", "abc", 0};
        //x_close(dev);
        x_execraw("bin", _argv);
    }

    x_close(dev);
    /*pid = x_fork();
    if (pid > 0) {
        log("parent!");
        int status;
        int pid2 = x_wait(&status);
        
        //msleep(3000);
        log("parent~\n");
    } else {
        log("child!");
        exit(789);
        
        log("child~\n");
    }
    log("end~\n");

    exit(-1);
    */
    while (1) {
        /* 等待子进程 */
    }

    return 0;
}