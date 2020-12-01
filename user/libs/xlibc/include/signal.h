#ifndef __XLIBC_SIGNAL_H__
#define __XLIBC_SIGNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "types.h"
#include "stddef.h"
#include <sys/exception.h>

#define SIGINT      EXP_CODE_INT
#define SIGILL      EXP_CODE_ILL
#define SIGTRAP     EXP_CODE_TRAP
#define SIGABRT     EXP_CODE_ABORT   
#define SIGBUS      EXP_CODE_BUS   
#define SIGFPE      EXP_CODE_FPE   
#define SIGKILL     EXP_CODE_FINALHIT   
#define SIGUSR1     EXP_CODE_USER  
#define SIGSEGV     EXP_CODE_SEGV
#define SIGPIPE     EXP_CODE_PIPE   
#define SIGALRM     EXP_CODE_ALRM 
#define SIGTERM     EXP_CODE_TERM
#define SIGSTKFLT   EXP_CODE_STKFLT
#define SIGCHLD     EXP_CODE_CHLD
#define SIGCONT     EXP_CODE_CONT
#define SIGSTOP     EXP_CODE_STOP
#define SIGTTIN     EXP_CODE_TTIN   
#define SIGTTOU     EXP_CODE_TTOU
#define SIGSYS      EXP_CODE_SYS   
#define SIGIO       EXP_CODE_DEVICE
#define NSIG        EXP_CODE_MAX_NR

typedef void (*sighandler_t)(int);

#define SIG_DFL         ((sighandler_t)0)        /* 默认信号处理方式 */
#define SIG_IGN         ((sighandler_t)1)        /* 忽略信号 */
#define SIG_BLOCK       ((sighandler_t)2)        /* 阻塞信号 */
#define SIG_UNBLOCK     ((sighandler_t)3)        /* 接触阻塞信号 */

static inline int kill(pid_t pid, int signo)
{
    return expsend(pid, (uint32_t) signo);
}

static inline int raise(int signo)
{
    return expraise((uint32_t) signo);
}

static inline int signal(int signo, sighandler_t handler)
{
    if (handler == SIG_DFL) {
        if (expcatch((uint32_t)signo, NULL) < 0)
            return -1;
    } else if (handler == SIG_IGN || handler == SIG_BLOCK) {
        if (expblock((uint32_t)signo) < 0)
            return -1;
    } else if (handler == SIG_UNBLOCK) {
        if (expunblock((uint32_t)signo) < 0)
            return -1;
    } else {
        if (expcatch((uint32_t)signo, (exp_hander_t) handler) < 0)
            return -1;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __XLIBC_SIGNAL_H__ */