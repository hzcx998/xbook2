#ifndef _ARCH_CPU_H
#define _ARCH_CPU_H

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


#endif  /* _ARCH_CPU_H */
