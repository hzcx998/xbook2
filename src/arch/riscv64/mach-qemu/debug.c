#include <arch/debug.h>
#include <arch/config.h>
#include <arch/hw.h>
#include <arch/sbi.h>

void debug_putchar(char ch)
{
    sbi_console_putchar(ch);
}

void arch_debug_init()
{
    // do nothing
}
