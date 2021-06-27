#include "test.h"
#include <sys/times.h>

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

    struct tms tmsbuf;
    printf("%s: %d\n", $(times), times(&tmsbuf));
    printf("%s: %d %d %d %d\n", $(times), tmsbuf.tms_cstime, tmsbuf.tms_cutime, tmsbuf.tms_stime, tmsbuf.tms_utime);
    
    return 0;
}
