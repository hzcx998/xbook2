#include "test.h"

int fork_test(int argc, char* argv[])
{
    int cpid, wstatus;
    cpid = fork();
    assert(cpid != -1);

    if(cpid > 0){
	wait(&wstatus);
	printf("  parent process. wstatus:%d\n", wstatus);
    }else{
	printf("  child process.\n");
	exit(0);
    }
    return 0;
}