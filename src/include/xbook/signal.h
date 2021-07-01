#ifndef _XBOOK_SIGNAL_H
#define _XBOOK_SIGNAL_H

#include <stddef.h>
#include <types.h>
#include <string.h>
#include <arch/atomic.h>
#include <xbook/spinlock.h>

/* signal number */
#define SIGNONE    0
#define SIGHUP     1
#define SIGINT     2
#define SIGQUIT    3
#define SIGILL     4
#define SIGTRAP    5
#define SIGABRT    6
#define SIGIOT     SIGABRT
#define SIGBUS     7
#define SIGFPE     8
#define SIGKILL    9
#define SIGUSR1   10
#define SIGSEGV   11
#define SIGUSR2   12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15
#define SIGSTKFLT 16
#define SIGCHLD   17
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20
#define SIGTTIN   21
#define SIGTTOU   22
#define SIGURG    23
#define SIGXCPU   24
#define SIGXFSZ   25
#define SIGVTALRM 26
#define SIGPROF   27
#define SIGWINCH  28
#define SIGIO     29
#define SIGPOLL   SIGIO
#define SIGPWR    30
#define SIGSYS    31
#define SIGUNUSED SIGSYS

#define _NSIG     64

typedef void signalfn_t(int);
typedef signalfn_t *sighandler_t;

typedef void restorefn_t(void);
typedef restorefn_t *sigrestore_t;

#define _NSIG_BPW	64
#define _NSIG_WORDS	(_NSIG / _NSIG_BPW)

typedef struct {
	unsigned long sig[_NSIG_WORDS];
} sigset_t;


#define SIG_ERR  ((void (*)(int))-1)
#define SIG_DFL  ((void (*)(int)) 0)
#define SIG_IGN  ((void (*)(int)) 1)

#define SA_NOCLDSTOP  1
#define SA_NOCLDWAIT  2
#define SA_SIGINFO    4
#define SA_ONSTACK    0x08000000
#define SA_RESTART    0x10000000    /* 需要实现 */
#define SA_NODEFER    0x40000000
#define SA_RESETHAND  0x80000000
#define SA_RESTORER   0x04000000

#define SA_NOMASK SA_NODEFER
#define SA_ONESHOT SA_RESETHAND

#define IS_BAD_SIGNAL(signo) \
    (signo < 1 || signo >= _NSIG)
    
/* sigprocmask的how参数值 */
#define SIG_BLOCK   1 //在阻塞信号集中加上给定的信号集
#define SIG_UNBLOCK 2 //从阻塞信号集中删除指定的信号集
#define SIG_SETMASK 3 //设置阻塞信号集(信号屏蔽码)

/* sigaction struct */
struct sigaction {
	sighandler_t	sa_handler;
	unsigned long	sa_flags;
	sigrestore_t sa_restorer;
	sigset_t	sa_mask;	/* mask last for extensibility */
};

/* 信号结构 */
typedef struct {
    atomic_t count;                     
    struct sigaction action[_NSIG];      /* 信号行为 */
    pid_t sender[_NSIG];                    /* 信号发送者 */
    spinlock_t signal_lock;                          /* 信号自旋锁 */
} signal_t;

typedef struct {
    trap_frame_t trap_frame;        /* 保存原来的栈框 */
    sigset_t old_mask;              /* 旧的阻塞mask */
} signal_frame_t;

int do_send_signal(pid_t pid, int signal, pid_t sender);
int force_signal(int signo, pid_t pid);
int force_signal_self(int signo);

/* 系统调用接口 */
int sys_rt_sigaction(int sig,
        const struct sigaction *act,
		struct sigaction *oact,
		size_t sigsetsize);
int sys_rt_sigprocmask(int how, sigset_t *nset,
		sigset_t *oset, size_t sigsetsize);
int sys_rt_sigreturn();
int sys_kill(pid_t pid, int signal);
int sys_tkill(pid_t tid, int signal);


static inline void sigaddset(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	if (_NSIG_WORDS == 1)
		set->sig[0] |= 1UL << sig;
	else
		set->sig[sig / _NSIG_BPW] |= 1UL << (sig % _NSIG_BPW);
}

static inline void sigdelset(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	if (_NSIG_WORDS == 1)
		set->sig[0] &= ~(1UL << sig);
	else
		set->sig[sig / _NSIG_BPW] &= ~(1UL << (sig % _NSIG_BPW));
}

static inline int sigismember(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	if (_NSIG_WORDS == 1)
		return 1 & (set->sig[0] >> sig);
	else
		return 1 & (set->sig[sig / _NSIG_BPW] >> (sig % _NSIG_BPW));
}

static inline int sigisemptyset(sigset_t *set)
{
	switch (_NSIG_WORDS) {
	case 4:
		return (set->sig[3] | set->sig[2] |
			set->sig[1] | set->sig[0]) == 0;
	case 2:
		return (set->sig[1] | set->sig[0]) == 0;
	case 1:
		return set->sig[0] == 0;
	default:
		return 0;
	}
}

static inline void sigemptyset(sigset_t *set)
{
	switch (_NSIG_WORDS) {
	default:
		memset(set, 0, sizeof(sigset_t));
		break;
	}
}

static inline void sigfillset(sigset_t *set)
{
	switch (_NSIG_WORDS) {
	default:
		memset(set, -1, sizeof(sigset_t));
		break;
	}
}

static inline void sigcopyset(sigset_t *dstset, sigset_t *srcset)
{
    int i;
    for (i = 0; i < _NSIG_WORDS; i++)
        dstset->sig[i] = srcset->sig[i];
}

static inline void sigorset(sigset_t *dstset, sigset_t *srcset)
{
    int i;
    for (i = 0; i < _NSIG_WORDS; i++)
        dstset->sig[i] |= srcset->sig[i];
}

static inline void sigandset(sigset_t *dstset, sigset_t *srcset)
{
    int i;
    for (i = 0; i < _NSIG_WORDS; i++)
        dstset->sig[i] &= ~srcset->sig[i];
}

#define sigmask(sig)	(1UL << ((sig) - 1))

#endif   /* _XBOOK_SIGNAL_H */
