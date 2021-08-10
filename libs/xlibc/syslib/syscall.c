#include <sys/syscall.h>
#include <sys/resource.h>
#include <sys/time.h>

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

int getitimer(int which, struct itimerval *curr_value)
{
    return syscall(SYS_GETITIMER, which, curr_value);
}

int setitimer(int which, const struct itimerval *restrict new_value,
    struct itimerval *restrict old_value)
{
    return syscall(SYS_GETITIMER, which, new_value, old_value);
}

int sched_get_priority_max(int policy)
{
    return syscall(SYS_SCHED_GET_PRIORITY_MAX, policy);
}

int sched_get_priority_min(int policy)
{
    return syscall(SYS_SCHED_GET_PRIORITY_MIN, policy);
}
