#include <arch/memory.h>
#include <arch/plic.h>
#include <arch/cpu.h>
#include <arch/interrupt.h>
#include <arch/sbi.h>
#include <xbook/debug.h>
#include <k210_phymem.h>

//
// the riscv Platform Level Interrupt Controller (PLIC).
//
void plic_init(void)
{
    #ifdef QEMU
    int hart = cpu_get_my_id();
    /* set this hart's S-mode priority threshold to 0. 
    0意味着不会在S模式产生中断 
    每个hart的第0个优先级置0 */
    write32(PLIC_SPRIORITY(hart), 0);
    #endif
}

void plic_enable_irq(irqno_t irqno)
{
    if (irqno < 0 || irqno >= IRQ_MAX_NR) {
        panic("[IRQ] plic enable bad irq %d!", irqno);
    }
    int hart = cpu_get_my_id();

    /* 打开PLIC上的S模式中断使能 */

    /* ENABLE是每位一个中断源 */
    uint32_t *enable_addr;
    #ifdef QEMU
    enable_addr = (uint32_t *)PLIC_SENABLE(hart);
    #else
    enable_addr = (uint32_t *)PLIC_MENABLE(hart);
    #endif  /* QEMU */

    enable_addr += irqno / 32;  // 指向(irqno / 32)个4字节地址
    uint8_t irqno_off = irqno % 32;
    // set uart's enable bit for this hart's S-mode. 
    write32(enable_addr, read32(enable_addr) | (1 << irqno_off));
    
    /* 设置中断对应的优先级，优先级是每个4字节一个中断源 */
    uint32_t *priority_addr = (uint32_t *) PLIC_PRIORITY;
    priority_addr += irqno;
    write32(priority_addr, 1);  // 设置优先级为1
}

void plic_disable_irq(irqno_t irqno)
{
    if (irqno < 0 || irqno >= IRQ_MAX_NR)
        panic("[IRQ] plic enable bad irq %d!", irqno);
    int hart = cpu_get_my_id();
    
    /* 打开PLIC上的S模式中断使能 */
    uint32_t *enable_addr;
    #ifdef QEMU
    enable_addr = (uint32_t *)PLIC_SENABLE(hart);
    #else
    enable_addr = (uint32_t *)PLIC_MENABLE(hart);
    #endif  /* QEMU */

    enable_addr += irqno / 32;  // 指向(irqno / 32)个4字节地址
    uint8_t irqno_off = irqno % 32;
    // set uart's enable bit for this hart's S-mode. 
    write32(enable_addr, read32(enable_addr) & ~(1 << irqno_off)); 

    /* 设置中断对应的优先级，优先级是每个4字节一个中断源 */
    uint32_t *priority_addr = (uint32_t *) PLIC_PRIORITY;
    priority_addr += irqno;
    write32(priority_addr, 0);  // 设置优先级为0
}

// ask the PLIC what interrupt we should serve.
int plic_claim(void)
{
    int hart = cpu_get_my_id();
    int irq;
    #ifndef QEMU
    irq = *(uint32_t*)PLIC_MCLAIM(hart);
    #else
    irq = *(uint32_t*)PLIC_SCLAIM(hart);
    #endif
    return irq;
}

// tell the PLIC we've served this IRQ.
void plic_complete(int irq)
{
    int hart = cpu_get_my_id();
    #ifndef QEMU
    *(uint32_t*)PLIC_MCLAIM(hart) = irq;
    #else
    *(uint32_t*)PLIC_SCLAIM(hart) = irq;
    #endif

    #ifndef QEMU 
    w_sip(r_sip() & ~2);    // clear pending bit
    sbi_set_mie();
    #endif
}

int interrupt_do_irq(trap_frame_t *frame)
{
    irqno_t irq = plic_claim();
    if (irq < 0 || irq >= IRQ_MAX_NR) {
        errprintln("[irq] bad irq number %d!\n", irq);
        return -1;
    }
    if (!irq)   // irq=0 => no irq
        return -1;
    if (!irq_handle(irq, frame)) {
		return -1;
	}
    return 0;
}