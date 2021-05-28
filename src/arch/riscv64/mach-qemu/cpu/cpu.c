#include <arch/cpu.h>
#include <arch/riscv.h>

void cpu_do_nothing()
{

}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
cpuid_t cpu_get_my_id()
{
    int id = r_tp();
    return id;
}

cpuid_t cpu_attached_list[CPU_NR_MAX];

void cpu_get_attached_list(cpuid_t *cpu_list, unsigned int *count)
{
    cpu_list[0] = cpu_get_my_id();
    *count = 1;
}

void cpu_init()
{
    int i;
    for (i = 0; i < CPU_NR_MAX; i++) {
        cpu_attached_list[i] = i;
    }
}

/* TODO: make udelay better */
void cpu_do_udelay(int usec)
{
    int i, j;
    for (i = 0; i < 10 * usec; i++)
        for (j = 0; j < 100; j++);
}