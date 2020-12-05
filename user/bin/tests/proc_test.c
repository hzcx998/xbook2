#include "test.h"
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define TEST_N  2

int proc_test(int argc, char *argv[])
{
    #if TEST_N == 0
    char *_argv[3] = {"/usr/hello", "hello, world!\n", NULL};
    #elif  TEST_N == 1
    char *_argv[2] = {"cal", NULL};
    #elif  TEST_N == 2
    char *_argv[2] = {"lua", NULL};
    #endif
    char *envp[3] = {"/usr", "/bin", NULL};
    pid_t pid = create_process(_argv, envp, PROC_CREATE_STOP);
    if (pid < 0) {
        printf("create process failed\n");
        return -1;
    }
    printf("create process ok %d\n", pid);
    ioctl(STDIN_FILENO, TTYIO_HOLDER, &pid);
    
    if (resume_process(pid) < 0)
        printf("resume process failed\n");
    printf("resume process ok %d\n", pid);
    int status;
    waitpid(pid, &status, 0);
    printf("child process %d exit with %d\n", pid, status);
    pid = getpid();
    ioctl(STDIN_FILENO, TTYIO_HOLDER, &pid);
    
    return 0;
}
