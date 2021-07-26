#define _GNU_SOURCE

#include "test.h"

int test_mem(int argc, char *argv[])
{
    char *buf = malloc(32);
    if (!buf) {
        printf("alloc failed\n");
        fflush(stdout);
        return -1;
    }
    memset(buf, 0x01, 32);
    free(buf);
    return 0;
}
