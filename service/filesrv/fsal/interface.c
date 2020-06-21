#include <fsal/fsal.h>
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
    if (ISBAD_FSALIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->close(idx);
}

static int __read(int idx, void *buf, size_t size)
{
    if (ISBAD_FSALIDX(idx))
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
    if (ISBAD_FSALIDX(idx))
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
    if (ISBAD_FSALIDX(idx))
        return -1;
    /* 查找对应的文件系统 */
    fsal_file_t *fp = FSAL_I2F(idx);
    fsal_t *fsal = fp->fsal;
    if (fsal == NULL)
        return -1;
    return fsal->lseek(idx, off, whence);
}

fsal_t fsif = {
    .open       = __open,
    .close      = __close,
    .read       = __read,
    .write      = __write,
    .lseek      = __lseek,
};