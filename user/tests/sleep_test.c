#include "test.h"

int sleep_test(int argc, char *argv[])
{
    
    printf("sleep 3 s.");
    sleep(3);
    
    printf("usleep.");
    usleep(200);
    usleep(300);
    usleep(500);
    usleep(1000);
    usleep(100000);
    printf("usleep done.");
    
    return 0;
}