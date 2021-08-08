#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{    
    printf("argc %d argv %x\n", argc, argv);
    int i;
    for (i = 0; i < argc; i++) {
        printf("argv[%d]=%s\n", i, argv[i]);
    }
    char **p = environ;
    printf("envp %x\n", p);
    for (i = 0; p[i]; i++) {
        printf("envp[%d]=%s\n", i, p[i]);
    }
    return 0;
}
