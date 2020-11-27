#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* 等待标志 */
#define WNOHANG     0X01    /* 若由pid指定的子进程未发生状态改变(没有结束)，则waitpid()不阻塞，立即返回0 */

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_WAIT_H */