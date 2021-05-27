#include "test.h"

#include <sys/wait.h>

static int i = 1000;
int waitpid_test(int argc, char* argv[])
{
    TEST_START(__func__);
    int cpid, wstatus;
    cpid = fork();
    assert(cpid != -1);
    if(cpid == 0){
	while(i--);
	sched_yield();
	printf("This is child process\n");
        exit(3);
    }else{
	pid_t ret = waitpid(cpid, &wstatus, 0);
	assert(ret != -1);
	if(ret == cpid && WEXITSTATUS(wstatus) == 3)
	    printf("waitpid successfully.\nwstatus: %x\n", WEXITSTATUS(wstatus));
	else
	    printf("waitpid error.\n");

    }
    TEST_END(__func__);
    return 0;
}
