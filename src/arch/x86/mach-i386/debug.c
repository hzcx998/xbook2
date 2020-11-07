#include <arch/debug.h>
#include <arch/config.h>
#include <arch/hw.h>

void debug_putchar(char ch)
{
#ifdef X86_CONSOLE_HW
    console_putchar(ch);
#endif /* X86_CONSOLE_HW */

#ifdef X86_SERIAL_HW
    serial_putchar(ch);
#endif /* X86_SERIAL_HW */
}

void init_kernel_debug()
{
    // 初始化控制台
	init_console_hw();

#ifdef X86_SERIAL_HW
    // 初始化串口
    init_serial_hw();
#endif /* X86_SERIAL_HW */
}
