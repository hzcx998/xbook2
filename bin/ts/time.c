#include "test.h"

int test_time(int argc, char *argv[])
{
    struct timespec tp;
    printf("clock_gettime: %d\n", clock_gettime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: sec: %d, nsec: %d\n", tp.tv_sec, tp.tv_nsec);
    tp.tv_sec += 100;
    printf("clock_settime: %d\n", clock_settime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: %d\n", clock_gettime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: sec: %d, nsec: %d\n", tp.tv_sec, tp.tv_nsec);
    tp.tv_sec += 100;
    printf("clock_settime: %d\n", clock_settime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: %d\n", clock_gettime(CLOCK_REALTIME, &tp));
    printf("clock_gettime: sec: %d, nsec: %d\n", tp.tv_sec, tp.tv_nsec);
    return 0;
}
