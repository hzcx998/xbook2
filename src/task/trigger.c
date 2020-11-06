#include <xbook/trigger.h>
#include <xbook/task.h>
#include <xbook/process.h>
#include <xbook/debug.h>
#include <xbook/syscall.h>
#include <xbook/schedule.h>
#include <arch/interrupt.h>
#include <arch/task.h>
#include <gui/message.h>

int do_active_trigger(pid_t pid, int trig, pid_t toucher)
{
    if (TRIGGER_IS_BAD(trig))
        return -1;
    if (toucher < 0)
        return -1;    
    task_t *task = task_find_by_pid(pid);
    if (task == NULL)
        return -1;
    if (task->triggers == NULL)
        return -1;
    triggers_t *trigger = task->triggers;
    unsigned long flags;
    spin_lock_irqsave(&trigger->trig_lock, flags);
    switch (trig)
    {
    case TRIGPAUSE:
        trigdelset(&trigger->blocked, TRIGRESUM);
        break;
    case TRIGRESUM:
    case TRIGHSOFT:    
        if (task->state == TASK_STOPPED) {
            task_wakeup(task);
        }
        task->exit_status = 0;
        trigdelset(&trigger->blocked, TRIGPAUSE);
        break;
    default:
        break;
    }

    if (trigismember(&trigger->set, trig))
        goto out;
    trigger->touchers[trig - 1] = toucher;
    trigaddset(&trigger->set, trig);
    if (!trigismember(&trigger->blocked, trig)) {
        trigger->flags |= TRIG_LEFT;
        task_wakeup(task);
    }
out:
    spin_unlock_irqrestore(&trigger->trig_lock, flags);
    trigger_calc_left(trigger);

    return 0;
}

int trigger_force(int trig, pid_t pid)
{
    if (TRIGGER_IS_BAD(trig))
        return -1;
    if (pid < 0)
        return -1;
    task_t *task = task_find_by_pid(pid);
    if (task == NULL)
        return -1;
    if (task->triggers == NULL)
        return -1;
    
    triggers_t *trigger = task->triggers;
    unsigned long flags;
    spin_lock_irqsave(&trigger->trig_lock, flags);
    if (trigger->actions[trig - 1].handler == TRIG_IGN)
        trigger->actions[trig - 1].handler = TRIG_DFL;
    trigdelset(&trigger->blocked, trig);
    spin_unlock_irqrestore(&trigger->trig_lock, flags);
    return do_active_trigger(pid, trig, task_current->pid);
}

int sys_trigger_active(int trig, pid_t pid)
{
    return do_active_trigger(pid, trig, task_current->pid);
}

int sys_trigger_handler(int trig, trighandler_t handler)
{
    if (TRIGGER_IS_BAD(trig))
        return -1;
    if (trig == TRIGHSOFT || trig == TRIGPAUSE)
        return -1;
    triggers_t *trigger = task_current->triggers;
    if (trigger == NULL)
        return -1;
    trig_action_t ta;
    ta.handler = handler;
    ta.flags = TA_ONCSHOT | TA_NOMASK;
    ta.mask = 0;

    unsigned long flags;
    spin_lock_irqsave(&trigger->trig_lock, flags);
    if (handler > 0) {
        trigger_set_action(trigger, trig, &ta);
    }
    spin_unlock_irqrestore(&trigger->trig_lock, flags);
    return 0;
}
int sys_trigger_action(int trig, trig_action_t *act, trig_action_t *oldact)
{
    if (TRIGGER_IS_BAD(trig))
        return -1;
    if (trig == TRIGHSOFT || trig == TRIGPAUSE)
        return -1;
    triggers_t *trigger = task_current->triggers;
    if (trigger == NULL)
        return -1;
    trig_action_t ta;
    unsigned long flags;
    spin_lock_irqsave(&trigger->trig_lock, flags);
    if (oldact) {
        trigger_get_action(trigger, trig, &ta);
        *oldact = ta;
    }
    if (act) {
        ta = *act;
        trigger_set_action(trigger, trig, &ta);
    }
    spin_unlock_irqrestore(&trigger->trig_lock, flags);
    return 0;
}

int sys_trigger_return(unsigned int ebx, unsigned int ecx, unsigned int esi, unsigned int edi, trap_frame_t *frame)
{
    return trigger_return_to_user(frame);
}

static int handle_trigger(trap_frame_t *frame, int trig)
{
    triggers_t *trigger = task_current->triggers;
    trig_action_t *act = &trigger->actions[trig - 1];
    trigger_frame_build(frame, trig, act);
    if (act->flags & TA_ONCSHOT) {
        act->handler = TRIG_DFL;
    }
    if (!(act->flags & TA_NODEFFER)) {
        spin_lock_irq(&trigger->trig_lock);
        trigorset(&trigger->blocked, &act->mask);
        trigaddset(&trigger->blocked, trig);
        trigger_calc_left(trigger);
        spin_unlock_irq(&trigger->trig_lock);
    }
    return 0;
}

int interrupt_do_trigger(trap_frame_t *frame)
{
    task_t *cur = task_current;
    if (cur->triggers == NULL)
        return -1;
    triggers_t *trigger = cur->triggers;
    if (!(trigger->flags & TRIG_LEFT))
        return -1;
    trig_action_t *ta;
    int trig;
    char have_trig;
    unsigned long flags;
    interrupt_save_and_disable(flags);
    for (trig = 1; trig <= TRIG_MAX; trig++) {
        have_trig = trigismember(&trigger->set, trig) && !trigismember(&trigger->blocked, trig);
        if (have_trig) {
            trigdelset(&trigger->set, trig);
            trigger->touchers[trig - 1] = -1;
            trigger_calc_left(trigger);
            ta = &trigger->actions[trig - 1];
            if (ta->handler == TRIG_IGN) {
                continue;   
            }
            if (ta->handler == TRIG_DFL) {
                if (cur->pid == USER_INIT_PROC_ID) {
                    continue;
                }
                switch (trig)
                {
                case TRIGUSR0:
                case TRIGUSR1:
                case TRIGRESUM: 
                case TRIGALARM:
                    continue;
                case TRIGPAUSE:
                    cur->exit_status = trig;
                    task_block(TASK_STOPPED);
                    continue;
                case TRIGDBG:
                    continue;
                case TRIGLSOFT:
                case TRIGHSOFT:
                case TRIGSYS:
                    trigaddset(&trigger->set, trig);
                    trigger_calc_left(trigger);
                    cur->exit_status = trig;
                    sys_exit(trig);
                    continue;
                default:
                    break;
                }
            }
            handle_trigger(frame, trig);
            break;
        }
    }
    interrupt_restore_state(flags);
    return 0;
}

int sys_trigger_proc_mask(int how, trigset_t *set, trigset_t *oldset)
{
    triggers_t *trigger = task_current->triggers;

    spin_lock_irq(&trigger->trig_lock);
    if (oldset != NULL) {
        *oldset = trigger->blocked;
    }
    if (how == TRIG_BLOCK) {
        if (set != NULL) {
            trigger->blocked |= *set;
            trigdelset(&trigger->blocked, TRIGHSOFT);
            trigdelset(&trigger->blocked, TRIGPAUSE);
        }
    } else if (how == TRIG_UNBLOCK) {
        if (set != NULL) {
            trigger->blocked &= ~*set;
        }
    } else if (how == TRIG_SETMASK) {
        if (set != NULL) {
            trigger->blocked = *set;
            trigdelset(&trigger->blocked, TRIGHSOFT);
            trigdelset(&trigger->blocked, TRIGPAUSE);
        }
    } else {
        spin_unlock_irq(&trigger->trig_lock);
        return -1;
    }
    spin_unlock_irq(&trigger->trig_lock);
    return 0;
}

int sys_trigger_pending(trigset_t *set)
{
    if (set == NULL)
        return -1;
    triggers_t *trigger = task_current->triggers;
    spin_lock_irq(&trigger->trig_lock);
    *set = trigger->set;
    spin_unlock_irq(&trigger->trig_lock);
    return 0;
}


void trigger_init(triggers_t *triggers)
{
    trigemptyset(&triggers->set);
    trigemptyset(&triggers->blocked);
    spinlock_init(&triggers->trig_lock);
    int i;
    for (i = 0; i < TRIG_NR; i++) {
        triggers->actions[i].handler = TRIG_DFL;
        triggers->actions[i].flags = 0;
        triggers->actions[i].mask = 0;
        triggers->touchers[i] = -1;
    }
    triggers->flags = 0;
}
