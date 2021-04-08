#include "test.h"
#include <unistd.h>
int id_test(int argc, char *argv[])
{
    pid_t pid = getpid();
    printf("my pid=%d\n", pid);
    
    pid_t ppid = getppid();
    printf("my ppid=%d\n", ppid);
    
    pid_t gpid = getpgid(0);
    printf("my gpid=%d\n", gpid);
    
    gpid = getpgid(1);
    printf("pid=1 gpid=%d\n", gpid);
    
    setpgid(0, 10);
    gpid = getpgid(0);
    printf("my new gpid=%d\n", gpid);
    
    setpgid(0, 0);
    gpid = getpgid(0);
    printf("my new gpid=%d\n", gpid);
    
    return 0;
}