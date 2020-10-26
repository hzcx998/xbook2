#ifndef _X86_CPU_H
#define _X86_CPU_H

#include <types.h>

/* CPU的数量 */
#define CPU_NR  1

#define cpu_lazy        __cpu_lazy
#define cpu_idle        __cpu_idle

static inline void arch_cpu_pause(void)
{
	__asm__ __volatile__ ("pause");
}

static inline void get_cpuid(unsigned int Mop,unsigned int Sop,unsigned int * a,unsigned int * b,unsigned int * c,unsigned int * d)
{
	__asm__ __volatile__ (
        "cpuid	\n\t"
        :"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d)
        :"0"(Mop),"2"(Sop)
    );
}

/**
 * 获取当前cpu的id
 */
static inline cpuid_t hal_cpu_cur_get_id()
{
    return 0x86; /* only support one cpu */
}


#endif  /* _X86_CPU_H */
