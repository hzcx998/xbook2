#include "test.h"

size_t stack[1024] = {0};
static int child_pid;

#undef SIGCHLD
#define SIGCHLD   17

static int child_func(void *arg){
    printf("  Child says successfully!\n");
    return 0;
}

int clone_test(int argc, char* argv[])
{
    int wstatus;
    child_pid = clone(child_func, NULL, stack, 1024, SIGCHLD);
    assert(child_pid != -1);
    if (child_pid == 0){
	exit(0);
    }else{
	if(wait(&wstatus) == child_pid)
	    printf("clone process successfully.\npid:%d\n", child_pid);
	else
	    printf("clone process error.\n");
    }
    return 0;
}