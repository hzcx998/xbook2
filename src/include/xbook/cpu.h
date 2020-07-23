#ifndef _XBOOK_CPU_H
#define _XBOOK_CPU_H

#include <arch/cpu.h>

static inline void cpu_pause(void)
{
	arch_cpu_pause();
}


#endif   /* _XBOOK_CPU_H */
