#include <arch/interrupt.h>
#include <xbook/debug.h>
#include <xbook/softirq.h>
#include <xbook/bitops.h>
#include <string.h>

task_assist_head_t task_assist_head;
task_assist_head_t high_task_assist_head;
static softirq_action_t softirq_actions[NR_SOFTIRQS];
static unsigned long softirq_evens;

static unsigned long softirq_get_evens()
{
    return softirq_evens;
}

static void softirq_set_evens(unsigned long evens)
{
    softirq_evens = evens;
}

void softirq_build(unsigned long softirq, void (*action)(softirq_action_t *))
{
    if (0 <= softirq && softirq < NR_SOFTIRQS) {
        softirq_actions[softirq].action = action;    
    }
}

void softirq_active(unsigned long softirq)
{
    if (0 <= softirq && softirq < NR_SOFTIRQS) {
        if (softirq_actions[softirq].action)
            softirq_evens |= (1 << softirq);  
    }
}

static void softirq_do_handle()
{
    softirq_action_t *action;
    unsigned long evens;
    int irq_redo = MAX_IRQ_REDO_COUNT;

    evens = softirq_get_evens();
redo:
    if (evens) {
        softirq_set_evens(0);
        action = &softirq_actions[0];
        interrupt_enable();
        do {
            if (evens & 1)
                action->action(action);
            action++;
            evens >>= 1;
        } while(evens);
        interrupt_disable();
        evens = softirq_get_evens();
        if (evens && --irq_redo)
            goto redo;
    }
}

void softirq_handle_in_interrupt()
{
    unsigned long evens;
    unsigned long flags;
    interrupt_save_state(flags);
    evens = softirq_get_evens();
    if (evens) 
        softirq_do_handle();
    interrupt_restore_state(flags);
}

void high_task_assist_schedule(task_assist_t *assist)
{
    if (!test_and_set_bit(HIGHTASK_ASSIST_SOFTIRQ, &assist->status)) {
        unsigned long flags;
        interrupt_save_state(flags);
        assist->next = high_task_assist_head.head;
        high_task_assist_head.head = assist;
        softirq_active(HIGHTASK_ASSIST_SOFTIRQ);
        interrupt_restore_state(flags);
    }
}

static void high_task_assist_action(softirq_action_t *action)
{
    task_assist_t *list;
    interrupt_disable();
    list = high_task_assist_head.head;
    high_task_assist_head.head = NULL;
    interrupt_enable();
    while (list != NULL) {
        task_assist_t *assist = list;
        list = list->next;
        if (!atomic_get(&assist->count)) {
            clear_bit(TASK_ASSIST_SCHED, &assist->status);
            assist->func(assist->data);
            continue;
        }
        interrupt_disable();
        assist->next = high_task_assist_head.head;
        high_task_assist_head.head = assist;
        softirq_active(HIGHTASK_ASSIST_SOFTIRQ);
        interrupt_enable();
    }
}

void task_assist_schedule(task_assist_t *assist)
{
    if (!test_and_set_bit(TASK_ASSIST_SOFTIRQ, &assist->status)) {
        unsigned long flags;
        interrupt_save_state(flags);
        assist->next = task_assist_head.head;
        task_assist_head.head = assist;
        softirq_active(TASK_ASSIST_SOFTIRQ);
        interrupt_restore_state(flags);
    }
}

static void task_assist_action(softirq_action_t *action)
{
    task_assist_t *list;
    interrupt_disable();
    list = task_assist_head.head;
    task_assist_head.head = NULL;
    interrupt_enable();
    while (list != NULL) {
        task_assist_t *assist = list;
        list = list->next;
        if (!atomic_get(&assist->count)) {
            clear_bit(TASK_ASSIST_SCHED, &assist->status);
            assist->func(assist->data);
        }
        interrupt_disable();
        assist->next = task_assist_head.head;
        task_assist_head.head = assist;
        softirq_active(TASK_ASSIST_SOFTIRQ);
        interrupt_enable();
    }
}

void softirq_init()
{
    softirq_evens = 0;
    high_task_assist_head.head = NULL;
    task_assist_head.head = NULL;
    /* 注册任务协助的行为 */
    softirq_build(HIGHTASK_ASSIST_SOFTIRQ, high_task_assist_action);
    softirq_build(TASK_ASSIST_SOFTIRQ, task_assist_action);
}
