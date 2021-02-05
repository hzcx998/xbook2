#include "test.h"
#include <signal.h>

static int count = 0;
void sig_handle(int num)
{
    printf("sig catched\n");
    count++;
    if (count < 5)
        signal(SIGINT, sig_handle);
}

int signal_test(int argc, char *argv[])
{
    signal(SIGINT, sig_handle);
    while (1)
    {
        /* code */
    }
    return 0;
}


