#include <xbook/exception.h>
#include <xbook/schedule.h>
#include <xbook/process.h>
#include <errno.h>

void exception_manager_init(exception_manager_t *exception_manager)
{
    INIT_LIST_HEAD(&exception_manager->exception_list);
    exception_manager->exception_number = 0;
    int i;
    for (i = 0; i < EXCEPTION_BLOCK_SIZE; i++) {
        exception_manager->exception_block[i] = 0;
    }
    spinlock_init(&exception_manager->manager_lock);
}

void exception_manager_exit(exception_manager_t *exception_manager)
{
    exception_t *exp, *tmp;
    list_for_each_owner_safe (exp, tmp, &exception_manager->exception_list, list) {
        mem_free(exp);
    }
}

exception_t *exception_create(uint32_t code, pid_t source, uint32_t arg, uint32_t flags)
{
    exception_t *exp = mem_alloc(sizeof(exception_t));
    if (!exp)
        return NULL;
    exp->code = code;
    exp->source = source;
    exp->arg = arg;
    exp->flags = flags;
    INIT_LIST_HEAD(&exp->list);
    return exp;
}

static bool exception_was_blocked(exception_manager_t *exception_manager, uint32_t code)
{
    unsigned long flags;
    spin_lock_irqsave(&exception_manager->manager_lock, flags);
    int i, j = 0;
    for (i = 0; i < EXCEPTION_BLOCK_SIZE; i++) {
        for (j = 0; j < 32; j++) {
            if (exception_manager->exception_block[i] & (1 << j) && 
                ((i * 32 + j) == code)) {
                return true;
            }
        } 
    }
    spin_unlock_irqrestore(&exception_manager->manager_lock, flags);
    return false;
}

void exception_add(exception_manager_t *exception_manager, exception_t *exp)
{
    list_add_tail(&exp->list, &exception_manager->exception_list);
    exception_manager->exception_number++;
}

void exception_del(exception_manager_t *exception_manager, exception_t *exp)
{
    ASSERT(list_find(&exp->list, &exception_manager->exception_list));
    list_del_init(&exp->list);
    exception_manager->exception_number--;
}

int exception_copy(exception_manager_t *dest, exception_manager_t *src) 
{
    exception_t *tmp;
    list_for_each_owner (tmp, &src->exception_list, list) {
        exception_t *exp = exception_create(tmp->code, tmp->source, tmp->arg, tmp->flags);
        if (!exp) {
            exception_manager_exit(dest);
            return -1;
        }
        exception_add(dest, exp);
    }
    return 0;
}

int exception_send(pid_t pid, uint32_t code, uint32_t arg)
{
    if (code >= EXP_CODE_MAX_NR || pid < 0)
        return -EINVAL;
    task_t *target = task_find_by_pid(pid); 
    if (!target)
        return -ESRCH;
    exception_manager_t *exception_manager = &target->exception_manager;
    if (exception_was_blocked(exception_manager, code)) {
        return 0; 
    }
    unsigned long irq_flags;
    spin_lock_irqsave(&exception_manager->manager_lock, irq_flags);
    exception_t *exp = exception_create(code, task_get_pid(task_current), arg, 0);
    if (!exp) {
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        return -ENOMEM; 
    }
    exception_add(exception_manager, exp);
    spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
    return 0;
}

static int exception_catch(exception_manager_t *exception_manager, exception_t *exp)
{
    task_t *cur = task_current;
    switch (exp->code) {
    case EXP_CODE_USER:
        printk("excetpion: task %d:%s catch %d: user exception.\n", cur->pid, cur->name, exp->code);
        // TODO: call user exception handler.
        break;
    case EXP_CODE_INTR:
        printk("excetpion: task %d:%s catch %d: interrupt exception.\n", cur->pid, cur->name, exp->code);
        // EXIT run
        sys_exit(-EXP_CODE_INTR);
        break;
    default:
        break;
    }
    return 0;
}

int exception_check(trap_frame_t *frame)
{
    exception_manager_t *exception_manager = &task_current->exception_manager;
    unsigned long irq_flags;
    spin_lock_irqsave(&exception_manager->manager_lock, irq_flags);
    if (!exception_manager->exception_number) {
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        return -1; 
    }
    ASSERT(!list_empty(&exception_manager->exception_list));
    while (1) {
        exception_t *exp = list_first_owner_or_null(&exception_manager->exception_list, exception_t, list);
        if (!exp)
            break;
        exception_del(exception_manager, exp);
        spin_unlock_irqrestore(&exception_manager->manager_lock, irq_flags);
        
        exception_catch(exception_manager, exp);
        mem_free(exp);
        spin_lock_irqsave(&exception_manager->manager_lock, irq_flags);
    }
}
