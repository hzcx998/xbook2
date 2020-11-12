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
void func2(char *b)
{
    char buf[102400] = {0};
    
    printf("BUF:%x\n", buf);
    func1(buf);
}

void func1(char *b)
{
    char buf[102400] = {0};
    printf("BUF:%x\n", buf);
    func2(buf);
}

int exp_test(int argc, char *argv[])
{
    func1(NULL);

    int a = 1;
    int b = 1;
    int c = a / (b - 1);

    char *addr = (char *) 0x70000000;
    *addr = 0;

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
