#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <sys/syscall.h>

int mount(
    const char *source,
    const char *target,
    const char *fstype,
    unsigned long flags,
    void *data
) {
    if (source == NULL || target == NULL || fstype == NULL)
        return -1;
    return syscall5(int, SYS_MOUNT, source, target, fstype, flags, data);
}

int unmount(const char *source, unsigned long flags)
{
    if (source == NULL)
        return -1;

    return syscall2(int, SYS_UNMOUNT, source, flags);
}

int mkfs(
    const char *source,
    const char *fstype,
    unsigned long flags
) {
    if (source == NULL || fstype == NULL)
        return -1;
    return syscall3(int, SYS_MKFS, source, fstype, flags);
}
