#include <xbook.h>
#include <string.h>

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
    
    //func(1000);
    dev = x_open("con0", 0);
    //printf("hello, printf %d %s %x\n", 123, "haha", 456);
    log("hello, xbook!");
    
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
    } else {
        char *_argv[3] = {"bin", "abc", 0};
        //x_close(dev);
        x_execraw(_argv[0], _argv);
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