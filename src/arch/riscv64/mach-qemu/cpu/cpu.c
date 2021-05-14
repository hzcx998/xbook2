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