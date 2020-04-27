#ifndef _SYS_UTHREAD_H
#define _SYS_UTHREAD_H

#include <xbook/stddef.h>

typedef long uthread_t;

/* 启动状态 */
#define UTHREAD_CREATE_DETACHED     1 /* 分离状态启动 */
#define UTHREAD_CREATE_JOINABLE     0 /* 正常启动进程 */

/* 取消状态 */
#define UTHREAD_CANCEL_ENABLE       1   /* 收到信号后设置为CANCEL状态 */
#define UTHREAD_CANCEL_DISABLE      0   /* 收到信号忽略信号继续运行 */

/* 取消动作执行时机 */
#define UTHREAD_CANCEL_DEFFERED     2   /* 收到信号后继续运行至下一个取消点再退出 */
#define UTHREAD_CANCEL_ASYCHRONOUS  3   /* 立即执行取消动作（退出） */

typedef struct __uthread_attr {
    int     detachstate;            /* 线程的分离状态 */
    void *  stackaddr;              /* 线程栈的位置 */
    size_t  stacksize;              /* 线程栈的大小 */
} uthread_attr_t;

#endif   /* _SYS_UTHREAD_H */