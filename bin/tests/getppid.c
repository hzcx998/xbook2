#include "test.h"

int getppid_test(int argc, char* argv[])
{
    TEST_START(__func__);
    pid_t ppid = getppid();
    if(ppid > 0) printf("  getppid success. ppid : %d\n", ppid);
    else printf("  getppid error.\n");
    TEST_END(__func__);
    return 0;
}
