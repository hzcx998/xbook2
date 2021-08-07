#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void exit_func1(void)
{
    printf("exit_func1!\n");
}

void exit_func2(void)
{
    printf("exit_func2!\n");
}

int main(int argc, char *argv[]) {
    int i; for (i = 0; i < argc; i++) {
        printf("arg:%s\n", argv[i]);
    }

    char *s = "hello, world!\n";
    write(1, s, strlen(s));
    printf("hello, word!\n");
    atexit(exit_func1);
    atexit(exit_func2);
    return 0;
}

