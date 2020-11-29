#include "test.h"
#include <stdio.h>
#include <execinfo.h>

#define LEN 3
static void test2(void)
{
    int i = 0;
    void *buffer[LEN] = {0};
    int n = backtrace(buffer, LEN);
    for (i = 0; i < n; i++) {
        fprintf(stdout, "%p -> ", buffer[i]);
    }
    printf("ok\n");
    return;
}

static void test1(void)
{
    test2();
}

static void test(void)
{
    test1();
}

int backtrace_test(int argc, char* argv[])
{
	test();
    return 0;
}