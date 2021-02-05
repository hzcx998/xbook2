#include <xbook/initcall.h>
#include <xbook/debug.h>

extern initcall_t __initcall_start[];
extern initcall_t __initcall_end[];
extern exitcall_t __exitcall_start[];
extern exitcall_t __exitcall_end[];

void initcalls_exec(void)
{
	initcall_t * func =  &(*__initcall_start);
	for (;func < &(*__initcall_end); func++)
		(*func)();
    keprint(PRINT_INFO "do init call done.\n");
}

void exitcalls_exec(void)
{
    initcall_t * func =  &(*__exitcall_start);
	for (;func < &(*__exitcall_end); func++)
		(*func)();
}
