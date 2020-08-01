#include <stdio.h>
#include <xcons.h>
#include <string.h>
#include <unistd.h>

#include "cmd.h"
#include "shell.h"

int main(int argc, char *argv[]) {
    printf("book os shell -v 0.01\n");
#if 0    
    int fd = open("pipe", O_PIPE | O_CREAT | O_RDONLY, 0);
    if (fd < 0) {
        printf("open pipe failed!\n");
        return -1;
    }
    printf("open pipe %d!\n", fd);
    int fd1 = open("pipe", O_PIPE | O_WRONLY, 0);
    if (fd1 < 0) {
        printf("open pipe failed!\n");
        return -1;
    }
    printf("open pipe %d!\n", fd1);
    
    int fd2 = dup(fd1);
    printf("dup2 fd %d!\n", fd2);

    int fd3 = dup(fd);
    printf("dup3 fd %d!\n", fd3);

    int fd4 = dup2(1, 0);
    printf("dup4 fd %d!\n", fd4);
    
    write(0, "hello!\n", 6);
#endif
    if (xcons_connect() < 0) {
        printf("xcons connect failed!\n");
        return -1;
    }

    if (init_cmd_man() < 0) {
        printf("init cmd failed!\n");
        return -1;
    }

    cmd_loop();

    exit_cmd_man();

    if (xcons_close() < 0) {
        printf("xcons close failed!\n");
        return -1;
    }
    
    return 0;
}