#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/proc.h>

int main(int argc, char *argv[])
{
    printf("bosh: say, hello!\n");
    
    printf("bosh: I am do nothing now!\n");
    
    exit(0);
    while (1);  /* loop forever */

    return 0;   
}

int read_key()
{
    int key = 0;
    while ((res_read(RES_STDINNO, 0, &key, 1)) <= 0);
    return key & 0xff;  /* 只取8位 */
}
