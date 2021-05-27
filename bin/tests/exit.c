#include "test.h"

int exit_test(int argc, char* argv[])
{
    int cpid, waitret, wstatus;
    cpid = fork();
    assert(cpid != -1);
    if(cpid == 0){
        exit(0);
    }else{
        waitret = wait(&wstatus);
        if(waitret == cpid) printf("exit OK.\n");
        else printf("exit ERR.\n");
    }
    return 0;
}
