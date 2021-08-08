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

int file_test2(int argc, char *argv[])
{
    char *buf = malloc(64*1024);
    if (buf == NULL) {
        printf("malloc for test failed!\n");
        return -1;
    }
    while (1)
    {
        int fd = open("/bin/lua", O_RDONLY);
        if (fd < 0) {
            printf("open file failed!\n");
            break;
        }
        while (1)
        {
            int rd = read(fd, buf, 64*1024);
            printf("read %d.\n", rd);
            if (rd <= 0)
                break;
        }
        close(fd);
        printf("read done.\n");
    }
    free(buf);
    printf("test end\n");
    return 0;
}

int file_test3(int argc, char *argv[])
{
    FILE *fp = NULL;
    fp = fopen("/tmp/test2.txt", "wb");
    if (!fp) {
        printf("fopen failed\n");
        return -1;
    }
    fwrite("hello", 5, 1, fp);
    fclose(fp);
    printf("test end\n");
    return 0;
}
