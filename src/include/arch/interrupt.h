#ifndef _ARCH_INTERRUPT_H
#define _ARCH_INTERRUPT_H

#include "general.h"
#include "atomic.h"

#define enable_intr     __enable_intr
#define disable_intr    __disable_intr

#define save_intr     __save_intr
#define restore_intr    __restore_intr

/* ----中断上半部分---- */
/* IRQ 编号 */
enum {
    IRQ0 = 0,
    IRQ1,
    IRQ2,
    IRQ3,
    IRQ4,
    IRQ5,
    IRQ6,
    IRQ7,
    IRQ8,
    IRQ9,
    IRQ10,
    IRQ11,
    IRQ12,
    IRQ13,
    IRQ14,
    IRQ15,
    NR_IRQS
};

#define IRQF_DISABLED       0x01
#define IRQF_SHARED         0x02
#define IRQF_TIMER          0x03

/* hardware interrupt controller */
struct hardware_intr_controller {
    void (*enable)(unsigned int irq);
    void (*disable)(unsigned int irq);
    unsigned int (*install)(unsigned int irq, void *arg);
    void (*uninstall)(unsigned int irq);
    /* 接收到中断后确定中断已经接收 */ 
    void (*ack)(unsigned int irq);
};

/* irq对应的处理 */
typedef struct irq_action {
    unsigned long data;
    int (*handler)(unsigned long, unsigned long);
    unsigned long flags;
    struct irq_action *next;     // 指向下一个行动
    /* 表示设备名字 */
    char *name;
} irq_action_t;

typedef struct irq_description {
    /* 硬件控制器，用来控制中断的硬件底层操作 */
    struct hardware_intr_controller *controller;
    struct irq_action *action;
    unsigned long flags;
    atomic_t device_count;       // 记录注册的设备数量

    /* 表示irq名字 */
    char *irqname;
} irq_description_t;

int register_irq(unsigned long irq,
    int (*handler)(unsigned long, unsigned long), 
    unsigned long flags,
    char *irqname,
    char *devname,
    unsigned long data);
int unregister_irq(unsigned long irq, void *data);

int handle_irq(unsigned long irq, trap_frame_t *frame);

void init_irq_description();

#endif  /* _ARCH_INTERRUPT_H */
