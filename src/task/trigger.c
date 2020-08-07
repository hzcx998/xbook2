#include <xbook/trigger.h>
#include <xbook/task.h>
#include <xbook/process.h>
#include <xbook/debug.h>
#include <xbook/syscall.h>
#include <arch/interrupt.h>
#include <arch/task.h>

/* 调试触发器：0不调试，1要调试 */
#define DEBUG_LOCAL   0

/**
 * do_active_trigger - 激活任务的某个触发器
 * 
 */
int do_active_trigger(pid_t pid, int trig, pid_t toucher)
{
    if (IS_BAD_TRIGGER(trig))
        return -1;
    if (toucher < 0)
        return -1;
#if DEBUG_LOCAL == 1 
    printk(KERN_DEBUG "do_active_trigger: toucher=%d pid=%d tirgger:%d\n",
        toucher, pid, trig);
#endif        
    task_t *task = find_task_by_pid(pid);
    if (task == NULL)
        return -1;
    if (task->triggers == NULL)
        return -1;
    triggers_t *trigger = task->triggers;

    /* 保护触发器集的操作 */
    unsigned long flags;
    spin_lock_irqsave(&trigger->trig_lock, flags);
    switch (trig)
    {
    case TRIGPAUSE: 
        /* 如果是暂停，那么恢复就不能屏蔽 */
        trigdelset(&trigger->blocked, TRIGRESUM);
        break;
    case TRIGRESUM: /* 恢复进程运行 */
    case TRIGHSOFT: 
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "do_active_trigger: may wakeup not ready task.\n");
#endif        
        if (task->state == TASK_STOPPED) { /* 处于停止状态就resume */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "do_active_trigger: wakeup stopped task=%s.\n", task->name);
#endif
            task_wakeup(task);
        }
        task->exit_status = 0;
        trigdelset(&trigger->blocked, TRIGPAUSE);
        break;
    default:
        break;
    }

    /* 如果触发器已经激活，就不再激活 */
    if (trigismember(&trigger->set, trig))
        goto out;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "do_active_trigger: write trigger set.\n");
#endif
    /* 填写触发者 */
    trigger->touchers[trig - 1] = toucher;
    /* 在触发器集中设置触发器 */
    trigaddset(&trigger->set, trig);
    
    /* 如果没有阻塞，就唤醒任务 */
    if (!trigismember(&trigger->blocked, trig)) {
        trigger->flags |= TRIG_LEFT;
        task_wakeup(task);
    }

out:
    spin_unlock_irqrestore(&trigger->trig_lock, flags);
    trigger_calc_left(trigger);

    return 0;
}

/**
 * trigger_force - 强制激活触发器
 * @trig: 触发器
 * @pid: 要激活的进程
 * 
 * 强制激活，即让触发器的处理方式都是默认的处理方式。
 */
int trigger_force(int trig, pid_t pid)
{
    if (IS_BAD_TRIGGER(trig))
        return -1;
    if (pid < 0)
        return -1;
    task_t *task = find_task_by_pid(pid);
    if (task == NULL)
        return -1;
    if (task->triggers == NULL)
        return -1;
    
    triggers_t *trigger = task->triggers;
    unsigned long flags;
    spin_lock_irqsave(&trigger->trig_lock, flags);
    /* 若为忽略，则变成默认处理方式 */
    if (trigger->actions[trig - 1].handler == TRIG_IGN)
        trigger->actions[trig - 1].handler = TRIG_DFL;
    
    /* 如果触发器被屏蔽了，就需要解除屏蔽 */
    trigdelset(&trigger->blocked, trig);
    spin_unlock_irqrestore(&trigger->trig_lock, flags);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "trigger_force: start.\n");
#endif
    return do_active_trigger(pid, trig, current_task->pid);
}

/**
 * trigger_active - 激活一个触发器
 * @trig: 触发器
 * @pid: 需要激活触发器的任务
 * 
 * @return: 成功返回0，失败返回-1
 */
int sys_trigger_active(int trig, pid_t pid)
{

#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "trigger_active: start.\n");
#endif
    
    /* 激活触发器 */
    return do_active_trigger(pid, trig, current_task->pid);
}

/**
 * trigger_handler - 设置触发器的处理方式
 * @trig: 触发器
 * @handler: 处理方式
 * 
 * @return: 成功返回0，失败返回-1
*/
int sys_trigger_handler(int trig, trighandler_t handler)
{
    if (IS_BAD_TRIGGER(trig))
        return -1;
    if (trig == TRIGHSOFT || trig == TRIGPAUSE) /* 重软件和暂停不能够捕捉和忽略 */
        return -1;
    triggers_t *trigger = current_task->triggers;
    if (trigger == NULL)
        return -1;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "trigger_handler: trig=%d handler=%x\n", trig, handler);
#endif
    trig_action_t ta;
    ta.handler = handler;
    ta.flags = TA_ONCSHOT | TA_NOMASK;
    ta.mask = 0;

    unsigned long flags;
    spin_lock_irqsave(&trigger->trig_lock, flags);
    if (handler > 0) {
        /* 设置触发器行为 */
        trigger_set_action(trigger, trig, &ta);
    }
    spin_unlock_irqrestore(&trigger->trig_lock, flags);
    return 0;
}
/**
 * trigger_action - 设置触发器的处理方式
 * @trig: 触发器
 * @act: 新处理方式
 * @oldact: 保存旧方式的指针
 * 
 * 如果act不为NULL，就设置触发器的处理方式为act指定的内容
 * 如果oldact不为NULL，就把之前的触发器的处理方式为保存下来
 * 
 * @return: 成功返回0，失败返回-1
*/
int sys_trigger_action(int trig, trig_action_t *act, trig_action_t *oldact)
{
    if (IS_BAD_TRIGGER(trig))
        return -1;
    if (trig == TRIGHSOFT || trig == TRIGPAUSE) /* 重软件和暂停不能够捕捉和忽略 */
        return -1;
    triggers_t *trigger = current_task->triggers;
    if (trigger == NULL)
        return -1;
    trig_action_t ta;
    unsigned long flags;
    spin_lock_irqsave(&trigger->trig_lock, flags);
    if (oldact) { /* 如果旧行为指针为空，就先保存旧的行为指针 */
        trigger_get_action(trigger, trig, &ta);
        *oldact = ta;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "trigger_action: old trig=%d handler=%x flags=%x\n",
        trig, oldact->handler, oldact->flags);
#endif
    }
    if (act) {
        ta = *act;
        trigger_set_action(trigger, trig, &ta);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "trigger_action: new trig=%d handler=%x flags=%x\n",
        trig, act->handler, act->flags);
#endif
    }
    spin_unlock_irqrestore(&trigger->trig_lock, flags);
    return 0;
}

/**
 * sys_trigger_return - 执行完可捕捉触发器后返回
 * @frame: 栈
 * 
 */
int sys_trigger_return(unsigned int ebx, unsigned int ecx, unsigned int esi, unsigned int edi, trap_frame_t *frame)
{
    return trigger_return(frame);
}

/**
 * handle_trigger - 处理触发器
 * @frame: 中断栈框
 * @trig: 触发器
 * 
 * 处理用户可捕捉的触发器
 * 
 * 成功返回0，失败返回-1
 */
static int handle_trigger(trap_frame_t *frame, int trig)
{
    triggers_t *trigger = current_task->triggers;
    /* 获取触发器行为 */
    trig_action_t *act = &trigger->actions[trig - 1];

#if DEBUG_LOCAL == 1
    /* 处理自定义函数 */
    printk(KERN_DEBUG "handle_trigger: handle trig=%d frame %x.\n", trig, frame);
#endif    
    
    /* 构建用户触发器栈框，返回时就可以处理用户自定义函数 */
    build_trigger_frame(trig, act, frame);

    /* 执行完信号后需要把触发器行为设置为默认的行为 */
    if (act->flags & TA_ONCSHOT) {
        act->handler = TRIG_DFL;
    }
    /* 没有标志才设置 */
    if (!(act->flags & TA_NODEFFER)) {
        /* 设置屏蔽，添加对当前触发器的屏蔽，计算是否还有触发器剩余 */
        spin_lock_irq(&trigger->trig_lock);
        trigorset(&trigger->blocked, &act->mask);
        trigaddset(&trigger->blocked, trig);
        trigger_calc_left(trigger);
        spin_unlock_irq(&trigger->trig_lock);
    }

    return 0;
}

/**
 * do_trigger - 执行触发器
 * frame: 中断栈框 
 */
int do_trigger(trap_frame_t *frame)
{
    task_t *cur = current_task;
    /* 内核线程没有触发器 */
    if (cur->triggers == NULL)
        return -1;
    triggers_t *trigger = cur->triggers;
    /* 没有触发器需要处理 */
    if (!(trigger->flags & TRIG_LEFT))
        return -1;

    trig_action_t *ta;
    /* 查看每一个触发器，看是否激活 */
    int trig;

    char have_trig;
    unsigned long flags;
    save_intr(flags);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "do_trigger: pid=%d frame %x start.\n", cur->pid, frame);
#endif
    for (trig = 1; trig <= TRIGMAX; trig++) {
        have_trig = 0;
        have_trig = trigismember(&trigger->set, trig) && !trigismember(&trigger->blocked, trig);
        /* 如果触发器激活了，就处理 */
        if (have_trig) {
#if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "do_trigger: pid=%d trigger=%d\n", cur->pid, trig);
#endif
            /* 处理后置0 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "do_trigger: before set=%x flags=%x\n", trigger->set, trigger->flags);
#endif
            trigdelset(&trigger->set, trig);
            trigger->touchers[trig - 1] = -1;   /* clean toucher */
            trigger_calc_left(trigger);
            
            ta = &trigger->actions[trig - 1];
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "do_trigger: after set=%x flags=%x\n", trigger->set, trigger->flags);
#endif
            if (ta->handler == TRIG_IGN) { /* ignore trigger */
#if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "do_trigger: ignore trigger=%d\n", trig);
#endif
                continue;   
            }
            if (ta->handler == TRIG_DFL) {   /* default trigger */
                /* init process do nothing. */
                if (cur->pid == INIT_PROC_PID) {
#if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "do_trigger: trigger is invailed for init process :)\n");
#endif
                    continue;
                }
                    
                /* do default thing through different trigger */
                switch (trig)
                {
                case TRIGUSR0: /* user0 */
                case TRIGUSR1: /* user1 */
                case TRIGRESUM: /* 在设置恢复触发器的时候就唤醒任务了 */
#if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "do_trigger: user or resume.\n");
#endif
                    continue;   /* default ignore. */
                case TRIGPAUSE: /* pause */
#if DEBUG_LOCAL == 1
                printk(KERN_DEBUG "do_trigger: stop trigger=%d\n", trig);
#endif
                    cur->exit_status = trig;
                    
                    task_block(TASK_STOPPED);   /* 停止运行 */
                    continue;
                case TRIGDBG: /* debug */
                    continue; /* not support now */
                case TRIGLSOFT: /* light software */
                case TRIGHSOFT: /* heavy software */
                case TRIGSYS: /* system */
                case TRIGALARM: /* alarm */
#if DEBUG_LOCAL == 1
                    printk(KERN_DEBUG "do_trigger: software or system trigger=%d\n", trig);
#endif
                    /* 避免本次未能退出，故再添加一次触发 */
                    trigaddset(&trigger->set, trig);
                    trigger_calc_left(trigger);
                    cur->exit_status = trig;
                    sys_exit(trig); /* 退出后不会往后面运行 */
                    continue;
                default:
                    break;
                }
            }
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "do_trigger: handle user or light software trigger=%d\n", trig);
#endif
            /* 捕捉用户态程序 */
            handle_trigger(frame, trig);
            break;  /* 捕捉一个触发器后，立即返回 */
        }
    }
    restore_intr(flags);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "do_trigger: scan done.\n");
#endif
    return 0;
}


/**
 * sys_trigger_proc_mask - 设置进程的触发器阻塞集
 * @how: 怎么设置阻塞
 * @set: 阻塞集
 * @oldset: 旧阻塞集
 * 
 * 如果set不为空，就要设置新集，如果oldset不为空，就要把旧集复制过去
 * 
 * return: 成功返回0，失败返回-1
 */
int sys_trigger_proc_mask(int how, trigset_t *set, trigset_t *oldset)
{
    triggers_t *trigger = current_task->triggers;

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
    triggers_t *trigger = current_task->triggers;
    spin_lock_irq(&trigger->trig_lock);
    *set = trigger->set;
    spin_unlock_irq(&trigger->trig_lock);
    return 0;
}


void trigger_init(triggers_t *triggers)
{
    /* 触发器置空 */
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
