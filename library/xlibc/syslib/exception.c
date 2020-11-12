#include <sys/syscall.h>
#include <sys/exception.h>
#include <sys/proc.h>
#include <stddef.h>
#include <errno.h>

static exp_hander_t __exception_handers[EXP_CODE_MAX_NR] = {0,};

int expsend(pid_t pid, uint32_t code, uint32_t arg)
{
    return syscall3(int, SYS_EXPSEND, pid, code, arg);
}

int expraise(uint32_t code, uint32_t arg)
{
    return expsend(getpid(), code, arg);
}

static int __expchkpoint(uint32_t *code, uint32_t *arg)
{
    return syscall2(int, SYS_EXPCHKPOINT, code, arg);
}

static int __expcatch(uint32_t code, uint32_t state)
{
    return syscall2(int, SYS_EXPCATCH, code, state);
}

static int __expblock(uint32_t code, uint32_t state)
{
    return syscall2(int, SYS_EXPBLOCK, code, state);
}

int expcatch(uint32_t code, exp_hander_t handler)
{
    if (!handler) {
        __exception_handers[code] = NULL;
        return __expcatch(code, 0);
    }
    int callret = __expcatch(code, 1);
    if (callret < 0) {
        return callret;
    }
    __exception_handers[code] = handler;
    return 0;
}

int expblock(uint32_t code)
{
    return __expblock(code, 1);
}

int expunblock(uint32_t code)
{
    return __expblock(code, 0);
}

int expcheck()
{
    uint32_t code;
    uint32_t arg;
    while (1) {
        if (__expchkpoint(&code, &arg) < 0)
            return -1;
        if (code >= EXP_CODE_MAX_NR) {
            return -EINVAL;
        }
        if (__exception_handers[code]) {
            __exception_handers[code](code, arg);
        }
    }
    return 0;
}
