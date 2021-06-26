#ifndef __SYS_RESOURCE_H__
#define __SYS_RESOURCE_H__

#include <stddef.h>

#define PRIO_MIN (-20)
#define PRIO_MAX 20

#define PRIO_PROCESS 0
#define PRIO_PGRP    1
#define PRIO_USER    2

int getpriority(int which, id_t who);
int setpriority(int which, id_t who, int prio);

#endif //__SYS_RESOURCE_H__
