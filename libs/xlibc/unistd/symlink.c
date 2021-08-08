#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>

int symlink(const char *existing, const char *new)
{
#ifdef SYS_SYMLINK
	return syscall(SYS_SYMLINK, existing, new);
#else
	return syscall(SYS_SYMLINKAT, existing, AT_FDCWD, new);
#endif
}
