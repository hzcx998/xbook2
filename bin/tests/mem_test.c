#include "test.h"

int mem_test(int argc, char *argv[])
{
    char *buf = malloc(32);
    if (!buf) {
        printf("alloc failed\n");
        fflush(stdout);
        return -1;
    }
    printf("buf:%p\n", buf);
    memset(buf, 0x01, 32);
    free(buf);
    return 0;
}
