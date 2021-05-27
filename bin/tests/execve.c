#include "test.h"

int execve_test(int argc, char* argv[])
{
    char *newargv[] = {"test_echo", NULL};
    char *newenviron[] = {NULL};
    execve("/bin/test_echo", newargv, newenviron);
    printf("  execve error.\n");
    return 0;
}
