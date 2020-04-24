#ifndef _SYS_UTHREAD_H
#define _SYS_UTHREAD_H

#include <stddef.h>

typedef long uthread_t;

/* 线程栈 */
#define UTHREAD_STACKSIZE_MIN   (16 * 1024) /* 线程栈最小大小 */
#define UTHREAD_STACKSIZE_DEL   UTHREAD_STACKSIZE_MIN /* 默认线程栈大小 */

/* 启动状态 */
#define UTHREAD_CREATE_DETACHED     1 /* 分离状态启动 */
#define UTHREAD_CREATE_JOINABLE     0 /* 正常启动进程 */

typedef struct __uthread_attr {
    int     detachstate;            /* 线程的分离状态 */
    void *  stackaddr;              /* 线程栈的位置 */
    size_t  stacksize;              /* 线程栈的大小 */
} uthread_attr_t;

/* control */
uthread_t uthread_create(
    uthread_attr_t *attr,
    void * (*start_routine) (void *),
    void *arg
);
void uthread_exit(void *retval);

/* attr */
int uthread_attr_init(uthread_attr_t *attr);
int uthread_attr_getdetachstate(const uthread_attr_t *attr, int *detachstate);
int uthread_attr_setdetachstate(uthread_attr_t *attr, int detachstate);    
int uthread_attr_getstacksize(const uthread_attr_t *attr, size_t *stacksize);
int uthread_attr_setstacksize(uthread_attr_t *attr, size_t stacksize);
int uthread_attr_getstackaddr(const uthread_attr_t *attr, void **stackaddr);
int uthread_attr_setstackaddr(uthread_attr_t *attr, void *stackaddr);

#endif  /* _SYS_UTHREAD_H */