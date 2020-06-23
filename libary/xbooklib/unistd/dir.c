#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/srvcall.h>
#include <srv/filesrv.h>

/* 任务可以打开的目录数量 */
#define _MAX_DIRDES_NR     32

struct _dirdes __dirdes_table[_MAX_DIRDES_NR] = {{0, -1}, }; 

#if 0
/* default work dir */
static char __current_work_dir[MAX_PATH_LEN] = {'c', ':', 0, };
#endif
static struct _dirdes *__alloc_dirdes()
{
    int i;
    for (i = 0; i < _MAX_DIRDES_NR; i++) {
        if (__dirdes_table[i].flags == 0) {
            __dirdes_table[i].flags = 1;
            __dirdes_table[i].diridx = -1;
            return &__dirdes_table[i];
        }
    }
    return NULL;
}

static void __free_dirdes(struct _dirdes *_dir)
{
    _dir->flags = 0;
    _dir->diridx = -1;
}

DIR *opendir(const char *path)
{
    if (path == NULL)
        return NULL;

    char *p = (char *) path;

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_OPENDIR, 0);
    SETSRV_ARG(&srvarg, 1, p, strlen(p) + 1);
    SETSRV_RETVAL(&srvarg, -1);

    /* 文件描述符地址 */
    struct _dirdes *_dir = __alloc_dirdes();
    if (_dir == NULL) {
        return NULL;        
    }
    
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return NULL;
        }
        _dir->diridx = GETSRV_RETVAL(&srvarg, int);
        return _dir;
    }
    return NULL;
}

int closedir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return -1;

    if (dir->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_CLOSEDIR, 0);
    SETSRV_ARG(&srvarg, 1, dir->diridx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        __free_dirdes(dir);
        return 0;
    }
    return -1;
}

struct dirent *readdir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return NULL;

    if (dir->flags == 0)
        return NULL;

    static struct dirent de;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_READDIR, 0);
    SETSRV_ARG(&srvarg, 1, dir->diridx, 0);
    SETSRV_ARG(&srvarg, 2, &de, sizeof(struct dirent));
    SETSRV_IO(&srvarg, (SRVIO_USER << 2));
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return NULL;
        }
        return &de;
    }
    return NULL;
}

int rewinddir(DIR *dir)
{
    if (dir < __dirdes_table || dir >= __dirdes_table + _MAX_DIRDES_NR)
        return -1;
    if (dir->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_REWINDDIR, 0);
    SETSRV_ARG(&srvarg, 1, dir->diridx, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int mkdir(const char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
        
    char *p = (char *) path;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_MKDIR, 0);
    SETSRV_ARG(&srvarg, 1, p, strlen(p) + 1);
    SETSRV_ARG(&srvarg, 2, mode, 0);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int rmdir(const char *path)
{
    if (path == NULL)
        return -1;
        
    char *p = (char *) path;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_RMDIR, 0);
    SETSRV_ARG(&srvarg, 1, p, strlen(p) + 1);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int rename(const char *source, const char *target)
{
    if (source == NULL || target == NULL)
        return -1;
        
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_RENAME, 0);
    SETSRV_ARG(&srvarg, 1, source, strlen(source) + 1);
    SETSRV_ARG(&srvarg, 2, target, strlen(target) + 1);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}
