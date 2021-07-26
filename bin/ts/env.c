#define _GNU_SOURCE
#include "test.h"

#define AUX_CNT 38

int test_env(int argc, char *argv[])
{
    #if 0
    int i;
    for (i = 0; i < argc; i++) {
        printf("argv[%d]=%s, addr=%x\n", i, argv[i], argv[i]);
    }
    char **p = environ;
    for (i = 0; p[i]; i++) {
        printf("envp[%d]=%s, addr=%x\n", i, p[i], p[i]);
    }
    char **auxv = environ + i + 1;
    size_t aux[AUX_CNT] = { 0 };
    for (i=0; auxv[i]; i+=2)
        if ((int)auxv[i]<AUX_CNT) {
            printf("auxv[%d]=%x\n", i, auxv[i]);
            aux[(int)auxv[i]] = auxv[i+1];
        }
    for (i=0; i < AUX_CNT; i++)
        if (aux[i]) 
            printf("aux[%d]=%x\n", i, aux[i]);
    #else
    
    printf("argc %d argv %x\n", argc, argv);
    fflush(stdout);
    int i;
    for (i = 0; i < argc; i++) {
        printf("argv[%d]=%x, addr=%s\n", i, argv[i], argv[i]);
        fflush(stdout);
    }
    char **p = environ;
    for (i = 0; p[i]; i++) {
        printf("envp[%d]=%x, addr=%s\n", i, p[i], p[i]);
        fflush(stdout);
    }
    char **auxv = environ + i + 1;
    size_t aux[AUX_CNT] = { 0 };
    for (i=0; auxv[i]; i+=2) {        
        printf("auxv[%d]=%x\n", i, auxv[i]);
        if ((int)auxv[i]<AUX_CNT) {
            aux[(int)auxv[i]] = auxv[i+1];
            printf("auxv[%d]=%x auxv[%d+1]=%x\n", i, auxv[i], auxv[i+1]);
            fflush(stdout);
        }
    }
    printf("print aux\n");
    fflush(stdout);

    for (i=0; i < AUX_CNT; i++)
        if (aux[i]) {
            printf("aux[%d]=%x\n", i, aux[i]);
            fflush(stdout);
        }
    #endif

    return 0;
}
