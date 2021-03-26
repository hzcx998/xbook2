#ifndef _X86_CPU_H
#define _X86_CPU_H

#include <types.h>

#define CPU_NR_MAX  1

cpuid_t cpu_get_my_id();
void cpu_get_attached_list(cpuid_t *cpu_list, unsigned int *count);
void cpu_init();

void cpu_do_sleep();
#define cpu_do_nothing() __asm__ __volatile__("nop")
void cpu_do_udelay(int usec);
__attribute__((always_inline)) static inline void cpu_do_pause(void)
{
	__asm__ __volatile__ ("pause");
}
static inline void cpu_do_cpuid(unsigned int mop,unsigned int sop,unsigned int *a,
        unsigned int *b,unsigned int *c,unsigned int *d)
{
	__asm__ __volatile__ (
        "cpuid	\n\t"
        :"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d)
        :"0"(mop),"2"(sop)
    );
}

#define cpu_sleep       cpu_do_sleep
#define cpu_idle        cpu_do_nohing
#define cpu_pause       cpu_do_pause
#define udelay          cpu_do_udelay

#endif  /* _X86_CPU_H */
