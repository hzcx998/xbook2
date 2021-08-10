#include <unistd.h>
#include <sys/syscall.h>

void sync(void)
{
	syscall(SYS_SYNC);
}
