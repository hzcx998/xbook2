#include "test.h"

int oscamp_signal(int argc, char *argv)
{
    char *q = (char *)0x1001;
    *q = 100;
    *q = *q;
    char *v = (char *)0xf0000000;
    *v = 100;
    return 0;
}
