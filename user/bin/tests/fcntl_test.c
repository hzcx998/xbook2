#include "test.h"
#include <fcntl.h>

int fcntl_test(int argc, char *argv[])
{
    int fd = open("/res/test.txt", O_RDONLY);
    if (fd < 0) {
        printf("open failed!\n");
        return -1;
    }
    char buf[32] = {0,};
    int nread = read(fd, buf, 32);
    printf("fd:%d buf:%s read:%d\n", fd, buf, nread);
    int nfd = fcntl(fd, F_DUPFD, 0);    
    memset(buf, 0, 32);
    lseek(nfd, 0, SEEK_SET);
    nread = read(nfd, buf, 32);
    printf("nfd:%d buf:%s read:%d\n", nfd, buf, nread);
    
    fcntl(fd, F_SETFD, FD_CLOEXEC);    
    printf("setfd?%d\n", fcntl(fd, F_GETFD, 0));
    fcntl(fd, F_SETFD, FD_NCLOEXEC);    
    printf("setfd?%d\n", fcntl(fd, F_GETFD, 0));
    
    close(fd);
    close(nfd);
    return 0;
}
