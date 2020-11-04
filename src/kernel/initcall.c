#include <xbook/initcall.h>
#include <xbook/debug.h>

extern initcall_t __initcall_start[];
extern initcall_t __initcall_end[];
extern exitcall_t __exitcall_start[];
extern exitcall_t __exitcall_end[];

void initcalls_exec(void)
{
	initcall_t * call;

	call =  &(*__initcall_start);
	while(call < &(*__initcall_end))
	{
		(*call)();
		call++;
	}
    printk(KERN_INFO "do init call done.\n");
}

void exitcalls_exec(void)
{
	exitcall_t * call;

	call =  &(*__exitcall_start);
	while(call < &(*__exitcall_end))
	{
		(*call)();
		call++;
	}
}
