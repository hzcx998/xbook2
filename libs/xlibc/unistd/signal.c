#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int signal(int signo, sighandler_t handler)
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

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
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
