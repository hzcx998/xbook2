#include <time.h>
#include <sys/time.h>

time_t time(time_t *t)
{
    time_t time;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    time = ts.tv_sec;
    if (t)
        *t = time;
    return time;
}
