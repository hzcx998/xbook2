#ifndef _XBOOK_TRIGGER_H
#define _XBOOK_TRIGGER_H

#include <types.h>
#include <sys/trigger.h>
#include <arch/interrupt.h>
#include <xbook/spinlock.h>

#define TRIG_LEFT           (1 << 0)
#define TRIG_CATCHED        (1 << 1)

typedef struct {
    trigset_t blocked;                     
    trigset_t set;                     
    trig_action_t actions[TRIG_NR];
    pid_t touchers[TRIG_NR];               
    unsigned long flags;
    spinlock_t trig_lock;
} triggers_t;

typedef struct {
	char *ret_addr;
	unsigned long trig;
    trigset_t oldmask;
    trap_frame_t trap_frame;
	char ret_code[8];
} trigger_frame_t;

void trigger_init(triggers_t *triggers);
int trigger_force(int trig, pid_t pid);
int sys_trigger_active(int trig, pid_t pid);
int sys_trigger_handler(int trig, trighandler_t handler);
int sys_trigger_action(int trig, trig_action_t *act, trig_action_t *oldact);
int sys_trigger_return(unsigned int ebx, unsigned int ecx, unsigned int esi, unsigned int edi, trap_frame_t *frame);
int sys_trigger_proc_mask(int how, trigset_t *set, trigset_t *oldset);
int sys_trigger_pending(trigset_t *set);

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
    trigger->actions[trig - 1].mask = ta->mask;
    trigdelset(&trigger->actions[trig - 1].mask, trigmask(TRIGHSOFT) | trigmask(TRIGPAUSE));
}

static inline void trigger_get_action(triggers_t *trigger, int trig, trig_action_t *ta)
{
    ta->flags = trigger->actions[trig - 1].flags;
    ta->handler = trigger->actions[trig - 1].handler;
    ta->mask = trigger->actions[trig - 1].mask;
}

#endif  /* _XBOOK_TRIGGER_H */
