#include "test.h"

#include <sys/ipc.h>

int shm_test(int argc, char *argv[])
{
    printf("----shm test----\n");
    
    int shmid = shmget("shm-test", 4096, IPC_CREAT | IPC_EXCL);
    if (shmid < 0) {
        printf("get shm failed!\n");
        return -1;
    }
    void *addr = shmmap(shmid, NULL, 0);
    printf("map addr:%x\n", addr);
    memset(addr, 0x5a, 4096);

    int pid = fork();
    if (pid == -1) {
        printf("fork failed!\n");
        return -1;
    } else if (pid > 0) {
        printf("parent.\n");
        int status;
        wait(&status);
        printf("child exit code %d.\n", status);
        shmunmap(addr, 0);
        shmput(shmid);
    } else {
        printf("child.\n");
        uint32_t *p = (uint32_t *) addr;
        printf("data: %x %x\n", p[0], p[1023]);
        shmunmap(addr, 0);
        exit(1234);
    }
    return 0;
}