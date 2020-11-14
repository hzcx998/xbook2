#include "test.h"
#include <sys/syscall.h>

int sys_test(int argc, char *argv[])
{
    /*
    while (1)
    {
        syscall0(int, 200);    
    }*/
    
    int fd = open("/res/test.txt", O_RDONLY);
    if (fd < 0) {
        printf("open failed!\n");
        return -1;
    }
    char buf[32] = {0,};
    int nread = read(fd, 0x90000000, 32);
    printf("read: %d buf: %s\n", nread, buf);
    int nwrite = write(fd, 0x90000000, 32);
    printf("write: %d\n", nwrite);
    close(fd);
    return 0;
}