#include <sys/resource.h>
#include <xbook/safety.h>
#include <xbook/schedule.h>
#include <errno.h>

int sys_getpriority(int which, id_t who)
{
    return -ENOSYS;
}

int sys_setpriority(int which, id_t who, int prio)
{
    return -ENOSYS;    
}

static int do_getrlimit(task_t *task, int resource, struct rlimit *rlim)
{
    if (!rlim)
        return -EFAULT;
    if (resource < 0 || resource > RLIM_NLIMITS)
        return -EINVAL;
    /* TODO: get limit */
    return 0;
}

int sys_getrlimit(int resource, struct rlimit *rlim)
{
    return do_getrlimit(task_current, resource, rlim);
}

static int do_setrlimit(task_t *task, int resource, const struct rlimit *rlim)
{
    if (!rlim)
        return -EFAULT;
    if (resource < 0 || resource > RLIM_NLIMITS)
        return -EINVAL;
    struct rlimit tmprlim;
    if (mem_copy_from_user(&tmprlim, (struct rlimit *)rlim, sizeof(struct rlimit)) < 0)
        return -EFAULT;
    if (tmprlim.rlim_cur > tmprlim.rlim_max)
        return -EINVAL;
    /* TODO: set limit */
    return 0;
}

int sys_setrlimit(int resource, const struct rlimit *rlim)
{
    return do_setrlimit(task_current, resource, rlim);
}

int sys_prlimit(pid_t pid, int resource, const struct rlimit *new_limit,
    struct rlimit *old_limit)
{
    task_t *task;
    if (!pid)
        task = task_current;
    else {
        task = task_find_by_pid(pid);
        if (!task)
            return -ESRCH;
    }
    int err;
    if (old_limit) {
        err = do_getrlimit(task, resource, old_limit);
        if (err < 0)
            return err;
    }
    if (new_limit) {
        err = do_setrlimit(task, resource, new_limit);
        if (err < 0)
            return err;
    }
    return 0;
}
