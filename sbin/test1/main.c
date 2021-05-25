#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

int fib(int n)
{
    if (n < 2)
        return 1;
    return fib(n - 1) + fib(n - 2);
}

int main()
{
    printf("hello, I am test1!");
    int n = fib(10);
    printf("fib:%d\n", n);
    _exit(0);
    return 0;
}
