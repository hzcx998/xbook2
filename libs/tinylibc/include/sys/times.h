#ifndef __SYS_TIMES_H__
#define __SYS_TIMES_H__

#include <stddef.h>

struct tms {
    clock_t tms_utime;  /* user time */
    clock_t tms_stime;  /* system time */
    clock_t tms_cutime; /* user time of children */
    clock_t tms_cstime; /* system time of children */
};

clock_t times(struct tms *buf);

#endif //__SYS_TIMES_H__
