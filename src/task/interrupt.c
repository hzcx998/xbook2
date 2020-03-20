/*
 * file:		kernel/device/interrupt.c
 * auther:	    Jason Hu
 * time:		2019/8/17
 * copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
 */

#include <book/interrupt.h>
#include <book/arch.h>
#include <book/debug.h>
#include <lib/string.h>

/* 从平台架构中导出一种处理硬件中断的控制方法 */
EXTERN struct HardwareIntController picHardwareIntContorller;

/* irq描述表，更深层次地表现中断 */
struct IrqDescription irqDescriptionTable[NR_IRQS];

/** 
 *  InitIrqDescription - 初始化中断描述 
 */
PUBLIC INIT void InitIrqDescription()
{
    int i;
    struct IrqDescription *irq;
    for (i = 0; i < NR_IRQS; i++) {
        irq = &irqDescriptionTable[i];
        irq->action = NULL;
        irq->flags = 0;
        irq->controller = NULL;
        irq->irqname = NULL;
        irq->irqname = NULL;
        AtomicSet(&irq->deviceCount, 0);
    }

}

/**
 * GetIrqDescription - 通过irq获取中断描述
 * @irq: 中断号
 * 
 * 获取中断描述表中的一项
 */
PRIVATE struct IrqDescription *GetIrqDescription(unsigned int irq)
{
    /* 要是在范围内才返回表中的项 */
    if (0 <= irq && irq < NR_IRQS) {
        return &irqDescriptionTable[irq];
    }
    return NULL;
}

/**
 * RegisterIRQ - 注册一个中断
 * @irq: 中断号
 * @handler: 中断处理函数
 * @flags: 注册时的标志
 * @irqname: 中断所在irq的名字
 * @devname: 设备名字（如果是共享中断，用于不同的行为的名字）
 * @data: 中断处理传入的数据
 * 
 * 注册一个中断，并且打开该线
 */
PUBLIC int RegisterIRQ(unsigned int irq,
    void (*handler)(unsigned int irq, unsigned int data), 
    unsigned int flags,
    char *irqname,
    char *devname,
    unsigned int data)
{
    struct IrqDescription *irqDesc = GetIrqDescription(irq);
    if (!irqDesc) 
        return -1;
    
    /* 指定硬件控制器 */
    irqDesc->controller = &picHardwareIntContorller;
    
    /* 设置irq名字 */
    irqDesc->irqname = irqname;

    irqDesc->flags = flags;

    /* 创建一个中断行为 */
    struct IrqAction *action = kmalloc(sizeof(struct IrqAction), GFP_KERNEL);
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
        struct IrqAction *p = irqDesc->action;
        
        /* 如果指针是空，说明还没有行为 */
        if (p == NULL) {
            /* 注册到第一个行为 */
            irqDesc->action = action;
            
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
        irqDesc->action = action;

    }

    /* 增加一个设备记录 */
    AtomicInc(&irqDesc->deviceCount);

    /* 执行控制器的安装和打开中断操作 */
    irqDesc->controller->install(irq, handler);
    irqDesc->controller->enable(irq);

    return 0;
}


/**
 * UnregisterIRQ - 注销中断
 * @irq: 中断号
 * @data: 注销时需要的数据
 * 
 * 注销中断，如果不是共享中断就直接关闭中断线，不然就减少中断数
 * 直到中断数为0就关闭中断线
 */
PUBLIC int UnregisterIRQ(unsigned int irq, unsigned int data)
{
    struct IrqDescription *irqDesc = GetIrqDescription(irq);
    if (!irqDesc) 
        return -1;

    /* 注销的时候不能产生中断 */
    unsigned long flags = InterruptSave();
    
    /* 删除action */
    if (irqDesc->flags & IRQF_SHARED) {    
        /* 指向行为头 */
        struct IrqAction *p = irqDesc->action, *prev;

        do {
            prev = p;
            /* 找到该设备 */        
            if (p->data == data) {

                /* 如果是第一个就找到了 */
                if (p == irqDesc->action) {
                    irqDesc->action = irqDesc->action->next;
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
        kfree(irqDesc->action);
        /* 设置为空 */
        irqDesc->action = NULL;
    }

    AtomicDec(&irqDesc->deviceCount);
    
    /* 如果没有设备在这条中断线上，就关闭该中断 */
    if (!AtomicGet(&irqDesc->deviceCount)) {    
        /* 执行控制器的卸载和关闭中断操作 */
        irqDesc->controller->disable(irq);
        irqDesc->controller->uninstall(irq);
        
        irqDesc->controller = NULL;
        irqDesc->irqname = NULL;
        irqDesc->flags = 0;    
    }

    /* 恢复之前的中断状态 */
    InterruptRestore(flags);
    return 0;
}

/** 
 * HandleAction - 处理中断行为
 * @irq: irq号
 * @action: 具体要做的某一个行为
 */
PRIVATE int HandleAction(unsigned int irq, struct IrqAction *action)
{
    
    /* 如果有关闭中断标志。就关闭中断 */
    if (action->flags & IRQF_DISABLED) {
        DisableInterrupt();    
    }
    
    /* 具体的行为处理 */
    action->handler(irq, action->data);
    
    /* 如果有关闭中断标志。就打开中断 */
    if (action->flags & IRQF_DISABLED) {
        EnableInterrupt();
    }
    return 0;
}


/**
 * HandleIRQ - 处理irq
 * @irq: irq号
 * @frame: 中断栈
 */
PUBLIC int HandleIRQ(unsigned int irq, struct TrapFrame *frame)
{
    struct IrqDescription *irqDesc = GetIrqDescription(irq);
    if (!irqDesc) 
        return -1;
    
    /* 获取行为并执行行为 */
    struct IrqAction *action = irqDesc->action;

    /* 处理中断行为 */
    while (action)
    {
        HandleAction(irq, action);

        /* 指向下一个行为 */
        action = action->next;
    }
    /* 进行应答 */    
    if (irqDesc->controller->ack) {
        irqDesc->controller->ack(irq);
    }

    return -1;
}
