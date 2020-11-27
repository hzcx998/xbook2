#include "test.h"

int file_test(int argc, char *argv[])
{
    int fd = open("tmp.txt", O_CREAT | O_RDWR);
    if (fd < 0)
        sys_err("open file failed!");
    
    pid_t pid = fork();
    if (pid < 0)
        sys_err("fork failed!");
    if (pid > 0) {
        char *str1 = "hello, parent!\n";
        int i = 0;
        while (i < 10) {
            i++;
            if (write(fd, str1, strlen(str1)) > 0)
                printf("parent wirte:%s\n", str1);
        }
        close(fd);
    } else {
        char *str2 = "hello, child!\n";
        int j = 0;
        while (j < 10) {
            j++;
            if (write(fd, str2, strlen(str2)) > 0)
                printf("child wirte:%s\n", str2);
        }
        close(fd);
    }
    return 0;
}
