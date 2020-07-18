#ifndef _X86_MEMORY_H
#define _X86_MEMORY_H


#define	flush_tlb_one(addr)	\
	__asm__ __volatile__	("invlpg	(%0)	\n\t"::"r"(addr):"memory")
/*

*/

#define flush_tlb()						\
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


#endif   /* _X86_MEMORY_H */
