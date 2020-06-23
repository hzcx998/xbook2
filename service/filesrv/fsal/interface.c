#include <fsal/fsal.h>
#include <fsal/dir.h>
#include <core/filesrv.h>
#include <core/fstype.h>
#include <stdio.h>
#include <drivers/disk.h>

static int __open(void *path, int flags)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }
        
    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->open(new_path, flags);
}

static int __close(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->close(idx);
}

static int __ftruncate(int idx, off_t offset)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->ftruncate(idx, offset);
}

static int __read(int idx, void *buf, size_t size)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->read(idx, buf, size);
}

static int __write(int idx, void *buf, size_t size)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->write(idx, buf, size);
}

static int __lseek(int idx, off_t off, int whence)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->lseek(idx, off, whence);
}

static int __fsync(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->fsync(idx);
}

static int __opendir(char *path)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }
        
    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->opendir(new_path);
}

static int __closedir(int idx)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_dir_t *pdir = FSAL_I2D(idx);
    fsal_t *fsal = pdir->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->closedir(idx);
}

static int __readdir(int idx, void *buf)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_dir_t *pdir = FSAL_I2D(idx);
    fsal_t *fsal = pdir->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->readdir(idx, buf);
}

static int __mkdir(char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }
        
    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->mkdir(new_path, mode);
}

static int __unlink(char *path)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }

    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->unlink(new_path);
}

static int __rename(char *old_path, char *new_path)
{
    if (old_path == NULL || new_path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(old_path);
    if (fpath == NULL) {
        printf("path %s not found!\n", old_path);
        return -1;
    }

    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", old_path);
        return -1;
    }

    /* 转换路径 */
    char old_path2[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, old_path2, old_path) < 0)
        return -1;

    char new_path2[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path2, new_path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->rename(old_path2, new_path2);
}


static int __state(char *path, void *buf)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }

    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->state(new_path, buf);
}

static int __chmod(char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }

    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->chmod(new_path, mode);
}

static int __fchmod(int idx, mode_t mode)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_dir_t *pdir = FSAL_I2D(idx);
    fsal_t *fsal = pdir->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->fchmod(idx, mode);
}

static int __utime(char *path, time_t actime, time_t modtime)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }

    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->utime(new_path, actime, modtime);
}

static int __feof(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->feof(idx);
}

static int __ferror(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->ferror(idx);
}

static off_t __ftell(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->ftell(idx);
}

static size_t __fsize(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->fsize(idx);
}

static int __rewind(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->rewind(idx);
}

static int __rewinddir(int idx)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_dir_t *fp = FSAL_I2D(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->rewinddir(idx);
}

static int __rmdir(char *path)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }

    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->rmdir(new_path);
}


/* 挂载文件系统 */
int __mount(
    char *source,         /* 需要挂载的资源 */
    char *target,         /* 挂载到的目标位置 */
    char *fstype,         /* 文件系统类型 */
    unsigned long mountflags    /* 挂载标志 */
) {
    if (source == NULL || target == NULL || fstype == NULL)
        return -1;
    
    /* 查找要挂载的资源 */
    if (disk_res_find((char *) source) < 0) {
        printf("[%s] %s: source %s not found!\n", SRV_NAME, __func__, source);
        return -1;
    }

    /* 查看目标位置是否可用 */
    if (fsal_path_find((void *) target) != NULL) {
        printf("[%s] %s: target %s had mounted!\n", SRV_NAME, __func__, target);
        return -1;
    }

    /* 查找文件系统类型 */
    fsal_t *fsal = fstype_find((char *)fstype);
    if (fsal == NULL) {
        printf("[%s] %s: filesystem type %s not found!\n", SRV_NAME, __func__, fstype);
        return -1;
    }

    printf("[%s] %s: will mount fs source %s target %s fstype %s.\n",
            SRV_NAME, __func__, source, target, fstype);

    /* 执行对应类型文件系统的挂载 */
    int retval = fsal->mount(source, target, fstype, mountflags);
    if (retval < 0) {
        printf("[%s] %s: mount fs source %s target %s fstype %s failed!\n",
            SRV_NAME, __func__, source, target, fstype);
        return -1;
    }
    return 0;
}

static int __unmount(char *path, unsigned long flags)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path);
    if (fpath == NULL) {
        printf("path %s not found!\n", path);
        return -1;
    }

    /* 查找对应的文件系统 */
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printf("path %s fsal error!\n", path);
        return -1;
    }

    /* 转换路径 */
    char new_path[FASL_PATH_LEN] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;

    /* 执行打开 */
    return fsal->unmount(new_path, flags);
}

/* 创建文件系统 */
int __mkfs(
    char *source,         /* 需要创建FS的设备 */
    char *fstype,         /* 文件系统类型 */
    unsigned long flags   /* 标志 */
) {
    if (source == NULL || fstype == NULL)
        return -1;
    
    /* 查找要挂载的资源 */
    if (disk_res_find(source) < 0) {
        printf("[%s] %s: source %s not found!\n", SRV_NAME, __func__, source);
        return -1;
    }

    /* 查找文件系统类型 */
    fsal_t *fsal = fstype_find((char *)fstype);
    if (fsal == NULL) {
        printf("[%s] %s: filesystem type %s not found!\n", SRV_NAME, __func__, fstype);
        return -1;
    }

    printf("[%s] %s: will make fs on source %s fstype %s.\n",
            SRV_NAME, __func__, source, fstype);

    /* 执行对应类型文件系统的挂载 */
    int retval = fsal->mkfs(source, fstype, flags);
    if (retval < 0) {
        printf("[%s] %s: make fs source %s fstype %s failed!\n",
            SRV_NAME, __func__, source, fstype);
        return -1;
    }
    return 0;
}

fsal_t fsif = {
    .mkfs       = __mkfs,
    .mount      = __mount,
    .unmount    = __unmount,
    .open       = __open,
    .close      = __close,
    .read       = __read,
    .write      = __write,
    .lseek      = __lseek,
    .opendir    = __opendir,
    .closedir   = __closedir,
    .readdir    = __readdir,
    .mkdir      = __mkdir,
    .unlink     = __unlink,
    .rename     = __rename,
    .ftruncate  = __ftruncate,
    .fsync      = __fsync,
    .state      = __state,
    .chmod      = __chmod,
    .fchmod     = __fchmod,
    .utime      = __utime,
    .feof       = __feof,
    .ferror     = __ferror,
    .ftell      = __ftell,
    .fsize      = __fsize,
    .rewind     = __rewind,
    .rewinddir  = __rewinddir,
    .rmdir      = __rmdir,
};
