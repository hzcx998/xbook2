#ifndef _XBOOK_PTHREAD_H
#define _XBOOK_PTHREAD_H

#include <sys/pthread.h>
#include <arch/atomic.h>

/* 一个进程最多32个线程 */
#define PTHREAD_MAX_NR      32

typedef struct pthread_desc {
    atomic_t thread_count;
} pthread_desc_t;

void pthread_desc_init(pthread_desc_t *pthread);
void pthread_desc_exit(pthread_desc_t *pthread);

pid_t sys_thread_create(
    pthread_attr_t *attr,
    task_func_t *func,
    void *arg,
    void *thread_entry
);
void sys_thread_exit(void *retval);
int sys_thread_detach(pthread_t thread);
int sys_thread_join(pthread_t thread, void **thread_return);

int sys_thread_setcanceltype(int type, int *oldtype);
int sys_thread_setcancelstate(int state, int *oldstate);
void sys_thread_testcancel(void);
int sys_thread_cancel(pthread_t thread);

#endif   /* _XBOOK_PTHREAD_H */
