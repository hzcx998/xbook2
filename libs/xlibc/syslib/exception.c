#include <sys/syscall.h>
#include <sys/exception.h>
#include <sys/proc.h>
#include <stddef.h>
#include <errno.h>

int expsend(pid_t pid, uint32_t code)
{
    return syscall2(int, SYS_EXPSEND, pid, code);
}

int expraise(uint32_t code)
{
    return expsend(getpid(), code);
}

static int __expblock(uint32_t code, uint32_t state)
{
    return syscall2(int, SYS_EXPBLOCK, code, state);
}

int expmask(uint32_t *mask)
{
    if (!mask)
        return -1;
    return syscall1(int, SYS_EXPMASK, mask);
}

int expcatch(uint32_t code, exp_hander_t handler)
{
    return syscall2(int, SYS_EXPCATCH, code, handler);
}

int expblock(uint32_t code)
{
    return __expblock(code, 1);
}

int expunblock(uint32_t code)
{
    return __expblock(code, 0);
}

void *exphandler(uint32_t code)
{
    return syscall1(void *, SYS_EXPHANDLER, code);
}
