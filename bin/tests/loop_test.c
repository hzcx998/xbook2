#include "test.h"

int loop_test(int argc, char *argv[])
{
    int fd0 = open("/dev/loop0", O_RDWR);    
    if (fd0 < 0) {
        fprintf(stderr, "open loop0 device failed!\n");
        return -1;
    }
    char buf[1024] = {0};
    int rb = read(fd0, buf, 1024);
    printf("read: bytes %d\n", rb);

    if (ioctl(fd0, DISKIO_SETUP, "/tmp/a.img") < 0) {
        fprintf(stderr, "setup device failed!\n");
        return -1;        
    }

    rb = read(fd0, buf, 1024);
    printf("read: bytes %d\n", rb);

    lseek(fd0, 2879, SEEK_SET);

    int wb = write(fd0, buf, 1024);
    printf("write: bytes %d\n", wb);
    
    close(fd0);
    return 0;
}
