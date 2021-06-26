#include <xbook/uid.h>
#include <xbook/schedule.h>
#include <xbook/safety.h>
#include <errno.h>

uid_t sys_getuid(void)
{
    return task_current->uid;
}

uid_t sys_geteuid(void)
{
    return task_current->euid;
}

gid_t sys_getgid(void)
{
    return task_current->gid;
}

gid_t sys_getegid(void)
{
    return task_current->egid;
}

int sys_setgid(gid_t gid)
{
    task_current->gid = gid;
    return 0;
}

int sys_setuid(uid_t uid)
{
    task_current->uid = uid;
    return 0;
}

int sys_setegid(gid_t egid)
{
    task_current->egid = egid;
    return 0;
}

int sys_seteuid(uid_t euid)
{
    task_current->euid = euid;
    return 0;
}

/**
 * 获取组
 * 如果size为0就返回组大小
 * 不然，则根据size大小存放到list中
 */
int sys_getgroups(int size, gid_t list[])
{
    if (size < 0 || size >= NGROUPS_MAX)
        return -EINVAL;
    task_t *cur = task_current;
    int count = 0;
    int i;
    if (!size) {    /* 获取附加进程组的数量 */
        for (i = 0; i < NGROUPS_MAX; i++) {
            if (cur->groups[i] != -1)
                count++;
        }
        return count;
    }
    if (!list)
        return -EFAULT;
    for (i = 0; i < NGROUPS_MAX && i < size; i++) {
        if (cur->groups[i] != -1) {
            if (mem_copy_to_user(&list[i], &cur->groups[i], sizeof(gid_t)) < 0)
                return -EFAULT;
            count++;
        }
    }
    return count;
}

/**
 * 设置组
 * 如果size为0就返回组大小
 * 不然，则根据size大小存放到list中
 */
int sys_setgroups(size_t size, const gid_t *list)
{
    if (size < 0 || size >= NGROUPS_MAX)
        return -EINVAL;
    task_t *cur = task_current;
    int i;
    if (!size) {    
        if (!list) { /* 清除所有组 */
            for (i = 0; i < NGROUPS_MAX; i++) {
                cur->groups[i] = -1;
            }
            return 0;
        }
        return -EINVAL;
    }
    /* 复制组 */
    for (i = 0; i < NGROUPS_MAX && i < size; i++) {
        gid_t gid = 0;
        if (mem_copy_from_user(&gid, (void *)&list[i], sizeof(gid_t)) < 0)
            return -EFAULT;
        cur->groups[i] = gid;
    }
    return 0;
}

/**
 * 如果某个id为-1，则不修改id的值
 */
int sys_setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
    task_t *cur = task_current;
    if (ruid != -1)
        cur->uid = ruid;
    if (euid != -1)
        cur->euid = euid;
    if (suid != -1)
        cur->suid = suid;
    return 0;
}

int sys_getresuid(uid_t *ruid, uid_t *euid, uid_t *suid)
{
    task_t *cur = task_current;
    if (ruid) {
        if (mem_copy_to_user(ruid, &cur->uid, sizeof(uid_t)) < 0)
            return -EFAULT;
    }
    if (euid) {
        if (mem_copy_to_user(euid, &cur->euid, sizeof(uid_t)) < 0)
            return -EFAULT;
    }
    if (suid) {
        if (mem_copy_to_user(suid, &cur->suid, sizeof(uid_t)) < 0)
            return -EFAULT;
    }
    return 0;
}

/**
 * 如果某个id为-1，则不修改id的值
 */
int sys_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
    task_t *cur = task_current;
    if (rgid != -1)
        cur->gid = rgid;
    if (egid != -1)
        cur->egid = egid;
    if (sgid != -1)
        cur->sgid = sgid;
    return 0;
}

int sys_getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid)
{
    task_t *cur = task_current;
    if (rgid) {
        if (mem_copy_to_user(rgid, &cur->gid, sizeof(uid_t)) < 0)
            return -EFAULT;
    }
    if (egid) {
        if (mem_copy_to_user(egid, &cur->egid, sizeof(uid_t)) < 0)
            return -EFAULT;
    }
    if (sgid) {
        if (mem_copy_to_user(sgid, &cur->sgid, sizeof(uid_t)) < 0)
            return -EFAULT;
    }
    return 0;
}