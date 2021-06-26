#ifndef __RISCV_H
#define __RISCV_H

#include <stdint.h>

#define RISCV_XLEN    64  /* 64位长度 */

#define SCAUSE_INTERRUPT    (1UL << (RISCV_XLEN - 1))

#define SCAUSE_S_SOFTWARE_INTR  1
#define SCAUSE_S_TIMER_INTR     5
#define SCAUSE_S_EXTERNAL_INTR  9

// which hart (core) is this?
static inline uint64 mhartid_read()
{
    uint64 x;
    asm volatile("csrr %0, mhartid" : "=r" (x) );
    return x;
}

// Machine Status Register, mstatus

#define MSTATUS_MPP_MASK (3L << 11) // previous mode.
#define MSTATUS_MPP_M (3L << 11)
#define MSTATUS_MPP_S (1L << 11)
#define MSTATUS_MPP_U (0L << 11)
#define MSTATUS_MIE (1L << 3)    // machine-mode interrupt enable.

static inline uint64 mstatus_read()
{
    uint64 x;
    asm volatile("csrr %0, mstatus" : "=r" (x) );
    return x;
}

static inline void mstatus_write(uint64 x)
{
    asm volatile("csrw mstatus, %0" : : "r" (x));
}

// machine exception program counter, holds the
// instruction address to which a return from
// exception will go.
static inline void mepc_write(uint64 x)
{
    asm volatile("csrw mepc, %0" : : "r" (x));
}

// Supervisor Status Register, sstatus

#define SSTATUS_SPP (1L << 8)  // Previous mode, 1=Supervisor, 0=User
#define SSTATUS_SPIE (1L << 5) // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE (1L << 4) // User Previous Interrupt Enable
#define SSTATUS_SIE (1L << 1)  // Supervisor Interrupt Enable
#define SSTATUS_UIE (1L << 0)  // User Interrupt Enable

static inline uint64 sstatus_read()
{
    uint64 x;
    asm volatile("csrr %0, sstatus" : "=r" (x) );
    return x;
}

static inline void sstatus_write(uint64 x)
{
    asm volatile("csrw sstatus, %0" : : "r" (x));
}

// Supervisor Interrupt Pending
static inline uint64 sip_read()
{
    uint64 x;
    asm volatile("csrr %0, sip" : "=r" (x) );
    return x;
}

static inline void sip_write(uint64 x)
{
    asm volatile("csrw sip, %0" : : "r" (x));
}

// Supervisor Interrupt Enable
#define SIE_SEIE (1L << 9) // external
#define SIE_STIE (1L << 5) // timer
#define SIE_SSIE (1L << 1) // software
static inline uint64 sie_read()
{
    uint64 x;
    asm volatile("csrr %0, sie" : "=r" (x) );
    return x;
}

static inline void sie_write(uint64 x)
{
    asm volatile("csrw sie, %0" : : "r" (x));
}

// Machine-mode Interrupt Enable
#define MIE_MEIE (1L << 11) // external
#define MIE_MTIE (1L << 7)  // timer
#define MIE_MSIE (1L << 3)  // software
static inline uint64 mie_read()
{
    uint64 x;
    asm volatile("csrr %0, mie" : "=r" (x) );
    return x;
}

static inline void mie_write(uint64 x)
{
    asm volatile("csrw mie, %0" : : "r" (x));
}

// machine exception program counter, holds the
// instruction address to which a return from
// exception will go.
static inline void sepc_write(uint64 x)
{
    asm volatile("csrw sepc, %0" : : "r" (x));
}

static inline uint64 sepc_read()
{
    uint64 x;
    asm volatile("csrr %0, sepc" : "=r" (x) );
    return x;
}

// Machine Exception Delegation
static inline uint64 medeleg_read()
{
    uint64 x;
    asm volatile("csrr %0, medeleg" : "=r" (x) );
    return x;
}

static inline void medeleg_write(uint64 x)
{
    asm volatile("csrw medeleg, %0" : : "r" (x));
}

// Machine Interrupt Delegation
static inline uint64 mideleg_read()
{
    uint64 x;
    asm volatile("csrr %0, mideleg" : "=r" (x) );
    return x;
}

static inline void mideleg_write(uint64 x)
{
    asm volatile("csrw mideleg, %0" : : "r" (x));
}

// Supervisor Trap-Vector Base Address
// low two bits are mode.
static inline void stvec_write(uint64 x)
{
    asm volatile("csrw stvec, %0" : : "r" (x));
}

static inline uint64 stvec_read()
{
    uint64 x;
    asm volatile("csrr %0, stvec" : "=r" (x) );
    return x;
}

// Machine-mode interrupt vector
static inline void mtvec_write(uint64 x)
{
    asm volatile("csrw mtvec, %0" : : "r" (x));
}

// use riscv's sv39 page table scheme.
#define SATP_SV39 (8L << 60)

/* bits: 0-43 */
#define SATP_SV39_PPN_MASK (0XFFFFFFFFFFF)

#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64)pagetable) >> 12))

/* 从SATP中解析出页表地址 */
#define SATP_PGTBL(satp) (((satp) & SATP_SV39_PPN_MASK) << 12)

// supervisor address translation and protection;
// holds the address of the page table.
static inline void satp_write(uint64 x)
{
    asm volatile("csrw satp, %0" : : "r" (x));
}

static inline uint64 satp_read()
{
    uint64 x;
    asm volatile("csrr %0, satp" : "=r" (x) );
    return x;
}

// Supervisor Scratch register, for early trap handler in trampoline.S.
static inline void sscratch_write(uint64 x)
{
    asm volatile("csrw sscratch, %0" : : "r" (x));
}

static inline void mscratch_write(uint64 x)
{
    asm volatile("csrw mscratch, %0" : : "r" (x));
}

// Supervisor Trap Cause
static inline uint64 scause_read()
{
    uint64 x;
    asm volatile("csrr %0, scause" : "=r" (x) );
    return x;
}

// Supervisor Trap Value
static inline uint64 stval_read()
{
    uint64 x;
    asm volatile("csrr %0, stval" : "=r" (x) );
    return x;
}

// Machine-mode Counter-Enable
static inline void mcounteren_write(uint64 x)
{
    asm volatile("csrw mcounteren, %0" : : "r" (x));
}

static inline uint64 mcounteren_read()
{
    uint64 x;
    asm volatile("csrr %0, mcounteren" : "=r" (x) );
    return x;
}

// supervisor-mode cycle counter
static inline uint64 time_reg_read()
{
    uint64 x;
    // asm volatile("csrr %0, time" : "=r" (x) );
    // this instruction will trap in SBI
    asm volatile("rdtime %0" : "=r" (x) );
    return x;
}

static inline uint64 sp_reg_read()
{
    uint64 x;
    asm volatile("mv %0, sp" : "=r" (x) );
    return x;
}

// read and write tp, the thread pointer, which holds
// this core's hartid (core number), the index into cpus[].
static inline uint64 tp_reg_read()
{
    uint64 x;
    asm volatile("mv %0, tp" : "=r" (x) );
    return x;
}

static inline void tp_reg_write(uint64 x)
{
    asm volatile("mv tp, %0" : : "r" (x));
}

static inline uint64 ra_reg_read()
{
    uint64 x;
    asm volatile("mv %0, ra" : "=r" (x) );
    return x;
}

static inline uint64 fp_reg_read()
{
    uint64 x;
    asm volatile("mv %0, s0" : "=r" (x) );
    return x;
}

// flush the TLB.
static inline void sfence_vma()
{
    // the zero, zero means flush all TLB entries.
    // asm volatile("sfence.vma zero, zero");
    asm volatile("sfence.vma");
}

#define tlb_flush() sfence_vma()

static inline void fence_i()
{
    asm volatile("fence.i");
}

static inline void wait_for_interrupt(void)
{
	__asm__ __volatile__ ("wfi");
}

#endif