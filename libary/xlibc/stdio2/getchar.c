#include <stdio.h>
#include <sys/res.h>

int getchar(void)
{
    char ch;
    if (res_read(RES_STDINNO, 0, &ch, 1) < 0) 
        return -1;
    return (int) ch;
}
