#include "test.h"

int getpid_test(int argc, char* argv[])
{
    TEST_START(__func__);
    int pid = getpid();
    assert(pid >= 0);
    printf("getpid success.\npid = %d\n", pid);
    TEST_END(__func__);
    return 0;
}
