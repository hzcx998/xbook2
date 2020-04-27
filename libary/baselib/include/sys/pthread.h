#ifndef _SYS_PTHREAD_H
#define _SYS_PTHREAD_H

#include <stddef.h>

typedef unsigned long pthread_t;

/* 线程栈参数 */
#define PTHREAD_STACKSIZE_MIN       (16 * 1024) /* 最小栈大小 */   
#define PTHREAD_STACKSIZE_DEL       PTHREAD_STACKSIZE_MIN


/* 启动状态 */
#define PTHREAD_CREATE_DETACHED     1 /* 分离状态启动 */
#define PTHREAD_CREATE_JOINABLE     0 /* 正常启动进程 */

/* 取消状态 */
#define PTHREAD_CANCEL_ENABLE       1   /* 收到信号后设置为CANCEL状态 */
#define PTHREAD_CANCEL_DISABLE      0   /* 收到信号忽略信号继续运行 */

/* 取消动作执行时机 */
#define PTHREAD_CANCEL_DEFFERED     2   /* 收到信号后继续运行至下一个取消点再退出 */
#define PTHREAD_CANCEL_ASYCHRONOUS  3   /* 立即执行取消动作（退出） */

typedef struct __pthread_attr {
    int     detachstate;            /* 线程的分离状态 */
    void *  stackaddr;              /* 线程栈的位置 */
    size_t  stacksize;              /* 线程栈的大小 */
} pthread_attr_t;

/* control */
int pthread_create(
    pthread_t *thread,
    pthread_attr_t *attr,
    void * (*start_routine) (void *),
    void *arg
);
void pthread_exit(void *retval);
int pthread_join(pthread_t thread, void **thread_return);
int pthread_detach(pthread_t thread);
pthread_t pthread_self(void);
int pthread_equal(pthread_t thread1, pthread_t thread2);
int pthread_cancel(pthread_t thread);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);
void pthread_testcancel(void);

/* attr */
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);    
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);

#endif  /* _SYS_PTHREAD_H */