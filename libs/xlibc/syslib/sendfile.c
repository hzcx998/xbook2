#include <sys/sendfile.h>
#include <sys/syscall.h>

ssize_t sendfile(int out_fd, int in_fd, off_t *ofs, size_t count)
{
#ifdef SYS_SENDFILE
	return syscall(SYS_SENDFILE, out_fd, in_fd, ofs, count);
#else
	return -1;
#endif
}

weak_alias(sendfile, sendfile64);