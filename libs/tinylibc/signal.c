#include <signal.h>
#include <sys/syscall.h>

int rt_sigaction(int sig,
        const struct sigaction *act,
		struct sigaction *oact,
		size_t sigactsize)
{
    return syscall(SYS_rt_sigaction, sig, act, oact, sigactsize);
}

int rt_sigprocmask(int how, sigset_t *nset,
		sigset_t *oset, size_t sigsetsize)
{
    return syscall(SYS_rt_sigprocmask, how, nset, oset, sigsetsize);
}

int rt_sigreturn()
{
    return syscall(SYS_rt_sigreturn);
}