#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <sys/srvcall.h>
#include <srv/filesrv.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/filedes.h>

int stat(const char *path, struct stat *buf)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_STAT, 0);
    SETSRV_ARG(&srvarg, 1, full_path, strlen(full_path) + 1);
    SETSRV_ARG(&srvarg, 2, buf, sizeof(struct stat));
    SETSRV_IO(&srvarg, (SRVIO_USER << 2));
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int chmod(const char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_CHMOD, 0);
    SETSRV_ARG(&srvarg, 1, full_path, strlen(full_path) + 1);
    SETSRV_ARG(&srvarg, 2, mode, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int filestat(int fd, struct stat *buf)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FSTAT, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
    SETSRV_ARG(&srvarg, 2, buf, sizeof(struct stat));
    SETSRV_IO(&srvarg, (SRVIO_USER << 2));
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int fstat(int fd, struct stat *buf)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    if (_FILE_FLAGS(_fil) & _FILE_NORMAL) {
        return filestat(fd, buf);
    }
    return -1;
}