/*
 * xlibc/stdio/system.c
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int system(const char * cmd)
{
    pid_t pid;
    int status;
    if (cmd == NULL) {    
        return (1);
    }
    if((pid = fork()) < 0) {
        status = -1;
    } else if(pid == 0) {
        /* 启动sh来执行命令 */
        execl("/bin/sh", "sh", "-c", cmd, (char *)0);
        exit(127); //子进程正常执行则不会执行此语句
    } else {
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1;
                break;
            }
        }
    }
    return status;
}
