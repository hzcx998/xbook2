#include "test.h"
#include <sys/exception.h>

void exp_hander(uint32_t code, uint32_t arg)
{
    printf("handler code %d arg %d\n", code, arg);
}

int exp_test(int argc, char *argv[])
{
    expcatch(EXP_CODE_INT, exp_hander);
    int i = 0;
    while (1) {
        expcheck();
    }
    return 0;
}
