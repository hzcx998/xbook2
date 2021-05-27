#include "test.h"

int getcwd_test(int argc, char* argv[])
{
    TEST_START(__func__);
    char *cwd = NULL;
    char buf[128] = {0};
    cwd = getcwd(buf, 128);
    if(cwd != NULL) printf("getcwd: %s successfully!\n", buf);
    else printf("getcwd ERROR.\n");
    TEST_END(__func__);
    return 0;
}
