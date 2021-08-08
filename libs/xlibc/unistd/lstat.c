#include <sys/stat.h>
#include <fcntl.h>

int lstat(const char *restrict path, struct stat *restrict buf)
{
#ifdef SYS_FSTATAT
	return fstatat(AT_FDCWD, path, buf, AT_SYMLINK_NOFOLLOW);
#else
	return -1;
#endif
}
