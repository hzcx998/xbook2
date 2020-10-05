#include <stdio.h>
#include <unistd.h>

#define CPU_DEV "cpu0"

int main(int argc, char *argv[]) {
    int fd = open(CPU_DEV, O_DEVEX, 0);
    if (fd < 0) {
        printf("cpuinfo: open cpu device failed!\n");
        return -1;
    }
    char buf[51] = {0};   // brand
    if (read(fd, buf, 50) < 0)  {
        printf("cpuinfo: read cpu device failed!\n");
        close(fd);
        return -1;
    }
    printf("cpu: %s\n", buf);
    close(fd);
    return 0;
}