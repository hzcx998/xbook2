#include "test.h"

int test_kill(int argc, char *argv[])
{
    printf("start tkill\n");
    syscall(SYS_tkill, getpid(), SIGKILL);
    printf("start kill\n");
    syscall(SYS_kill, getpid(), SIGKILL);
    while (1)
    {
        /* code */
    }
    return 0;
}
