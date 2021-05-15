#include <arch/plic.h>
#include <arch/interrupt.h>
#include <arch/plic.h>
#include <types.h>
#include <xbook/hardirq.h>

static void controller_enable(irqno_t irq)
{
    /* 允许irq对应的终端响应 */
    plic_enable_irq(irq);
}

static void controller_disable(irqno_t irq)   
{
    /* 关闭irq对应的终端响应 */
    plic_disable_irq(irq);
}

static unsigned int controller_install(irqno_t irq, void * arg)
{
	/* 往中断表中注册中断 */
    return irq_register_handler(irq, (interrupt_handler_t)arg);
}

static void controller_uninstall(irqno_t irq)
{
    /* 解除中断表注册 */
    irq_unregister_handler(irq);
}

static void controller_ack(irqno_t irq)
{
	/* 完成中断后需要进行应答 */
    plic_complete(irq);
}

/* kernel will use global variable：interrupt_controller */
interrupt_controller_t interrupt_controller = {
    .install = controller_install, 
	.uninstall = controller_uninstall,
	.enable = controller_enable,
	.disable = controller_disable,
	.ack = controller_ack,
};
