#include <usrmsg.h>

int func(int n)
{   
    if (n == 0 || n == 1)
        return 1; 
    else 
        return n * func(n - 1); 
}

int fork()
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_FORK);
    umsg(msg);
    return umsg_get_retval(msg, int);
}

void exit(int status)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_EXIT);
    umsg_set_arg0(msg, status);
    umsg(msg);
}

int wait(int *status)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_WAIT);
    umsg_set_arg0(msg, 0);
    umsg(msg);
    if (status) /* not null */
        *status = umsg_get_arg0(msg, int);
    return umsg_get_retval(msg, int);
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

void log(char *str)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_WRITE);
    umsg_set_arg0(msg, "con0");
    umsg_set_arg1(msg, 0);
    umsg_set_arg2(msg, str);
    umsg_set_arg3(msg, __strlen(str));
    umsg(msg);
}

void msleep(int msecond)
{
    define_umsg(msg);
    umsg_set_type(msg, UMSG_MSLEEP);
    umsg_set_arg0(msg, msecond);
    umsg(msg);
}

int main(int argc, char *argv[])
{
    int i;
    char *p;
    for (i = 0; i < argc; i++) {
        p = (char *)argv[i];
        while (*p++);
    }
    //func(1000);
    define_umsg(msg);
    umsg_set_type(msg, UMSG_OPEN);
    umsg_set_arg0(msg, "con0");
    umsg(msg);

    log("hello, xbook!");
    
    int pid = fork();
    if (pid > 0) {
        log("parent!");
        int status;
        int pid2 = wait(&status);

        log("parent~\n");
        
    } else {
        log("child!");
        exit(123);
        
        log("child~\n");
    }
    
    while (1);
    return 0;
}