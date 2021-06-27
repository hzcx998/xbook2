#include "test.h"

int test_pid(int argc, char *argv[])
{
    printf("%s: %d\n", $(getpid), getpid());
    printf("%s: %d\n", $(getppid), getppid());
    printf("%s: %d\n", $(gettid), gettid());
    printf("%s: %d\n", $(getpgrp), getpgrp());
    printf("%s: %d\n", $(getpgid), getpgid(getpid()));
    printf("%s: %d\n", $(setpgid), setpgid(0, 10));
    printf("%s: %d\n", $(getpgid), getpgid(getpid()));
    printf("%s: %d\n", $(SYS_set_tid_address), syscall(SYS_set_tid_address, NULL));
    printf("%s: %d\n", $(getsid), getsid(0));
    printf("%s: %d\n", $(setsid), setsid());
    printf("%s: %d\n", $(setsid), setsid());
    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed!\n");
        return 1;
    }
    printf("pid %d\n", pid);
    if (!pid) {
        printf("child %s: %d\n", $(setsid), setsid());
    } else {
        wait(NULL);
    }
    return 0;
}
