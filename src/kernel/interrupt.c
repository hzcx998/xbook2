#include <arch/interrupt.h>
#include <xbook/stddef.h>
#include <xbook/types.h>

/* var: hardware_intr_contorller must be support in arch */
extern struct hardware_intr_controller hardware_intr_contorller;

/* irq描述表，更深层次地表现中断 */
irq_description_t irq_description_table[NR_IRQS];

/** 
 *  init_irq_description - 初始化中断描述 
 */
void init_irq_description()
{
    int i;
    irq_description_t *irq;
    for (i = 0; i < NR_IRQS; i++) {
        irq = &irq_description_table[i];
        irq->action = NULL;
        irq->flags = 0;
        irq->controller = NULL;
        irq->irqname = NULL;
        irq->irqname = NULL;
        atomic_set(&irq->device_count, 0);
    }
}

/**
 * get_irq_description - 通过irq获取中断描述
 * @irq: 中断号
 * 
 * 获取中断描述表中的一项
 */
static irq_description_t *get_irq_description(unsigned long irq)
{
    /* 要是在范围内才返回表中的项 */
    if (0 <= irq && irq < NR_IRQS) {
        return &irq_description_table[irq];
    }
    return NULL;
}

/**
 * register_irq - 注册一个中断
 * @irq: 中断号
 * @handler: 中断处理函数
 * @flags: 注册时的标志
 * @irqname: 中断所在irq的名字
 * @devname: 设备名字（如果是共享中断，用于不同的行为的名字）
 * @data: 中断处理传入的数据
 * 
 * 注册一个中断，并且打开该线
 */
int register_irq(unsigned long irq,
    int (*handler)(unsigned long, unsigned long), 
    unsigned long flags,
    char *irqname,
    char *devname,
    unsigned long data)
{
    irq_description_t *irq_desc = get_irq_description(irq);
    if (!irq_desc) 
        return -1;
    
    /* 指定硬件控制器 */
    irq_desc->controller = &hardware_intr_contorller;
    
    /* 设置irq名字 */
    irq_desc->irqname = irqname;

    irq_desc->flags = flags;

    /* 创建一个中断行为 */
    irq_action_t *action = kmalloc(sizeof(irq_action_t));
    if (!action)
        return -1;
    /* 为行为结构体赋值 */
    action->data = data;
    action->flags = flags;
    action->handler = handler;
    action->name = devname;
    action->next = NULL;
    
    /* 如果是共享中断，就添加行为 */
    if (flags & IRQF_SHARED) {
        /* 把新的行为插入到最后面 */
        
        /* 指向行为头 */
        irq_action_t *p = irq_desc->action;
        
        /* 如果指针是空，说明还没有行为 */
        if (p == NULL) {
            /* 注册到第一个行为 */
            irq_desc->action = action;
            
        } else {            
            /* 如果有下一个，就指向下一个 */
            while (p->next) {
                p = p->next;
            }

            /* 现在action指向最后一个，把当前的行为追加到最后面 */
            p->next = action;
        }
    } else {
        /* 不是共享中断，就没有下一个行为 */
        irq_desc->action = action;

    }

    /* 增加一个设备记录 */
    atomic_inc(&irq_desc->device_count);

    /* 执行控制器的安装和打开中断操作 */
    irq_desc->controller->install(irq, handler);
    irq_desc->controller->enable(irq);

    return 0;
}


/**
 * unregister_irq - 注销中断
 * @irq: 中断号
 * @data: 注销时需要的数据
 * 
 * 注销中断，如果不是共享中断就直接关闭中断线，不然就减少中断数
 * 直到中断数为0就关闭中断线
 */
int unregister_irq(unsigned long irq, void *data)
{
    irq_description_t *irq_desc = get_irq_description(irq);
    if (!irq_desc) 
        return -1;

    /* 注销的时候不能产生中断 */
    unsigned long flags;
    save_intr(flags);

    /* 删除action */
    if (irq_desc->flags & IRQF_SHARED) {    
        /* 指向行为头 */
        irq_action_t *p = irq_desc->action, *prev;

        do {
            prev = p;
            /* 找到该设备 */        
            if (p->data == (unsigned long) data) {

                /* 如果是第一个就找到了 */
                if (p == irq_desc->action) {
                    irq_desc->action = irq_desc->action->next;
                } else {
                    /* 不是的话，就把自己从链表脱去，不影响链表头 */
                    prev->next = p->next;
                }
                
                /* 释放action */
                kfree(p);
                
                /* 释放完毕，退出 */
                break;
            }
            p = p->next;
        } while (p);
        
    } else {
        /* 释放action */
        kfree(irq_desc->action);
        /* 设置为空 */
        irq_desc->action = NULL;
    }

    atomic_dec(&irq_desc->device_count);
    
    /* 如果没有设备在这条中断线上，就关闭该中断 */
    if (!atomic_get(&irq_desc->device_count)) {    
        /* 执行控制器的卸载和关闭中断操作 */
        irq_desc->controller->disable(irq);
        irq_desc->controller->uninstall(irq);
        
        irq_desc->controller = NULL;
        irq_desc->irqname = NULL;
        irq_desc->flags = 0;    
    }

    /* 恢复之前的中断状态 */
    restore_intr(flags);
    return 0;
}

/** 
 * handle_action - 处理中断行为
 * @irq: irq号
 * @action: 具体要做的某一个行为
 */
static int handle_action(unsigned long irq, irq_action_t *action)
{
    unsigned long retval = 0;
    /* 如果有关闭中断标志。就关闭中断 */
    if (action->flags & IRQF_DISABLED) {
        disable_intr();    
    }
    
    /* 具体的行为处理 */
    retval = action->handler(irq, action->data);
    
    /* 如果有关闭中断标志。就打开中断 */
    if (action->flags & IRQF_DISABLED) {
        enable_intr();
    }
    return retval;
}

/**
 * handle_irq - 处理irq
 * @irq: irq号
 * @frame: 中断栈
 */
int handle_irq(unsigned long irq, trap_frame_t *frame)
{
    irq_description_t *irq_desc = get_irq_description(irq);
    if (!irq_desc) 
        return -1;
    
    /* 获取行为并执行行为 */
    irq_action_t *action = irq_desc->action;

    /* 处理中断行为 */
    while (action)
    {
        handle_action(irq, action);

        /* 指向下一个行为 */
        action = action->next;
    }
    /* 进行应答 */    
    if (irq_desc->controller->ack) {
        irq_desc->controller->ack(irq);
    }

    return -1;
}
