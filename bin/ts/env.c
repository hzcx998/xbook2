#include "test.h"

extern char **_environ;
int test_env(int argc, char *argv[])
{
    int i;
    for (i = 0; i < argc; i++) {
        printf("argv[%d]=%s, addr=%x\n", i, argv[i], argv[i]);
    }
    char **p = _environ;
    for (i = 0; p[i]; i++) {
        printf("envp[%d]=%s, addr=%x\n", i, p[i], p[i]);
    }
    return 0;
}
