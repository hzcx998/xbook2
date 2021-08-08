#include <sys/syscall.h>
#include <sys/resource.h>

int getpriority(int which, id_t who)
{
	return syscall(SYS_GETPRIORITY, which, who);
}

int setpriority(int which, id_t who, int prio)
{
	return syscall(SYS_SETPRIORITY, which, who, prio);
}

int getrlimit(int resource, struct rlimit *rlim)
{
    return syscall(SYS_GETRLIMIT, resource, rlim);
}
int setrlimit(int resource, const struct rlimit *rlim)
{
    return syscall(SYS_SETRLIMIT, resource, rlim);
}

int prlimit(pid_t pid, int resource, const struct rlimit *new_limit,
    struct rlimit *old_limit)
{
    return syscall(SYS_PRLIMIT64, pid, resource, new_limit, old_limit);
}
