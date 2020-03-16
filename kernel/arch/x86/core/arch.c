#include "arch.h"
#include <xbook/debug.h>

int init_arch()
{	
    /* 第一件事情就是初始化调试 */
	init_kernel_debug();
    printk("hello, xbook!\n");
	return 0;
}
