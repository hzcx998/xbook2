#include "test.h"

int wait_test(int argc, char* argv[])
{
    TEST_START(__func__);
    int cpid, wstatus;
    cpid = fork();
    if(cpid == 0){
	printf("This is child process\n");
        exit(0);
    }else{
	pid_t ret = wait(&wstatus);
	assert(ret != -1);
	if(ret == cpid)
	    printf("wait child success.\nwstatus: %d\n", wstatus);
	else
	    printf("wait child error.\n");
    }
    TEST_END(__func__);
    return 0;
}
