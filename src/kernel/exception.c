#include <xbook/exception.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <xbook/safety.h>
#include <errno.h>

void exception_manager_init(exception_manager_t *exception_manager)
{
    list_init(&exception_manager->exception_list);
    list_init(&exception_manager->catch_list);
    
    exception_manager->exception_number = 0;
    exception_manager->catch_number = 0;
    exception_manager->in_user_mode = 0;
    int i;
    for (i = 0; i < EXCEPTION_SETS_SIZE; i++) {
        exception_manager->exception_block[i] = 0;
        exception_manager->exception_catch[i] = 0;
        exception_manager->handlers[i] = NULL;
    }
    spinlock_init(&exception_manager->manager_lock);
}

void exception_manager_exit(exception_manager_t *exception_manager)
{
    exception_t *exp, *tmp;
    list_for_each_owner_safe (exp, tmp, &exception_manager->exception_list, list) {
        mem_free(exp);
    }
    exception_manager->exception_number = 0;
    exception_manager->in_user_mode = 0;
    list_for_each_owner_safe (exp, tmp, &exception_manager->catch_list, list) {
        mem_free(exp);
    }
    exception_manager->catch_number = 0;
}

exception_t *exception_create(uint32_t code, pid_t source, uint32_t flags)
{
    exception_t *exp = mem_alloc(sizeof(exception_t));
    if (!exp)
        return NULL;
    exp->code = code;
    exp->source = source;
    exp->flags = flags;
    list_init(&exp->list);
    return exp;
}

static bool exception_was_blocked(exception_manager_t *exception_manager, uint32_t code)
{
    if (code == EXP_CODE_FINALHIT || code == EXP_CODE_STOP)
        return false;
    bool blocked = exception_manager->exception_block[code / 32] & (1 << (code % 32));
    return blocked;
}

static bool exception_can_catch(exception_manager_t *exception_manager, uint32_t code)
{
    if (code == EXP_CODE_FINALHIT || code == EXP_CODE_STOP)
        return false;
    bool catch = exception_manager->exception_catch[code / 32] & (1 << (code % 32));
    return catch;
}

static int exception_enable_catch(exception_manager_t *exception_manager, uint32_t code)
{
    if (code == EXP_CODE_FINALHIT || code == EXP_CODE_STOP)
        return -1;
    unsigned long flags;
    spin_lock_irqsave(&exception_manager->manager_lock, flags);
    exception_manager->exception_catch[code / 32] |= (1 << (code % 32));
    spin_unlock_irqrestore(&exception_manager->manager_lock, flags);
    return 0;
}

static int exception_disable_catch(exception_manager_t *exception_manager, uint32_t code)
{
    if (code == EXP_CODE_FINALHIT || code == EXP_CODE_STOP)
        return -1;
    unsigned long flags;
    spin_lock_irqsave(&exception_manager->manager_lock, flags);
    exception_manager->exception_catch[code / 32] &= ~(1 << (code % 32));
    spin_unlock_irqrestore(&exception_manager->manager_lock, flags);
    return 0;
}

void exception_enable_block(exception_manager_t *exception_manager, uint32_t code)
{
    if (code == EXP_CODE_FINALHIT || code == EXP_CODE_STOP)
        return;
    unsigned long flags;
    spin_lock_irqsave(&exception_manager->manager_lock, flags);
    exception_manager->exception_block[code / 32] |= (1 << (code % 32));
    spin_unlock_irqrestore(&exception_manager->manager_lock, flags);
}

void exception_disable_block(exception_manager_t *exception_manager, uint32_t code)
{
    if (code == EXP_CODE_FINALHIT || code == EXP_CODE_STOP)
        return;
    unsigned long flags;
    spin_lock_irqsave(&exception_manager->manager_lock, flags);
    exception_manager->exception_block[code / 32] &= ~(1 << (code % 32));
    spin_unlock_irqrestore(&exception_manager->manager_lock, flags);
}

void exception_add_normal(exception_manager_t *exception_manager, exception_t *exp)
{
    list_add_tail(&exp->list, &exception_manager->exception_list);
    exception_manager->exception_number++;
}

void exception_del_normal(exception_manager_t *exception_manager, exception_t *exp)
{
    assert(list_find(&exp->list, &exception_manager->exception_list));
    list_del_init(&exp->list);
    exception_manager->exception_number--;
}

void exception_add_catch(exception_manager_t *exception_manager, exception_t *exp)
{
    list_add_tail(&exp->list, &exception_manager->catch_list);
    exception_manager->catch_number++;
}

void exception_del_catch(exception_manager_t *exception_manager, exception_t *exp)
{
    assert(list_find(&exp->list, &exception_manager->catch_list));
    list_del_init(&exp->list);
    exception_manager->catch_number--;
}

int exception_copy(exception_manager_t *dest, exception_manager_t *src) 
{
    exception_t *tmp;
    list_for_each_owner (tmp, &src->exception_list, list) {
        exception_t *exp = exception_create(tmp->code, tmp->source, tmp->flags);
        if (!exp) {
            exception_manager_exit(dest);
            return -1;
        }
        exception_add_normal(dest, exp);
    }
    list_for_each_owner (tmp, &src->catch_list, list) {
        exception_t *exp = exception_create(tmp->code, tmp->source, tmp->flags);
        if (!exp) {
            exception_manager_exit(dest);
            return -1;
        }
        exception_add_catch(dest, exp);
    }
    return 0;
}

bool exception_filter(task_t *target, uint32_t code)
{
    if (code == EXP_CODE_CONT) {
        if (TASK_WAS_STOPPED(target)) {
            task_wakeup(target);       
        }
        return true;
    }
    return false;
}

/**
 * 只发送异常到队列上，但是不激活该异常。
 */
int exception_send(pid_t pid, uint32_t code)
{
    if (code >= EXP_CODE_MAX_NR || pid < 0)
        return -EINVAL;
    task_t *target = task_find_by_pid(pid); 
    if (!target)
        return -ESRCH;
    exception_manager_t *exception_manager = &target->exception_manager;
    unsigned long irq_flags;
    spin_lock_irqsave(&exception_manager->manager_lock, irq_flags);
    if (exception_was_blocked(exception_manager, code) || exception_manager->in_user_mode) {
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        return -EPERM;
    }
    if (exception_can_catch(exception_manager, code)) {
        exception_t *exp = exception_create(code, task_get_pid(task_current), 0);
        if (!exp) {
            spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
            return -ENOMEM; 
        }
        exception_add_catch(exception_manager, exp);
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        task_wakeup(target);
        return 0;
    }
    if (exception_filter(target, code)) {
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        return 0;
    }
    exception_t *exp = exception_create(code, task_get_pid(task_current), 0);
    if (!exp) {
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        return -ENOMEM; 
    }
    exception_add_normal(exception_manager, exp);
    spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
    task_wakeup(target);
    return 0;
}

int exception_force(pid_t pid, uint32_t code)
{
    task_t *target = task_find_by_pid(pid); 
    if (!target)
        return -ESRCH;
    exception_manager_t *exception_manager = &target->exception_manager;
    exception_disable_block(exception_manager, code);
    return exception_send(pid, code);
}

int exception_force_self(uint32_t code)
{
    return exception_force(sys_get_pid(), code);
}

int exception_raise(uint32_t code)
{
    return exception_send(sys_get_pid(), code);
}

bool exception_cause_exit(exception_manager_t *exception_manager)
{
    unsigned long iflags;
    spin_lock_irqsave(&exception_manager->manager_lock, iflags);
    exception_t *exp;
    list_for_each_owner (exp, &exception_manager->exception_list, list) {
        if (exp->code != EXP_CODE_CHLD &&
            exp->code != EXP_CODE_USER &&
            exp->code != EXP_CODE_STOP &&
            exp->code != EXP_CODE_CONT &&
            exp->code != EXP_CODE_TRAP &&
            exp->code != EXP_CODE_ALRM) {
            spin_unlock_irqrestore(&exception_manager->manager_lock, iflags);
            return true;
        }
    }
    list_for_each_owner (exp, &exception_manager->catch_list, list) {
        if (exp->code != EXP_CODE_CHLD &&
            exp->code != EXP_CODE_USER &&
            exp->code != EXP_CODE_STOP &&
            exp->code != EXP_CODE_CONT &&
            exp->code != EXP_CODE_TRAP &&
            exp->code != EXP_CODE_ALRM) {
            spin_unlock_irqrestore(&exception_manager->manager_lock, iflags);
            return true;
        }
    }
    spin_unlock_irqrestore(&exception_manager->manager_lock, iflags);
    return false;
}

static int exception_dispatch(exception_manager_t *exception_manager, exception_t *exp)
{
    task_t *cur = task_current;
    switch (exp->code) {
    case EXP_CODE_CHLD:
    case EXP_CODE_USER:
    case EXP_CODE_ALRM:
        // Ignore
        break;
    case EXP_CODE_STOP:
        cur->exit_status = -exp->code;
        task_block(TASK_STOPPED);
        break;
    case EXP_CODE_TRAP:
        // TODO: add trace trap code here.
        break;
    default:
        sys_exit(-exp->code);
        break;
    }
    return 0;
}

int exception_check_kernel(trap_frame_t *frame)
{
    exception_manager_t *exception_manager = &task_current->exception_manager;
    unsigned long irq_flags;
    spin_lock_irqsave(&exception_manager->manager_lock, irq_flags);
    if (!exception_manager->exception_number) {
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        return -1;
    }
    assert(!list_empty(&exception_manager->exception_list));
    while (1) {
        exception_t *exp = list_first_owner_or_null(&exception_manager->exception_list, exception_t, list);
        if (!exp)
            break;
        exception_del_normal(exception_manager, exp);
        exception_t tmp_exp = *exp;
        mem_free(exp);
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        exception_dispatch(exception_manager, &tmp_exp);
        spin_lock_irqsave(&exception_manager->manager_lock, irq_flags);
    }
    spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
    return 0;
}

static int exception_handle(exception_manager_t *exception_manager, exception_t *exp, trap_frame_t *frame)
{
    exception_handler_t handler = exception_manager->handlers[exp->code];
    if (handler) {
        exception_disable_catch(exception_manager, exp->code);
        exception_frame_build(exp->code, handler, frame);
        exception_manager->in_user_mode = 1;
        exception_manager->handlers[exp->code] = NULL;
    }
    return 0;
}

int exception_check_user(trap_frame_t *frame)
{
    exception_manager_t *exception_manager = &task_current->exception_manager;
    unsigned long irq_flags;
    spin_lock_irqsave(&exception_manager->manager_lock, irq_flags);
    if (!exception_manager->catch_number || exception_manager->in_user_mode) { /* 如果已经在用户态就不能再处理用户态的异常，避免嵌套 */
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        return -1;
    }
    assert(!list_empty(&exception_manager->catch_list));
    exception_t *exp = list_first_owner(&exception_manager->catch_list, exception_t, list);
    exception_del_catch(exception_manager, exp);
    spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
    exception_t tmp_exp = *exp;
    mem_free(exp);
    exception_handle(exception_manager, &tmp_exp, frame);
    return 0;
}

/* called in interrupt env. */
void exception_check(trap_frame_t *frame)
{
    exception_check_kernel(frame);
    exception_check_user(frame);
}

int sys_expsend(pid_t pid, uint32_t code)
{
    return exception_send(pid, code);
}

int sys_expcatch(uint32_t code, exception_handler_t handler)
{
    if (code >= EXP_CODE_MAX_NR)
        return -EINVAL;
    if (safety_check_range(handler, PAGE_SIZE) < 0)
        return -EINVAL;

    exception_manager_t *exception_manager = &task_current->exception_manager;
    if (handler) {
        if (!exception_enable_catch(exception_manager, code)) 
            exception_manager->handlers[code] = handler;
    } else {
        if (!exception_disable_catch(exception_manager, code))
            exception_manager->handlers[code] = NULL;    
    }
    return 0;
}

int sys_expblock(uint32_t code, uint32_t state)
{
    if (code >= EXP_CODE_MAX_NR)
        return -EINVAL;
    exception_manager_t *exception_manager = &task_current->exception_manager;
    if (state) {
        exception_enable_block(exception_manager, code);
    } else {
        exception_disable_block(exception_manager, code);
    }
    return 0;
}

int sys_excetion_return(unsigned int ebx, unsigned int ecx, unsigned int esi, unsigned int edi, trap_frame_t *frame)
{
    return exception_return(frame);
}
