#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <stddef.h>

#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF 2

struct timeval {
    time_t tv_sec;         /* seconds */
    time_t tv_usec;        /* microseconds */
};

struct itimerval {
    struct timeval it_interval; /* Interval for periodic timer */
    struct timeval it_value;    /* Time until next expiration */
};

int getitimer(int which, struct itimerval *curr_value);
int setitimer(int which, const struct itimerval *restrict new_value,
    struct itimerval *restrict old_value);

#endif //__SYS_TIME_H__
