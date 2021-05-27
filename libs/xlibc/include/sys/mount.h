#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

int mount(
    const char *source,
    const char *target,
    const char *fstype,
    unsigned long flags,
    void *data
);

int unmount(const char *source, unsigned long flags);

#define umount(source) unmount((source), 0)

int mkfs(
    const char *source,
    const char *fstype,
    unsigned long flags
);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_MOUNT_H */