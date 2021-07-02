#ifndef _SYS_TIMEX_H
#define _SYS_TIMEX_H

#include "time.h"

struct timex {
    int modes;           /* mode selector */
    long offset;         /* time offset (usec) */
    long freq;           /* frequency offset (scaled ppm) */
    long maxerror;       /* maximum error (usec) */
    long esterror;       /* estimated error (usec) */
    int status;          /* clock command/status */
    long constant;       /* pll time constant */
    long precision;      /* clock precision (usec) (read only) */
    long tolerance;      /* clock frequency tolerance (ppm)
                            (read only) */
    struct timeval time; /* current time (read only) */
    long tick;           /* usecs between clock ticks */
};

int sys_adjtimex(struct timex *buf);

#endif  /* _SYS_TIMEX_H */