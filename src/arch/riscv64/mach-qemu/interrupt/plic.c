#include <arch/memory.h>
#include <arch/plic.h>
#include <arch/cpu.h>
#include <k210_qemu_phymem.h>

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

void plicinit(void) {
  /* 设置某个中断的优先级 */
	write32(PLIC_V + DISK_IRQ * sizeof(uint32), 1);
	write32(PLIC_V + UART_IRQ * sizeof(uint32), 1);
}

void
plicinithart(void)
{
  int hart = cpu_get_my_id();
  /* 打开PLIC上的中断使能 */
  #ifdef QEMU
  // set uart's enable bit for this hart's S-mode. 
  *(uint32*)PLIC_SENABLE(hart)= (1 << UART_IRQ) | (1 << DISK_IRQ);
  // set this hart's S-mode priority threshold to 0.
  *(uint32*)PLIC_SPRIORITY(hart) = 0;
  #else
  uint32 *hart_m_enable = (uint32*)PLIC_MENABLE(hart);
  *(hart_m_enable) = read32(hart_m_enable) | (1 << DISK_IRQ);
  uint32 *hart0_m_int_enable_hi = hart_m_enable + 1;
  *(hart0_m_int_enable_hi) = read32(hart0_m_int_enable_hi) | (1 << (UART_IRQ % 32));
  #endif
}

// ask the PLIC what interrupt we should serve.
int
plic_claim(void)
{
  int hart = cpu_get_my_id();
  int irq;
  #ifndef QEMU
  irq = *(uint32*)PLIC_MCLAIM(hart);
  #else
  irq = *(uint32*)PLIC_SCLAIM(hart);
  #endif
  return irq;
}

// tell the PLIC we've served this IRQ.
void
plic_complete(int irq)
{
  int hart = cpu_get_my_id();
  #ifndef QEMU
  *(uint32*)PLIC_MCLAIM(hart) = irq;
  #else
  *(uint32*)PLIC_SCLAIM(hart) = irq;
  #endif
}

