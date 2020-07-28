#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <sys/srvcall.h>
#include <srv/filesrv.h>

int mount(
    const char *source,
    const char *target,
    const char *fstype,
    unsigned long flags
) {
    if (source == NULL || target == NULL || fstype == NULL)
        return -1;

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_MOUNT, 0);
    SETSRV_ARG(&srvarg, 1, source, strlen(source) + 1);
    SETSRV_ARG(&srvarg, 2, target, strlen(target) + 1);
    SETSRV_ARG(&srvarg, 3, fstype, strlen(fstype) + 1);
    SETSRV_ARG(&srvarg, 4, flags, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int unmount(const char *source, unsigned long flags)
{
    if (source == NULL)
        return -1;

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_UNMOUNT, 0);
    SETSRV_ARG(&srvarg, 1, source, strlen(source) + 1);
    SETSRV_ARG(&srvarg, 2, flags, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int mkfs(
    const char *source,
    const char *fstype,
    unsigned long flags
) {
    if (source == NULL || fstype == NULL)
        return -1;

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_MKFS, 0);
    SETSRV_ARG(&srvarg, 1, source, strlen(source) + 1);
    SETSRV_ARG(&srvarg, 2, fstype, strlen(fstype) + 1);
    SETSRV_ARG(&srvarg, 3, flags, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}
