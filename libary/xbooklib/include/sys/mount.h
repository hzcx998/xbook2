#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

int mount(
    const char *source,
    const char *target,
    const char *fstype,
    unsigned long flags
);

int unmount(const char *source, unsigned long flags);

int mkfs(
    const char *source,
    const char *fstype,
    unsigned long flags
);

#endif  /* _SYS_MOUNT_H */