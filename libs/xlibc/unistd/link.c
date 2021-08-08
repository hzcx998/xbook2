#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>

int link(const char *existing, const char *new)
{
#ifdef SYS_LINK
	return syscall(SYS_LINK, existing, new);
#else
	return syscall(SYS_LINKAT, AT_FDCWD, existing, AT_FDCWD, new, 0);
#endif
}
