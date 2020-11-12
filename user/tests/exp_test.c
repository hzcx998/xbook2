#include "test.h"
#include <sys/exception.h>

void exp_hander(uint32_t code, uint32_t arg)
{
    printf("handler code %d arg %d\n", code, arg);
    fflush(stdout);
    expunblock(EXP_CODE_INT);
    if (arg > 10000) {
        expcatch(EXP_CODE_INT, NULL);
    }
}

int exp_test(int argc, char *argv[])
{
    expcatch(EXP_CODE_INT, exp_hander);
    int i = 0;
    while (1) {
        i++;
        expcheck();
        if (i % 1000 == 0) {
            //
            expsend(getpid(), EXP_CODE_INT, i);
        }
    }
    return 0;
}
