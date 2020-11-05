#include <arch/cpu.h>

cpuid_t cpu_attached_list[CPU_NR_MAX];

cpuid_t cpu_get_my_id()
{
    // TODO: calc cpuid 
    return cpu_attached_list[0];
}

void cpu_get_attached_list(cpuid_t *cpu_list, unsigned int *count)
{
    cpu_list[0] = cpu_get_my_id();
    *count = 1;
}

void cpu_init()
{
    int i;
    for (i = 0; i < CPU_NR_MAX; i++) {
        cpu_attached_list[i] = 0;
    }
    cpu_attached_list[0] = 0x80386;
}
