#include <xbook/initcall.h>

extern initcall_t __initcall_start[];
extern initcall_t __initcall_end[];
extern exitcall_t __exitcall_start[];
extern exitcall_t __exitcall_end[];

void do_initcalls(void)
{
	initcall_t * call;

	call =  &(*__initcall_start);
	while(call < &(*__initcall_end))
	{
		(*call)();
		call++;
	}
}

void do_exitcalls(void)
{
	exitcall_t * call;

	call =  &(*__exitcall_start);
	while(call < &(*__exitcall_end))
	{
		(*call)();
		call++;
	}
}
