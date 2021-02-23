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
#define SIGTSTP      SIGSTOP
#define SIGTTIN     EXP_CODE_TTIN   
#define SIGTTOU     EXP_CODE_TTOU
#define SIGSYS      EXP_CODE_SYS   
#define SIGIO       EXP_CODE_DEVICE
#define SIG_UNUSED  EXP_CODE_MAX_NR
#define SIGHUP      (SIG_UNUSED + 0)
#define SIGWINCH    (SIG_UNUSED + 1)
#define SIGVTALRM   (SIG_UNUSED + 2)
#define SIGPROF     (SIG_UNUSED + 3)
#define SIGQUIT     (SIG_UNUSED + 4)
#define SIG_UNUSED_ (SIGPROF + 1)
#define NSIG        SIG_UNUSED_

#define IS_BAD_SIGNAL(signo) \
    (signo < 1 || signo >= NSIG)

typedef void (*sighandler_t)(int);

typedef unsigned int sigset_t;

#ifndef _SIG_ATOMIC_T_DEFINED
typedef int sig_atomic_t;
#define _SIG_ATOMIC_T_DEFINED
#endif /* _SIG_ATOMIC_T_DEFINED */

struct sigaction {
    void (*sa_handler)(int);        // 默认信号处理函数 
    int sa_flags;                   // 信号处理标志
    sigset_t sa_mask;               // 信号捕捉时的屏蔽信号集
};

#define SIG_DFL         ((sighandler_t)0)        /* 默认信号处理方式 */
#define SIG_IGN         ((sighandler_t)1)        /* 忽略信号 */
#define SIG_BLOCKED     ((sighandler_t)2)        /* 阻塞信号 */
#define SIG_UNBLOCKED   ((sighandler_t)3)        /* 接触阻塞信号 */

/* sigprocmask的how参数值 */
#define SIG_BLOCK   1 //在阻塞信号集中加上给定的信号集
#define SIG_UNBLOCK 2 //从阻塞信号集中删除指定的信号集
#define SIG_SETMASK 3 //设置阻塞信号集(信号屏蔽码)

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
    } else if (handler == SIG_IGN || handler == SIG_BLOCKED) {
        if (expblock((uint32_t)signo) < 0)
            return -1;
    } else if (handler == SIG_UNBLOCKED) {
        if (expunblock((uint32_t)signo) < 0)
            return -1;
    } else {
        if (expcatch((uint32_t)signo, (exp_hander_t) handler) < 0)
            return -1;
    }
    return 0;
}

static inline int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    if (IS_BAD_SIGNAL(signum))
        return -1;
    if (oldact) {
        // 保存旧的信号行为
        oldact->sa_handler = (sighandler_t) exphandler(signum);
        oldact->sa_flags = 0;
        oldact->sa_mask = 0;
    }
    // 设置新的信号行为, sa_flags: unused， sa_mask: unused
    return signal(signum, act->sa_handler);
}

static inline int sigaddset(sigset_t *mask,int signo)
{
    if (IS_BAD_SIGNAL(signo))
        return -1;
    *mask |= (1 << signo);
    return 0;
}

static inline int sigdelset(sigset_t *mask,int signo)
{
    if (IS_BAD_SIGNAL(signo))
        return -1;
    *mask &= ~(1 << signo);
    return 0;
}

static inline int sigemptyset(sigset_t *mask)
{
    *mask = 1;  /* 把第一位置1 */ 
    return 0;
}

static inline int sigfillset(sigset_t *mask)
{
    *mask = 0xffffffff;  /* 全部置1 */ 
    return 0;
}

static inline int sigismember(sigset_t *mask,int signo)
{
    if (IS_BAD_SIGNAL(signo))
        return 0;
    return (*mask & (1 << signo));
}

static inline int sigisempty(sigset_t *mask)
{
    if (*mask > 1) {
        return 0;
    } else {
        return 1;
    }
}

static inline int sigisfull(sigset_t *mask)
{
    if (*mask == 0xffffffff) {
        return 1;
    } else {
        return 0;
    }
}

static inline int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if (!set)
        return -1;
    if (oldset) {
        uint32_t mask = 0;
        expmask(&mask);
        *oldset = (sigset_t) mask;
    }
    int i;
    if (how == SIG_BLOCK) {
        for (i = 1; i < NSIG; i++) {
            if (i == SIGKILL || i == SIGSTOP)
                continue;
            if (*set & (1 << i)) {
                expblock(i);
            }
        }
    } else if (how == SIG_UNBLOCK) {
        for (i = 1; i < NSIG; i++) {
            if (i == SIGKILL || i == SIGSTOP)
                continue;
            if (*set & (1 << i)) {
                expunblock(i);
            }
        }
    } else if (how == SIG_SETMASK) {
        for (i = 1; i < NSIG; i++) {
            if (i == SIGKILL || i == SIGSTOP)
                continue;
            if (*set & (1 << i)) {
                expblock(i);
            } else {
                expunblock(i);
            }
        }
    } else {
        return -1;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __XLIBC_SIGNAL_H__ */