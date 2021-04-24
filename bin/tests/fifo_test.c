#include "test.h"
#include <sys/ipc.h>
#include <sys/wait.h>
#include <errno.h>

int fifo_test(int argc, char *argv[])
{
    // unlink("test_fifo");
    if (mkfifo("/pipe/test_fifo", 0666) < 0) {
        printf("open fifo failed!\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid > 0) {
        int fifo_r = open("/pipe/test_fifo", O_RDONLY);
        if (fifo_r < 0) {
            printf("open read fifo faield! %d\n", fifo_r);
            perror("open file error!\n");
            return -1;
        }
        while (1)
        {
            char buf[16] = {0};
            if (read(fifo_r, buf, 15) > 0)
                printf("buf:%s\n", buf);
            if (waitpid(pid, NULL, WNOHANG) == pid) {
                printf("write exit, parent need exit now!\n");
                break;    
            }
        }
        printf("read over, close %d!\n", fifo_r);    
        close(fifo_r);

        printf("parent unlink fifo %d\n", unlink("/pipe/test_fifo"));    

    } else {
        int fifo_w = open("/pipe/test_fifo", O_WRONLY);
        if (fifo_w < 0) {
            printf("open write fifo faield! %d\n", fifo_w);
            perror("open file error!\n");
            return -1;
        }
        int t = 0;
        while (t < 100)
        {
            t++;
            if (write(fifo_w, "hello, world!\n", 12) > 0)
                printf("write done!\n");    
        }
        printf("write over! %d\n", fifo_w);    
        close(fifo_w);
    }

    if (mkfifo("/pipe/test_fifo", 0666) < 0) {
        printf("open fifo failed!\n");
        return -1;
    }
    printf("unlink fifo %d\n", unlink("/pipe/test_fifo"));    
    return 0;
}
