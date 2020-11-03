#ifndef _XBOOK_HARDIRQ_H
#define _XBOOK_HARDIRQ_H

#include <arch/interrupt.h>
#include <types.h>

#define IRQF_DISABLED       0x01
#define IRQF_SHARED         0x02
#define IRQF_TIMER          0x03

#define IRQ_HANDLED        0
#define IRQ_NEXTONE        -1

typedef struct {
    void (*enable)(irqno_t irq);
    void (*disable)(irqno_t irq);
    unsigned int (*install)(irqno_t irq, void *arg);
    void (*uninstall)(irqno_t irq);
    void (*ack)(irqno_t irq);
} interrupt_controller_t;

extern interrupt_controller_t interrupt_controller;

typedef int (*irq_handler_t)(irqno_t, void *);

#define irq_enable(n) interrupt_controller.enable(n)
#define irq_disable(n) interrupt_controller.disable(n)

typedef struct irq_action {
    void *data;
    irq_handler_t handler;
    unsigned long flags;
    struct irq_action *next;
    char *name;
} irq_action_t;

typedef struct irq_description {
    interrupt_controller_t *controller;
    struct irq_action *action;
    unsigned long flags;
    atomic_t device_count;
    char *irqname;
} irq_description_t;

int irq_register(irqno_t irq,
    irq_handler_t handler, 
    unsigned long flags,
    char *irqname,
    char *devname,
    void *data);
int irq_unregister(irqno_t irq, void *data);
int irq_handle(irqno_t irq, trap_frame_t *frame);
void irq_description_init();

#endif  /* _XBOOK_HARDIRQ_H */
