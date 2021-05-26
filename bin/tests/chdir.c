#include "test.h"

static char buffer[30];
int chdir_test(int argc, char* argv[])
{
    mkdir("test_chdir", 0666);
    int ret = chdir("test_chdir");
    printf("chdir ret: %d\n", ret);
    assert(ret == 0);
    getcwd(buffer, 30);
    printf("  current working dir : %s\n", buffer);
    return 0;
}