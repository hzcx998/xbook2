#include <stdio.h>
#include <stdlib.h>

void exit_func1(void)
{
    printf("exit_func1!\n");
}

void exit_func2(void)
{
    printf("exit_func2!\n");
}

int main(int argc, char *argv[]) {
    printf("hello, world!\n");
    atexit(exit_func1);
    atexit(exit_func2);
    return 0;
}