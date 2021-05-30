#ifndef _RISCV64_INTERRUPT_H
#define _RISCV64_INTERRUPT_H

#include <arch/riscv.h>

#define IRQ_OFFSET	        16    /* IRQ在中断表中的偏移，最开始的用于系统异常 */
#define IRQ_MAX_NR          80
#define MAX_INTERRUPT_NR    (IRQ_OFFSET + IRQ_MAX_NR)    /* 最大支持的中断数量 */

#define NR_IRQS     IRQ_MAX_NR

enum {
    EP_INSTRUCTION_ADDRESS_MISALIGNED = 0,
    EP_INSTRUCTION_ACCESS_FAULT,
    EP_ILLEGAL_INSTRUCTION,
    EP_BREAKPOINT,
    EP_LOAD_ADDRESS_MISALIGNED,
    EP_LOAD_ACCESS_FAULT,
    EP_STORE_ADDRESS_MISALIGNED,
    EP_STORE_ACCESS_FAULT,
    EP_ENVIRONMENT_CALL_U_MODE,
    EP_ENVIRONMENT_CALL_S_MODE,
    EP_RESERVED10,
    EP_ENVIRONMENT_CALL_M_MODE,
    EP_INSTRUCTION_PAGE_FAULT,          /* page attr */
    EP_LOAD_PAGE_FAULT,                 /* read data */
    EP_RESERVED14,
    EP_STORE_PAGE_FAULT,                /* write data */
};

typedef struct {
    /*   0 */ uint64_t kernel_satp;   // kernel page table
    /*   8 */ uint64_t kernel_sp;     // top of process's kernel stack
    /*  16 */ uint64_t kernel_trap;   // usertrap()
    /*  24 */ uint64_t epc;           // saved user program counter
    /*  32 */ uint64_t kernel_hartid; // saved kernel tp
    /*  40 */ uint64_t ra;
    /*  48 */ uint64_t sp;
    /*  56 */ uint64_t gp;
    /*  64 */ uint64_t tp;
    /*  72 */ uint64_t t0;
    /*  80 */ uint64_t t1;
    /*  88 */ uint64_t t2;
    /*  96 */ uint64_t s0;
    /* 104 */ uint64_t s1;
    /* 112 */ uint64_t a0;
    /* 120 */ uint64_t a1;
    /* 128 */ uint64_t a2;
    /* 136 */ uint64_t a3;
    /* 144 */ uint64_t a4;
    /* 152 */ uint64_t a5;
    /* 160 */ uint64_t a6;
    /* 168 */ uint64_t a7;
    /* 176 */ uint64_t s2;
    /* 184 */ uint64_t s3;
    /* 192 */ uint64_t s4;
    /* 200 */ uint64_t s5;
    /* 208 */ uint64_t s6;
    /* 216 */ uint64_t s7;
    /* 224 */ uint64_t s8;
    /* 232 */ uint64_t s9;
    /* 240 */ uint64_t s10;
    /* 248 */ uint64_t s11;
    /* 256 */ uint64_t t3;
    /* 264 */ uint64_t t4;
    /* 272 */ uint64_t t5;
    /* 280 */ uint64_t t6;
} trap_frame_t;


typedef void (*interrupt_handler_t)(trap_frame_t *);

void interrupt_expection_init(void);

void interrupt_disable(void);
void interrupt_enable(void);
int irq_register_handler(int irq, interrupt_handler_t function);
int irq_unregister_handler(int irq);
void trap_frame_dump(trap_frame_t *frame);
void interrupt_dispatch(trap_frame_t *frame);
void trap_init(void);

// flags: 0-15: SSTATUS_SIE, 16-31: (SIE_SEIE | SIE_SSIE | SIE_STIE)
#define interrupt_save_and_disable(flags)                  \
    do {                                    \
        flags = (sstatus_read() & SSTATUS_SIE); \
        interrupt_disable();    \
    } while (0)

#define interrupt_restore_state(flags)               \
    do {                                    \
        sstatus_write(sstatus_read() | flags); \
    } while (0)

static inline int interrupt_enabled()
{
    uint64_t x = sstatus_read();
    return (x & SSTATUS_SIE) != 0;
}

// arch local 
void usertrap(void);
void usertrapret(void);
void forkret();

int page_do_fault(trap_frame_t *frame, int is_user, int expcode);

#endif  /* _RISCV64_INTERRUPT_H */
