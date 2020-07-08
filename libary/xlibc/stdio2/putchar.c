#include <stdio.h>
#include <sys/res.h>

int putchar(int ch)
{
    if (res_write(RES_STDOUTNO, 0, &ch, 1) < 0) 
        return -1;
    return (int) ch;
}
