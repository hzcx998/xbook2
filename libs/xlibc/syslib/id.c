#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

int setuid(uid_t uid)
{
    return 0;
}
uid_t getuid(void)
{
    return 0;
}
uid_t geteuid(void)
{
    return 0;
}

int setgid(gid_t gid)
{
    return 0;
}

gid_t getgid(void)
{
    return 0;
}

gid_t getegid(void)
{
    return 0;
}

int setpgid(pid_t pid,pid_t pgid)
{
    int retval = syscall2(int, SYS_SETPGID, pid, pgid);
    if (retval < 0) {
        _set_errno(-retval);
        return -1;
    }
    return retval;
}

int setpgrp(void)
{
    return setpgid(0, 0);
}

pid_t getpgid( pid_t pid)
{
    pid_t retval = syscall1(pid_t, SYS_GETPGID, pid);
    if (retval < 0) {
        _set_errno(-retval);
        return -1;
    }
    return retval;
}

pid_t getpgrp(void)
{
    return getpgid(0);
}
