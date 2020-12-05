#ifndef _SYS_PROC_H
#define _SYS_PROC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

#define PROC_NAME_LEN 32

#define PROC_CREATE_STOP   0X01            /* 创建后停止运行，不执行 */

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

/* process */
pid_t fork();
void _exit(int status);
int wait(int *status);
int waitpid(pid_t pid, int *status, int options);
pid_t getpid();
pid_t getppid();
pid_t gettid();
unsigned long sleep(unsigned long second);
void sched_yeild();
int tstate(tstate_t *ts, int *idx);
int getver(char *buf, int len);
int create_process(char *const argv[], char *const envp[], unsigned int flags);
int resume_process(pid_t pid);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_PROC_H */
