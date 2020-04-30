#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include "ktime.h"

unsigned long alarm(unsigned long second);
unsigned long ktime(ktime_t *ktm);
unsigned long getticks();

#endif  /* _SYS_TIME_H */
