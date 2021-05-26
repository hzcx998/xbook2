#include <unistd.h>
#include <sys/syscall.h>

pid_t clone(int (*fn)(void *arg), void *arg, void *stack, 
            size_t stack_size, unsigned long flags)
{
    if (stack)
	    stack += stack_size;
    return syscall6(pid_t, SYS_CLONE, fn, stack, flags, NULL, NULL, NULL);
}