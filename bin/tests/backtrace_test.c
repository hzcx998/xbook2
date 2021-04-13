#include "test.h"
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>

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

static void test4(void)
{
    char *p = NULL;
    *p = 10;
}

static void test3(void)
{
    test4();
}

int backtrace_test(int argc, char* argv[])
{
	test();
    return 0;
}

void segvhandle(int signo)
{
    printf("handled user\n");
    int i = 0;
    void *buffer[LEN] = {0};
    int n = backtrace(buffer, LEN);
    for (i = 0; i < n; i++) {
        fprintf(stdout, "%p -> ", buffer[i]);
    }
    printf("handled user done\n");
}

int backtrace_test2(int argc, char* argv[])
{
    signal(SIGSEGV, segvhandle);
    test3();
    return 0;
}