#ifndef _RISCV64_CPU_H
#define _RISCV64_CPU_H

#include <types.h>

#define CPU_NR_MAX  2

void cpu_do_nothing();


#define cpu_idle        cpu_do_nothing

cpuid_t cpu_get_my_id();

#endif  /* _RISCV64_CPU_H */
