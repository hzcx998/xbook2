#ifndef _SYS_PROC_H
#define _SYS_PROC_H

#include <types.h>

#define PROC_NAME_LEN 32

/* 任务状态 */
typedef struct _tstate {
    pid_t ts_pid;          /* 进程id */
    pid_t ts_ppid;         /* 父进程id */
    pid_t ts_tgid;         /* 线程组id */
    char ts_state;         /* 运行状态 */
    unsigned long ts_priority;  /* 优先级 */
    unsigned long ts_timeslice; /* 时间片 */
    unsigned long ts_runticks;  /* 运行的ticks数 */
    char ts_name[PROC_NAME_LEN];      /* 任务名字 */
} tstate_t;

#endif   /* _SYS_PROC_H */