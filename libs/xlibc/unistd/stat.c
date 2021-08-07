#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/syscall.h>

int stat(const char *path, struct stat *buf)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);
    const char *p = (const char *) full_path;

    return syscall2(int, SYS_STAT, p, buf);
}

int chmod(const char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);
    const char *p = (const char *) full_path;

    return syscall2(int, SYS_CHMOD, p, mode);
}