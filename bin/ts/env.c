#include "test.h"

#define AUX_CNT 38

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
    char **auxv = _environ + i + 1;
    size_t aux[AUX_CNT] = { 0 };
    for (i=0; auxv[i]; i+=2)
        if ((int)auxv[i]<AUX_CNT) {
            printf("auxv[%d]=%x\n", i, auxv[i]);
            aux[(int)auxv[i]] = auxv[i+1];
        }
    for (i=0; i < AUX_CNT; i++)
        if (aux[i]) 
            printf("aux[%d]=%x\n", i, aux[i]);

    return 0;
}
