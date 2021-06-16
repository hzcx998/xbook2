#include <xbook/fsal.h>
#include <xbook/dir.h>
#include <xbook/fstype.h>
#include <xbook/file.h>
#include <xbook/path.h>
#include <stddef.h>
#include <errno.h>
#include <xbook/diskman.h>
#include <xbook/debug.h>

// #define DEBUG_FSALIF

static int fsalif_incref(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))
        return -1;
    fsal_file_inc_reference(fp);
    return 0;
}

static int fsalif_decref(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))
        return -1;
    fsal_file_dec_reference(fp);
    return 0;
}

static int fsalif_open(void *path, int flags)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint(PRINT_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint(PRINT_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0) {
        keprint(PRINT_ERR "path %s switch error!\n", path);
        return -1;
    }
    if (!fsal->open)
        return -ENOSYS;
    int handle = fsal->open(new_path, flags);
    if (handle >= 0)
        fsalif_incref(handle);
    /*else
        keprint(PRINT_ERR "path %s real open error!\n", path);
    */
    return handle;
}

static int fsalif_close(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))
        return -1;
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (fsalif_decref(idx) < 0)
        return -EINVAL;
    if (!fsal_file_need_close(fp)) {
        return 0;   /* no need to close */
    }
    if (!fsal->close)
        return -ENOSYS;
    return fsal->close(idx);
}

static int fsalif_ftruncate(int idx, off_t offset)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->ftruncate)
        return -ENOSYS;
    return fsal->ftruncate(idx, offset);
}

static int fsalif_read(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->read)
        return -ENOSYS;
    return fsal->read(idx, buf, size);
}

static int fsalif_write(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -EINVAL;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL) {
        errprint("fsalif write: idx %d fsal null\n", fsal);
        return -1;
    }
    if (!fsal->write)
        return -ENOSYS;
    return fsal->write(idx, buf, size);
}

static int fsalif_fastread(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->fastread)
        return -ENOSYS;
    return fsal->fastread(idx, buf, size);
}

static int fsalif_fastwrite(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -EINVAL;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL) {
        errprint("fsalif write: idx %d fsal null\n", fsal);
        return -1;
    }
    if (!fsal->fastwrite)
        return -ENOSYS;
    return fsal->write(idx, buf, size);
}

static int fsalif_lseek(int idx, off_t off, int whence)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->lseek)
        return -ENOSYS;
    return fsal->lseek(idx, off, whence);
}

static int fsalif_fsync(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->fsync)
        return -ENOSYS;
    return fsal->fsync(idx);
}

static int fsalif_opendir(char *path)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint(PRINT_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint(PRINT_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0) {
        keprint(PRINT_ERR "path %s switch error!\n", path);
        return -1;
    }
    if (!fsal->opendir)
        return -ENOSYS;
    return fsal->opendir(new_path);
}

static int fsalif_closedir(int idx)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    fsal_t *fsal = pdir->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->closedir)
        return -ENOSYS;
    return fsal->closedir(idx);
}

static int fsalif_readdir(int idx, void *buf)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    fsal_t *fsal = pdir->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->readdir)
        return -ENOSYS;
    return fsal->readdir(idx, buf);
}

static int fsalif_mkdir(char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint(PRINT_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint(PRINT_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    if (!fsal->mkdir)
        return -ENOSYS;
    return fsal->mkdir(new_path, mode);
}

static int fsalif_unlink(char *path)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    if (!fsal->unlink)
        return -ENOSYS;
    return fsal->unlink(new_path);
}

static int fsalif_rename(char *old_path, char *new_path)
{
    if (old_path == NULL || new_path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(old_path, 1);
    if (fpath == NULL) {
        keprint("path %s not found!\n", old_path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint("path %s fsal error!\n", old_path);
        return -1;
    }
    char old_path2[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, old_path2, old_path) < 0)
        return -1;

    char new_path2[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path2, new_path) < 0)
        return -1;
    if (!fsal->rename)
        return -ENOSYS;
    return fsal->rename(old_path2, new_path2);
}


static int fsalif_state(char *path, void *buf)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0) {
        errprint("path %s switch error!\n", path);
        return -1;
    }
    if (!fsal->state)
        return -ENOSYS;
    return fsal->state(new_path, buf);
}

static int fsalif_fstat(int idx, void *buf)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -EINVAL;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL) {
        return -EINVAL;
    }
    if (!fsal->fstat)
        return -ENOSYS;
    return fsal->fstat(idx, buf);
}

static int fsalif_chmod(char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    if (!fsal->chmod)
        return -ENOSYS;
    return fsal->chmod(new_path, mode);
}

static int fsalif_fchmod(int idx, mode_t mode)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -EINVAL;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL) {
        return -EINVAL;
    }
    if (!fsal->fchmod)
        return -ENOSYS;
    return fsal->fchmod(idx, mode);
}

static int fsalif_utime(char *path, time_t actime, time_t modtime)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint("path %s not found!\n", path);
        return -1;
    }

    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    if (!fsal->utime)
        return -ENOSYS;
    return fsal->utime(new_path, actime, modtime);
}

static int fsalif_feof(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->feof)
        return -ENOSYS;
    return fsal->feof(idx);
}

static int fsalif_ferror(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->ferror)
        return -ENOSYS;
    return fsal->ferror(idx);
}

static off_t fsalif_ftell(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->ftell)
        return -ENOSYS;
    return fsal->ftell(idx);
}

static size_t fsalif_fsize(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->fsize)
        return -ENOSYS;
    return fsal->fsize(idx);
}

static int fsalif_rewind(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->rewind)
        return -ENOSYS;
    return fsal->rewind(idx);
}

static int fsalif_rewinddir(int idx)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *fdir = FSAL_I2D(idx);
    fsal_t *fsal = fdir->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->rewinddir)
        return -ENOSYS;
    return fsal->rewinddir(idx);
}

static int fsalif_rmdir(char *path)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint(PRINT_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint(PRINT_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    if (!fsal->rmdir)
        return -ENOSYS;
    return fsal->rmdir(new_path);
}

static int fsalif_chdir(char *path)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        keprint("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    if (!fsal->chdir)
        return -ENOSYS;
    return fsal->chdir(new_path);
}

static int fsalif_ioctl(int idx, int cmd, void *arg)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->ioctl)
        return -ENOSYS;
    return fsal->ioctl(idx, cmd, arg);
}

static int fsalif_fastio(int idx, int cmd, void *arg)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->fastio)
        return -ENOSYS;
    return fsal->fastio(idx, cmd, arg);
}

static int fsalif_fcntl(int idx, int cmd, long arg)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    if (!fsal->fcntl)
        return -ENOSYS;
    return fsal->fcntl(idx, cmd, arg);
}

int fsalif_mount(
    char *source,         /* 需要挂载的资源 */
    char *target,         /* 挂载到的目标位置 */
    char *fstype,         /* 文件系统类型 */
    unsigned long mountflags    /* 挂载标志 */
) {
    if (source == NULL || target == NULL || fstype == NULL)
        return -1;
    
    if (disk_info_find_with_path((char *) source) < 0) {
        keprint(PRINT_ERR "[%s] %s: source %s not found!\n", FS_MODEL_NAME, __func__, source);
        return -1;
    }
    
    if (fsal_path_find((void *) target, 0) != NULL) {
        keprint(PRINT_ERR "[%s] %s: target %s had mounted!\n", FS_MODEL_NAME, __func__, target);
        return -1;
    }

    if (!strcmp("auto", fstype)) {  /* 如果是auto，选择默认文件系统 */
        fstype = FSAL_FSTYPE_DEFAULT;
    }

    fsal_t *fsal = fstype_find((char *)fstype);
    if (fsal == NULL) {
        keprint("[%s] %s: filesystem type %s not found!\n", FS_MODEL_NAME, __func__, fstype);
        return -1;
    }
    int retval = fsal->mount(source, target, fstype, mountflags);
    if (retval < 0) {
        keprint(PRINT_ERR "[%s] %s: mount fs source %s target %s fstype %s failed!\n",
            FS_MODEL_NAME, __func__, source, target, fstype);
        return -1;
    }
    return 0;
}

/**
 * origin_path: 原始路径，从用户传递进来的路径
 * path: 转换后的路径，会物理路径
 */
static int fsalif_unmount(char *origin_path, char *path, unsigned long flags)
{
    if (origin_path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(origin_path, 0);
    if (fpath == NULL) {
        keprint(PRINT_ERR "path %s not found!\n", origin_path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint(PRINT_ERR "path %s fsal error!\n", origin_path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, origin_path) < 0)
        return -1;
    return fsal->unmount(origin_path, new_path, flags);
}

int fsalif_mkfs(
    char *source,         /* 需要创建FS的设备 */
    char *fstype,         /* 文件系统类型 */
    unsigned long flags   /* 标志 */
) {
    if (source == NULL || fstype == NULL)
        return -1;
    if (disk_info_find_with_path(source) < 0) {
        keprint(PRINT_ERR "[%s] %s: source %s not found!\n", FS_MODEL_NAME, __func__, source);
        return -1;
    }
    if (!strcmp("auto", fstype)) {  /* 如果是auto，选择默认文件系统 */
        fstype = FSAL_FSTYPE_DEFAULT;
    }
    fsal_t *fsal = fstype_find((char *)fstype);
    if (fsal == NULL) {
        keprint(PRINT_ERR "[%s] %s: filesystem type %s not found!\n", FS_MODEL_NAME, __func__, fstype);
        return -1;
    }
    int retval = fsal->mkfs(source, fstype, flags);
    if (retval < 0) {
        keprint(PRINT_ERR "[%s] %s: make fs source %s fstype %s failed!\n",
            FS_MODEL_NAME, __func__, source, fstype);
        return -1;
    }
    return 0;
}

static int fsalif_access(const char *path, int mode)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find((void *) path, 1);
    if (fpath == NULL) {
        keprint("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        keprint("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, (char *) path) < 0)
        return -1;
    if (!fsal->access)
        return -ENOSYS;
    return fsal->access(new_path, mode);
}

static void *fsalif_mmap(int idx, void *addr, size_t length, int prot, int flags, off_t offset)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return (void *) -EINVAL;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return (void *) -1;
    if (!fsal->mmap)
        return (void *) -ENOSYS;
    return fsal->mmap(idx, addr, length, prot, flags, offset);
}

static int fsalif_select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout)
{
    /* TODO: add fsalif select */
    dbgprint("normal file select not support\n");
    return -ENOSYS;
}

/* 文件的抽象层接口 */
fsal_t fsif = {
    .name       = "fsif",
    .subtable   = NULL,
    .mkfs       = fsalif_mkfs,
    .mount      = fsalif_mount,
    .unmount    = fsalif_unmount,
    .open       = fsalif_open,
    .close      = fsalif_close,
    .read       = fsalif_read,
    .write      = fsalif_write,
    .lseek      = fsalif_lseek,
    .opendir    = fsalif_opendir,
    .closedir   = fsalif_closedir,
    .readdir    = fsalif_readdir,
    .mkdir      = fsalif_mkdir,
    .unlink     = fsalif_unlink,
    .rename     = fsalif_rename,
    .ftruncate  = fsalif_ftruncate,
    .fsync      = fsalif_fsync,
    .state      = fsalif_state,
    .chmod      = fsalif_chmod,
    .fchmod     = fsalif_fchmod,
    .utime      = fsalif_utime,
    .feof       = fsalif_feof,
    .ferror     = fsalif_ferror,
    .ftell      = fsalif_ftell,
    .fsize      = fsalif_fsize,
    .rewind     = fsalif_rewind,
    .rewinddir  = fsalif_rewinddir,
    .rmdir      = fsalif_rmdir,
    .chdir      = fsalif_chdir,
    .ioctl      = fsalif_ioctl,
    .fcntl      = fsalif_fcntl,
    .fstat      = fsalif_fstat,
    .access     = fsalif_access,
    .incref     = fsalif_incref,
    .decref     = fsalif_decref,
    .fastread   = fsalif_fastread,
    .fastwrite  = fsalif_fastwrite,
    .fastio     = fsalif_fastio,
    .mmap       = fsalif_mmap,
    .select     = fsalif_select,
};
