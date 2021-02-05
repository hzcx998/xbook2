#include "test.h"
#include <sys/ipc.h>

int fifo_test(int argc, char *argv[])
{
    int fifo_r = openfifo("test_fifo", O_CREAT | O_RDONLY);
    if (fifo_r < 0) {
        printf("open read fifo faield!\n");
        return -1;
    }
    
    pid_t pid = fork();
    if (pid > 0) {
        while (1)
        {
            char buf[16] = {0};
            if (read(fifo_r, buf, 15) > 0)
                printf("buf:%s\n", buf);    
        }
        close(fifo_r);
    } else {
        int fifo_w = openfifo("test_fifo", O_WRONLY);
        if (fifo_w < 0) {
            printf("open write fifo faield!\n");
            return -1;
        }
        int t = 0;
        while (t < 100)
        {
            t++;
            if (write(fifo_w, "hello, world!\n", 12) > 0)
                printf("write done!\n");    
        }

        close(fifo_w);
    }

    return 0;
}
