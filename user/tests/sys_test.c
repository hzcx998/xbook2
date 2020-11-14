#include "test.h"
#include <sys/syscall.h>

int sys_test(int argc, char *argv[])
{
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

    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("usecond: %d\n", tv.tv_sec * 1000000000 + tv.tv_usec);

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("nsecond: %d\n", ts.ts_sec * 1000000 + ts.ts_nsec);

    return 0;
}