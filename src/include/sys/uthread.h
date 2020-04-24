#ifndef _SYS_UTHREAD_H
#define _SYS_UTHREAD_H

#include <xbook/stddef.h>

typedef long uthread_t;

typedef struct __uthread_attr {
    int     detachstate;            /* 线程的分离状态 */
    void *  stackaddr;              /* 线程栈的位置 */
    size_t  stacksize;              /* 线程栈的大小 */
} uthread_attr_t;

#endif   /* _SYS_UTHREAD_H */