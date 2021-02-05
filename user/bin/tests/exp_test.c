#include "test.h"
#include <sys/exception.h>

void exp_hander(uint32_t code)
{
    printf("handler code %d\n", code);
    expcatch(EXP_CODE_INT, exp_hander);
}

int exp_test(int argc, char *argv[])
{
    expcatch(EXP_CODE_INT, exp_hander);
    while (1) {
    }
    return 0;
}
