#ifndef _X86_INSTRUCTION_H
#define _X86_INSTRUCTION_H

void __invlpg_asm(unsigned int addr);
#define __invlpg_inline(vaddr) asm volatile ("invlpg %0"::"m" (vaddr):"memory")

/* __invlpg_asm or __invlpg_inline */
#define flush_tbl(vaddr)    __invlpg_inline(vaddr)

void __cpuid(unsigned int id_eax, unsigned int *eax, 
    unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
void __cpu_lazy();
void __cpu_idle(void);

char __xchg8(char *ptr, char value);
short __xchg16(short *ptr, short value);
int __xchg32(int *ptr, int value);

/* x86特性 */
#define X86_FEATURE_XMM2 (0*32+26) /* Streaming SIMD Extensions-2 */

/*
* Alternative instructions for different CPU types or capabilities.
*
* This allows to use optimized instructions even on generic binary
* kernels.
*
* length of oldinstr must be longer or equal the length of newinstr
* It can be padded with nops as needed.
*
* For non barrier like inlines please define new variants
* without volatile and memory clobber.
*/
#define alternative(oldinstr, newinstr, feature) \
asm volatile ("661:\n\t" oldinstr "\n662:\n" \
      ".section .altinstructions,\"a\"\n" \
      "   .align 4\n" \
      "   .long 661b\n"          /* label */ \
      "   .long 663f\n"    /* new instruction */ \
      "   .byte %c0\n"          /* feature bit */ \
      "   .byte 662b-661b\n"    /* sourcelen */ \
      "   .byte 664f-663f\n"    /* replacementlen */ \
      ".previous\n" \
      ".section .altinstr_replacement,\"ax\"\n" \
      "663:\n\t" newinstr "\n664:\n" /* replacement */\
      ".previous" :: "i" (feature) : "memory")

#define mb() alternative("lock; addl $0,0(%%esp)", "mfence", X86_FEATURE_XMM2)
#define rmb() alternative("lock; addl $0,0(%%esp)", "lfence", X86_FEATURE_XMM2)

#ifdef CONFIG_X86_OOSTORE
/* Actually there are no OOO store capable CPUs for now that do SSE, 
but make it already an possibility. */
#define wmb() Alternative("lock; addl $0,0(%%esp)", "sfence", X86_FEATURE_XMM)
#else
#define wmb() __asm__ __volatile__ ("": : :"memory")
#endif

/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")


#endif  /* _X86_INSTRUCTION_H */
