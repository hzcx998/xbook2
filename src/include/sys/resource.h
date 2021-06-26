#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <stddef.h>

/* 这些在xbook2内核钟并没有使用，只是为了兼容Linux系统系统调用 */
#define PRIO_MIN (-20)
#define PRIO_MAX 20

#define PRIO_PROCESS 0
#define PRIO_PGRP    1
#define PRIO_USER    2

int sys_getpriority(int which, id_t who);
int sys_setpriority(int which, id_t who, int prio);

#endif   /* _SYS_RESOURCE_H */