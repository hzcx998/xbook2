#include "test.h"

int yield_test(int argc, char* argv[])
{
    TEST_START(__func__);

    for (int i = 0; i < 3; ++i){
        if(fork() == 0){
	    for (int j = 0; j< 5; ++j){
                sched_yield();
                printf("  I am child process: %d. iteration %d.\n", getpid(), i);
	    }
	    exit(0);
        }
    }
    wait(NULL);
    wait(NULL);
    wait(NULL);
    TEST_END(__func__);
    return 0;
}
