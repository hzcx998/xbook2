#include <sys/mman.h>
#include <sys/syscall.h>

int msync(void *start, size_t len, int flags)
{
	return syscall(SYS_MSYNC, start, len, flags);
}
