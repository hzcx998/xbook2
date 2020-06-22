#include <fsal/fsal.h>
#include <fsal/dir.h>
#include <stdio.h>

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

static int __truncate(int idx, off_t offset)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->truncate(idx, offset);
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

static int __sync(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->sync(idx);
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

static int __mkdir(char *path)
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
    return fsal->mkdir(new_path);
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

static int __utime(char *path, uint32_t time)
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
    return fsal->utime(new_path, time);
}

fsal_t fsif = {
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
    .truncate   = __truncate,
    .sync       = __sync,
    .state      = __state,
    .chmod      = __chmod,
    .utime      = __utime,
};
