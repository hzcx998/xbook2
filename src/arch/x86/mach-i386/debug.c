#include <arch/debug.h>
#include <arch/config.h>
#include <arch/hw.h>

void debug_putchar(char ch)
{
#ifdef X86_CONSOLE_HW
    console_hardware_putchar(ch);
#endif

#ifdef X86_SERIAL_HW
    serial_hardware_putchar(ch);
#endif
}

void arch_debug_init()
{
    // 默认都会初始化控制台
	console_hardware_init();

#ifdef X86_SERIAL_HW
    // 初始化串口
    serial_hardware_init();
#endif
}
