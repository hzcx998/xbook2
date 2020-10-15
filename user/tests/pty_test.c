#include "test.h"

#include <pty.h>

int pty_test(int argc, char *argv[])
{
    printf("----pty test----\n");

    int fdm =  posix_openpt(O_RDWR);
    if (fdm < 0) {
        printf("open device failed!\n");
        return -1;
    }
    printf("fd: %d\n", fdm);


    unlockpt(fdm);
    grantpt(fdm);

    char *sname = ptsname(fdm);
    
    if (sname == NULL) {
        printf("get slave tty name failed!\n");
        return -1;
    }
    
    printf("slave tty %s\n", sname);
    int fds = open(sname, O_DEVEX | O_RDWR, 0);
    if (fds < 0) {
        printf("open slave failed!\n");
        return -1;
    }

    printf("fd: %d\n", fds);
    char buf[32] = {0, };
    strcpy(buf, "hello, ptty!\n");
    write(fdm, buf, strlen(buf));
    char buf1[32] = {0, };
    read(fds, buf1, strlen(buf));
    printf("buf1: %s", buf1);

    close(fds);
    close(fdm);

    return 0;
}