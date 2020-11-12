#include "test.h"
#include <sys/ipc.h>

int fifo_test(int argc, char *argv[])
{
    
    pid_t pid = fork();
    if (pid > 0) {
        int fifo_r = open("test_fifo", O_CREAT | O_FIFO | O_RDONLY);
        if (fifo_r < 0) {
            printf("open read fifo faield!\n");
            return -1;
        }
        while (1)
        {
            char buf[16] = {0};
            read(fifo_r, buf, 15);
            printf("buf:%s\n", buf);    
    
        }
        
        close(fifo_r);
    } else {
        int fifo_w = open("test_fifo", O_FIFO | O_WRONLY);
        if (fifo_w < 0) {
            printf("open write fifo faield!\n");
            return -1;
        }
        while (1)
        {
            write(fifo_w, "hello, world!\n", 12);
            printf("write done!\n");    
        }
        
        close(fifo_w);
    }

    return 0;
}
