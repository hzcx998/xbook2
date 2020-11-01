#ifndef _X86_CPU_H
#define _X86_CPU_H

#include <types.h>

/* CPU的数量 */
#define CPU_NR  1

static inline void cpu_do_pause(void)
{
	__asm__ __volatile__ ("pause");
}

void cpu_do_sleep();
void cpu_do_nohing(void);

#define cpu_sleep       cpu_do_sleep
#define cpu_idle        cpu_do_nohing
#define cpu_pause       cpu_do_pause

void cpu_do_udelay(int usec);

static inline void cpu_do_cpuid(unsigned int mop,unsigned int sop,unsigned int *a,unsigned int *b,unsigned int *c,unsigned int *d)
{
	__asm__ __volatile__ (
        "cpuid	\n\t"
        :"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d)
        :"0"(mop),"2"(sop)
    );
}

static inline cpuid_t hal_cpu_get_id()
{
    return 0x86; /* only support one cpu */
}

#define udelay              cpu_do_udelay

#endif  /* _X86_CPU_H */
