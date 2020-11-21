#include <fsal/fsal.h>
#include <fsal/dir.h>
#include <fsal/fstype.h>
#include <fsal/file.h>
#include <fsal/path.h>
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
        printk(KERN_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk(KERN_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    
    int handle = fsal->open(new_path, flags);
    if (handle >= 0)
        fsalif_incref(handle);
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
    return fsal->read(idx, buf, size);
}

static int fsalif_write(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
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
    return fsal->fsync(idx);
}

static int fsalif_opendir(char *path)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        printk(KERN_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk(KERN_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0) {
        printk(KERN_ERR "path %s switch error!\n", path);
        return -1;
    }
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
    return fsal->readdir(idx, buf);
}

static int fsalif_mkdir(char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        printk(KERN_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk(KERN_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    return fsal->mkdir(new_path, mode);
}

static int fsalif_unlink(char *path)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        printk("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    return fsal->unlink(new_path);
}

static int fsalif_rename(char *old_path, char *new_path)
{
    if (old_path == NULL || new_path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(old_path, 1);
    if (fpath == NULL) {
        printk("path %s not found!\n", old_path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk("path %s fsal error!\n", old_path);
        return -1;
    }
    char old_path2[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, old_path2, old_path) < 0)
        return -1;

    char new_path2[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path2, new_path) < 0)
        return -1;
    return fsal->rename(old_path2, new_path2);
}


static int fsalif_state(char *path, void *buf)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        printk("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    return fsal->state(new_path, buf);
}

static int fsalif_fstat(int idx, void *buf)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    fsal_t *fsal = pdir->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->fstat(idx, buf);
}

static int fsalif_chmod(char *path, mode_t mode)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        printk("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    return fsal->chmod(new_path, mode);
}

static int fsalif_fchmod(int idx, mode_t mode)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    fsal_t *fsal = pdir->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->fchmod(idx, mode);
}

static int fsalif_utime(char *path, time_t actime, time_t modtime)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        printk("path %s not found!\n", path);
        return -1;
    }

    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
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
    return fsal->rewind(idx);
}

static int fsalif_rewinddir(int idx)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *fp = FSAL_I2D(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->rewinddir(idx);
}

static int fsalif_rmdir(char *path)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        printk(KERN_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk(KERN_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    return fsal->rmdir(new_path);
}

static int fsalif_chdir(char *path)
{
    if (path == NULL)
        return -1;
    fsal_path_t *fpath = fsal_path_find(path, 1);
    if (fpath == NULL) {
        printk("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    return fsal->chdir(new_path);
}

static int fsalif_ioctl(int idx, int cmd, unsigned long arg)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->ioctl(idx, cmd, arg);
}

static int fsalif_fcntl(int idx, int cmd, long arg)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
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
    
    if (disk_info_find((char *) source) < 0) {
        printk(KERN_ERR "[%s] %s: source %s not found!\n", FS_MODEL_NAME, __func__, source);
        return -1;
    }
    if (fsal_path_find((void *) target, 0) != NULL) {
        printk(KERN_ERR "[%s] %s: target %s had mounted!\n", FS_MODEL_NAME, __func__, target);
        return -1;
    }
    fsal_t *fsal = fstype_find((char *)fstype);
    if (fsal == NULL) {
        printk("[%s] %s: filesystem type %s not found!\n", FS_MODEL_NAME, __func__, fstype);
        return -1;
    }
    int retval = fsal->mount(source, target, fstype, mountflags);
    if (retval < 0) {
        printk(KERN_ERR "[%s] %s: mount fs source %s target %s fstype %s failed!\n",
            FS_MODEL_NAME, __func__, source, target, fstype);
        return -1;
    }
    return 0;
}

static int fsalif_unmount(char *path, unsigned long flags)
{
    if (path == NULL)
        return -1;
    
    fsal_path_t *fpath = fsal_path_find(path, 0);
    if (fpath == NULL) {
        printk(KERN_ERR "path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk(KERN_ERR "path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, path) < 0)
        return -1;
    return fsal->unmount(new_path, flags);
}

int fsalif_mkfs(
    char *source,         /* 需要创建FS的设备 */
    char *fstype,         /* 文件系统类型 */
    unsigned long flags   /* 标志 */
) {
    if (source == NULL || fstype == NULL)
        return -1;
    if (disk_info_find(source) < 0) {
        printk(KERN_ERR "[%s] %s: source %s not found!\n", FS_MODEL_NAME, __func__, source);
        return -1;
    }
    fsal_t *fsal = fstype_find((char *)fstype);
    if (fsal == NULL) {
        printk(KERN_ERR "[%s] %s: filesystem type %s not found!\n", FS_MODEL_NAME, __func__, fstype);
        return -1;
    }
    int retval = fsal->mkfs(source, fstype, flags);
    if (retval < 0) {
        printk(KERN_ERR "[%s] %s: make fs source %s fstype %s failed!\n",
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
        printk("path %s not found!\n", path);
        return -1;
    }
    fsal_t *fsal = fpath->fsal;
    if (fsal == NULL) {
        printk("path %s fsal error!\n", path);
        return -1;
    }
    char new_path[MAX_PATH] = {0};
    if (fsal_path_switch(fpath, new_path, (char *) path) < 0)
        return -1;
    return fsal->access(new_path, mode);
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
};
