#include <arch/pic.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <xbook/hardirq.h>

void pic_init(void)
{
    /* mask all interrupts */
	ioport_out8(PIC_MASTER_CTLMASK,  0xff);
	ioport_out8(PIC_SLAVE_CTLMASK,   0xff);

	ioport_out8(PIC_MASTER_CTL,      0x11);
	ioport_out8(PIC_MASTER_CTLMASK,  0x20);
	ioport_out8(PIC_MASTER_CTLMASK,  1 << 2);
	ioport_out8(PIC_MASTER_CTLMASK,  0x01);

    ioport_out8(PIC_SLAVE_CTL,       0x11);
	ioport_out8(PIC_SLAVE_CTLMASK,   0x28);
	ioport_out8(PIC_SLAVE_CTLMASK,   2);
	ioport_out8(PIC_SLAVE_CTLMASK,   0x01);

	/* mask all interrupts */
	ioport_out8(PIC_MASTER_CTLMASK,  0xff);
	ioport_out8(PIC_SLAVE_CTLMASK,   0xff);
}

static void pic_enable(irqno_t irq)
{
    if(irq < 8){    /* clear master */
        ioport_out8(PIC_MASTER_CTLMASK, ioport_in8(PIC_MASTER_CTLMASK) & ~(1 << irq));
    } else {
        /* clear irq 2 first, then clear slave */
        ioport_out8(PIC_MASTER_CTLMASK, ioport_in8(PIC_MASTER_CTLMASK) & ~(1 << IRQ2_CONNECT));    
        ioport_out8(PIC_SLAVE_CTLMASK, ioport_in8(PIC_SLAVE_CTLMASK) & ~ (1 << (irq - 8)));
    }
}

static void pic_disable(irqno_t irq)   
{
	if(irq < 8){    /* set master */ 
        ioport_out8(PIC_MASTER_CTLMASK, ioport_in8(PIC_MASTER_CTLMASK) | (1 << irq));
    } else {
        /* set slave */
        ioport_out8(PIC_SLAVE_CTLMASK, ioport_in8(PIC_SLAVE_CTLMASK) | (1 << (irq - 8)));
    }
}

static unsigned int pic_install(irqno_t irq, void * arg)
{
	irq_register_handler((char )irq, (interrupt_handler_t)arg);
	return 1;
}

static void pic_uninstall(irqno_t irq)
{
	irq_unregister_handler((char )irq);
}

static void pic_ack(irqno_t irq)
{
	if (irq >= 8) { /* slaver */
		ioport_out8(PIC_SLAVE_CTL,  PIC_EIO);
	}
	ioport_out8(PIC_MASTER_CTL,  PIC_EIO);
}

/* kernel will use var：interrupt_controller */
interrupt_controller_t interrupt_controller = {
    .install = pic_install, 
	.uninstall = pic_uninstall,
	.enable = pic_enable,
	.disable = pic_disable,
	.ack = pic_ack,
};

int interrupt_do_irq(trap_frame_t *frame)
{
    irqno_t irq;
	/* 中断向量号减去异常占用的向量号，就是IRQ号 */
	irq = frame->vec_no - 0x20;
	if (!irq_handle(irq, frame)) {
		return -1;
	}
	return 0;
}