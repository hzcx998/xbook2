#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <stddef.h>
#include <types.h>

/* 这些在xbook2内核钟并没有使用，只是为了兼容Linux系统系统调用 */
#define PRIO_MIN (-20)
#define PRIO_MAX 20

#define PRIO_PROCESS 0
#define PRIO_PGRP    1
#define PRIO_USER    2

#define RLIM_INFINITY (~0ULL)
#define RLIM_SAVED_CUR RLIM_INFINITY
#define RLIM_SAVED_MAX RLIM_INFINITY

#define RLIMIT_CPU     0
#define RLIMIT_FSIZE   1
#define RLIMIT_DATA    2
#define RLIMIT_STACK   3
#define RLIMIT_CORE    4
#ifndef RLIMIT_RSS
#define RLIMIT_RSS     5
#define RLIMIT_NPROC   6
#define RLIMIT_NOFILE  7
#define RLIMIT_MEMLOCK 8
#define RLIMIT_AS      9
#endif
#define RLIMIT_LOCKS   10
#define RLIMIT_SIGPENDING 11
#define RLIMIT_MSGQUEUE 12
#define RLIMIT_NICE    13
#define RLIMIT_RTPRIO  14
#define RLIMIT_RTTIME  15
#define RLIMIT_NLIMITS 16

#define RLIM_NLIMITS RLIMIT_NLIMITS

int sys_getpriority(int which, id_t who);
int sys_setpriority(int which, id_t who, int prio);

typedef unsigned long long rlim_t;

struct rlimit {
	rlim_t rlim_cur;
	rlim_t rlim_max;
};

int sys_getrlimit(int resource, struct rlimit *rlim);
int sys_setrlimit(int resource, const struct rlimit *rlim);
int sys_prlimit(pid_t pid, int resource, const struct rlimit *new_limit,
    struct rlimit *old_limit);

#endif   /* _SYS_RESOURCE_H */