#include "test.h"

int tty_test(int argc, char *argv[])
{
    int fd;
    char * file = "/dev/tty0";
    fd = open (file, O_RDONLY);
    printf("%s", file);
    if(isatty(fd))
    {
        printf("is a tty. \n");
        printf("ttyname = %s \n", ttyname(fd));
    } else
        printf(" is not a tty\n");
    close(fd);
    return 0;
}
