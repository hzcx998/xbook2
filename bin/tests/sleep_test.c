#include "test.h"

int sleep_test(int argc, char *argv[])
{
    printf("sleep 3 s.");
    sleep(1);
    
    printf("usleep1.");
    usleep(2000);
    printf("usleep2.");
    if (usleep(3000000) < 0)
        perror("sleep err:");

    printf("usleep done.");
    return 0;
}