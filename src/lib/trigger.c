#include <xbook/trigger.h>

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
    *set = 0xffffffffUL;  /* 全部置1 */ 
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
 * trigismember - 判断触发器是否置1
 * @set: 触发器集
 */
int trigorset(trigset_t *set, trigset_t *setb)
{
    *set |= *setb;
    return 0;
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
    if (*set == 0xffffffffUL) {
        return 1;
    } else {
        return 0;
    }
}

int trigmask(int trig)
{
    if (IS_BAD_TRIGGER(trig))
        return -1;
    return (1 << trig);
}
