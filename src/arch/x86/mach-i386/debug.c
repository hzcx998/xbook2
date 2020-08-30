#include <arch/debug.h>
#include <arch/config.h>

/**
 * init_kernel_debug
 * 
 */

//define "debug_putchar"
void (*debug_putchar)(char ch);

void init_kernel_debug()
{

#if CONFIG_DEBUG_METHOD == 1
    // 初始化控制台
	init_console_debug();
    debug_putchar = &console_putchar;
#elif CONFIG_DEBUG_METHOD == 2
    // 初始化串口
    init_serial_debug();
    debug_putchar = &serial_putchar;
#endif /* CONFIG_DEBUG_METHOD */
}
