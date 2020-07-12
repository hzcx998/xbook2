#ifndef _XBOOK_TRIGGER_H
#define _XBOOK_TRIGGER_H

#include <types.h>
#include <sys/trigger.h>
#include <arch/interrupt.h>

#define TRIG_LEFT           (1 << 0)        /* 有剩余的触发器没处理 */
#define TRIG_CATCHED        (1 << 1)        /* 触发器被捕捉，并且已经处理了 */

typedef struct {
    trigset_t set;                      /* 触发器集 */                     
    trig_action_t actions[TRIG_NR];     /* 触发器行为 */
    pid_t touchers[TRIG_NR];            /* 触发者进程id */               
    unsigned long flags;                /* 触发器标志 */
} triggers_t;

typedef struct {
	char *ret_addr;                 /* 记录返回地址 */
	unsigned long trig;             /* 触发器 */
    trap_frame_t trap_frame;        /* 保存原来的栈框 */
	char ret_code[8];               /* 构建返回的系统调用代码 */
} trigger_frame_t;

void trigger_init(triggers_t *triggers);
int trigger_force(int trig, pid_t pid);
int sys_trigger_active(int trig, pid_t pid);
int sys_trigger_handler(int trig, trighandler_t handler);
int sys_trigger_action(int trig, trig_action_t *act, trig_action_t *oldact);
int sys_trigger_return(unsigned int ebx, unsigned int ecx, unsigned int esi, unsigned int edi, trap_frame_t *frame);

/**
 * trigger_calc_left - 计算是否还有信号需要处理
 */
static inline void trigger_calc_left(triggers_t *trigger)
{
    if (trigger->set > 1)
        trigger->flags |= TRIG_LEFT;
    else 
        trigger->flags &= ~TRIG_LEFT;
}

static inline void trigger_set_action(triggers_t *trigger, int trig, trig_action_t *ta)
{
    trigger->actions[trig - 1].flags = ta->flags;
    trigger->actions[trig - 1].handler = ta->handler;
}

static inline void trigger_get_action(triggers_t *trigger, int trig, trig_action_t *ta)
{
    ta->flags = trigger->actions[trig - 1].flags;
    ta->handler = trigger->actions[trig - 1].handler;
}

#endif  /* _XBOOK_TRIGGER_H */
