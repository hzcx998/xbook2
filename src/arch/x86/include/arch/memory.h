#ifndef _X86_MEMORY_H
#define _X86_MEMORY_H


#define	tlb_flush_one(addr)	\
	__asm__ __volatile__	("invlpg	(%0)	\n\t"::"r"(addr):"memory")
/*

*/

#define tlb_flush()						\
do								\
{								\
	unsigned long	tmpreg;					\
	__asm__ __volatile__ 	(				\
				"movl	%%cr3,	%0	\n\t"	\
				"movl	%0,	%%cr3	\n\t"	\
				:"=r"(tmpreg)			\
				:				\
				:"memory"			\
				);				\
}while(0)

char mem_xchg8(char *ptr, char value);
short mem_xchg16(short *ptr, short value);
int mem_xchg32(int *ptr, int value);

#define xchg(ptr,v) ((__typeof__(*(ptr)))mem_xchg((unsigned int) \
        (v),(ptr),sizeof(*(ptr))))

/**
 * __Xchg: 交换一个内存地址和一个数值的值
 * @x: 数值
 * @ptr: 内存指针
 * @size: 地址值的字节大小
 * 
 * 返回交换前地址中的值
 */
static inline unsigned int  mem_xchg(unsigned int x, 
        volatile void * ptr, int size)
{
    int old;
    switch (size) {
        case 1:
            old = mem_xchg8((char *)ptr, x);
            break;
        case 2:
            old = mem_xchg16((short *)ptr, x);
            break;
        case 4:
            old = mem_xchg32((int *)ptr, x);
            break;
    }
    return old;
}


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


unsigned int phy_mem_get_size_from_hardware();

#endif   /* _X86_MEMORY_H */
