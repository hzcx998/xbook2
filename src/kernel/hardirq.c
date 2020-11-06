#include <xbook/hardirq.h>
#include <xbook/memalloc.h>
#include <stddef.h>
#include <types.h>

irq_description_t irq_descriptions[NR_IRQS];

void irq_description_init()
{
    int i;
    irq_description_t *irq;
    for (i = 0; i < NR_IRQS; i++) {
        irq = &irq_descriptions[i];
        irq->action = NULL;
        irq->flags = 0;
        irq->controller = NULL;
        irq->irqname = NULL;
        irq->irqname = NULL;
        atomic_set(&irq->device_count, 0);
    }
}

static irq_description_t *irq_description_get(irqno_t irq)
{
    if (0 <= irq && irq < NR_IRQS) {
        return &irq_descriptions[irq];
    }
    return NULL;
}

int irq_register(irqno_t irq,
    irq_handler_t handler, 
    unsigned long flags,
    char *irqname,
    char *devname,
    void *data)
{
    irq_description_t *irq_desc = irq_description_get(irq);
    if (!irq_desc) 
        return -1;
    irq_desc->controller = &interrupt_controller;
    irq_desc->irqname = irqname;
    irq_desc->flags = flags;

    irq_action_t *action = mem_alloc(sizeof(irq_action_t));
    if (!action)
        return -1;
    action->data = data;
    action->flags = flags;
    action->handler = handler;
    action->name = devname;
    action->next = NULL;
    if (flags & IRQF_SHARED) {
        irq_action_t *p = irq_desc->action;
        if (p == NULL) {
            irq_desc->action = action;
        } else {            
            while (p->next) {
                p = p->next;
            }
            p->next = action;
        }
    } else {
        irq_desc->action = action;
    }
    atomic_inc(&irq_desc->device_count);
    irq_desc->controller->install(irq, handler);
    irq_desc->controller->enable(irq);
    return 0;
}

/**
 * irq_unregister - 注销中断
 * @irq: 中断号
 * @data: 注销时需要的数据
 * 
 * 注销中断，如果不是共享中断就直接关闭中断线，不然就减少中断数
 * 直到中断数为0就关闭中断线
 */
int irq_unregister(irqno_t irq, void *data)
{
    irq_description_t *irq_desc = irq_description_get(irq);
    if (!irq_desc) 
        return -1;

    unsigned long flags;
    interrupt_save_and_disable(flags);
    if (irq_desc->flags & IRQF_SHARED) {
        irq_action_t *p = irq_desc->action, *prev;
        do {
            prev = p;  
            if (p->data == data) {
                if (p == irq_desc->action) {
                    irq_desc->action = irq_desc->action->next;
                } else {
                    prev->next = p->next;
                }
                mem_free(p);
                break;
            }
            p = p->next;
        } while (p);
    } else {
        mem_free(irq_desc->action);
        irq_desc->action = NULL;
    }
    atomic_dec(&irq_desc->device_count);
    if (!atomic_get(&irq_desc->device_count)) {    
        irq_desc->controller->disable(irq);
        irq_desc->controller->uninstall(irq);
        irq_desc->controller = NULL;
        irq_desc->irqname = NULL;
        irq_desc->flags = 0;    
    }
    interrupt_restore_state(flags);
    return 0;
}

static int do_handle_action(irqno_t irq, irq_action_t *action)
{
    unsigned long retval = 0;
    if (action->flags & IRQF_DISABLED) {
        interrupt_disable();    
    }
    retval = action->handler(irq, action->data);
    if (action->flags & IRQF_DISABLED) {
        interrupt_enable();
    }
    return retval;
}

int irq_handle(irqno_t irq, trap_frame_t *frame)
{
    irq_description_t *irq_desc = irq_description_get(irq);
    if (!irq_desc) 
        return -1;
    irq_action_t *action = irq_desc->action;
    while (action)
    {
        if (do_handle_action(irq, action) == IRQ_HANDLED)
            break;
        action = action->next;
    }
    if (irq_desc->controller->ack) {
        irq_desc->controller->ack(irq);
    }
    return -1;
}
