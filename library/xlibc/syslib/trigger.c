#include <sys/trigger.h>
#include <sys/syscall.h>

/**
 * trigaddset - 触发器集添加一个触发器
 * @set: 触发器集
 * @trig: 触发器
 */
int trigaddset(trigset_t *set,int trig)
{
    if (IS_BAD_TRIGGER(trig))
        return -1;
    *set |= (1 << trig);
    return 0;
}

/**
 * trigdelset - 触发器集删除一个触发器
 * @set: 触发器集
 * @trig: 触发器
 */
int trigdelset(trigset_t *set,int trig)
{
    if (IS_BAD_TRIGGER(trig))
        return -1;
    *set &= ~(1 << trig);
    return 0;
}

/**
 * trigemptyset - 触发器集置空
 * @set: 触发器集
 */
int trigemptyset(trigset_t *set)
{
    *set = 1;  /* 把第一位置1 */ 
    return 0;
}

/**
 * trigfillset - 触发器集置满
 * @set: 触发器集
 */
int trigfillset(trigset_t *set)
{
    *set = 0xffffffff;  /* 全部置1 */ 
    return 0;
}

/**
 * trigismember - 判断触发器是否置1
 * @set: 触发器集
 */
int trigismember(trigset_t *set,int trig)
{
    if (IS_BAD_TRIGGER(trig))
        return 0;
    return (*set & (1 << trig));
}

/**
 * trigisempty - 判断触发器集是空集
 * @set: 触发器集
 */
int trigisempty(trigset_t *set)
{
    if (*set > 1) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * trigisfull - 判断触发器集是满集
 * @set: 触发器集
 */
int trigisfull(trigset_t *set)
{
    if (*set == 0xffffffff) {
        return 1;
    } else {
        return 0;
    }
}

int trigger(int trig, trighandler_t handler)
{
    return syscall2(int, SYS_TRIGGER, trig, handler);
}

int trigger_action(int trig, trig_action_t *act, trig_action_t *oldact)
{
    return syscall3(int, SYS_TRIGGERACT, trig, act, oldact);
}

int triggeron(int trig, pid_t pid)
{
    return syscall2(int, SYS_TRIGGERON, trig, pid);
}

int trigprocmask(int how, trigset_t *set, trigset_t *oldset)
{
    return syscall3(int, SYS_TRIGPROCMASK, how, set, oldset);
}

int trigpending(trigset_t *set)
{
    return syscall1(int, SYS_TRIGPENDING, set);
}

int trigmask(int trig)
{
    if (IS_BAD_TRIGGER(trig))
        return -1;
    return (1 << trig);
}
