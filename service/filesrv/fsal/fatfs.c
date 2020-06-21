#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <ff.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <filesrv.h>
#include <unistd.h>


typedef struct {
    FATFS *fsobj;        /* 挂载对象指针 */
} fatfs_extention_t;

fatfs_extention_t fatfs_extention;

/**
 * 挂载文件系统
*/
static int __mount(void *path, char drive, int flags)
{
    FATFS *fsobj;           /* Filesystem object */
    fsobj = malloc(sizeof(FATFS));
    if (fsobj == NULL) 
        return -1;
    memset(fsobj, 0, sizeof(FATFS));
    FRESULT res;
    const TCHAR *p = (const TCHAR *) path;
    res = f_mount(fsobj, p, 1);
    if (res != FR_OK) {
        printf("%s: %s: mount on path %s failed, code %d!\n", SRV_NAME, __func__, p, res);
        free(fsobj);
        return -1;
    }
    fatfs_extention.fsobj = fsobj;

    if (fsal_path_insert((void *)p, drive, &fatfs_fsal)) {
        printf("%s: %s: insert path %s failed!\n", SRV_NAME, __func__, p);
        free(fsobj);
        return -1;
    }
    return 0;
}


/**
 * 挂载文件系统
*/
static int __unmount(void *path, int flags)
{
    FRESULT res;
    const TCHAR *p = (const TCHAR *) path;
    fatfs_extention_t *ext = &fatfs_extention;
    res = f_mount(NULL, p, 0);
    if (res != FR_OK) {
        printf("%s: %s: unmount on path %s failed, code %d.\n", SRV_NAME, __func__, p, res);
        return -1;
    }

    if (fsal_path_remove((void *) p)) {
        printf("%s: %s: remove path %s failed!\n", SRV_NAME, __func__, p);
        return -1;
    }

    free(ext->fsobj);
    return 0;
}

/**
 * 挂载文件系统
*/
static int __mkfs(void *path, int flags)
{
    FRESULT res;        /* API result code */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */

    const TCHAR *p = (const TCHAR *) path;
    /* 在磁盘上创建文件系统 */
    res = f_mkfs(p, 0, work, sizeof(work));
    if (res != FR_OK) {
        printf("%s: make fs on drive %s failed with resoult code %d.\n", SRV_NAME, p, res);
        return -1;
    }
    return 0;
}

static int __open(void *path, int flags)
{
    fsal_file_t *fp = fsal_file_alloc();
    if (fp == NULL)
        return -1;
    /* 指向FATFS的抽象层 */
    fp->fsal = &fatfs_fsal;

    const TCHAR *p = (const TCHAR *) path;

    /* 抽象层标志转换成FATFS的标志 */
    BYTE mode = 0;  /* 文件打开模式 */
    if (flags & O_RDONLY) {
        mode |= FA_READ;
    } else if (flags & O_WRONLY) {
        mode |= FA_WRITE;
    } else if (flags & O_RDWR) {
        mode |= FA_READ | FA_WRITE;
    }
    if (flags & O_TRUNC) {
        mode |= FA_CREATE_ALWAYS;
    } else if (flags & O_APPEDN) {
        mode |= FA_OPEN_APPEND;
    } else if (flags & O_CREAT) {
        mode |= FA_OPEN_ALWAYS;
    }

    FRESULT fres;
    fres = f_open(&fp->file.fatfs, p, mode);
    if (fres != FR_OK) {
        fsal_file_free(fp);
        return -1;
    }
    return FSAL_F2I(fp);
}

static int __close(int idx)
{
    if (ISBAD_FSALIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    /* 保存文件标志 */
    int flags = fp->flags;
    if (fsal_file_free(fp) < 0)
        return -1;

    FRESULT fres;
    fres = f_close(&fp->file.fatfs);
    if (fres != FR_OK) {
        fp->flags = flags; /* 恢复文件信息 */
        return -1;
    }
    return 0;
}

static int __read(int idx, void *buf, size_t size)
{
    if (ISBAD_FSALIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
        
    FRESULT fr;
    UINT br;
    fr = f_read(&fp->file.fatfs, buf, size, &br);
    if (fr != FR_OK)
        return -1;
    return br;
}

static int __write(int idx, void *buf, size_t size)
{
    if (ISBAD_FSALIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
        
    FRESULT fr;
    UINT bw;
    fr = f_write(&fp->file.fatfs, buf, size, &bw);
    if (fr != FR_OK)
        return -1;
    return bw;
}

static int __lseek(int idx, off_t offset, int whence)
{
    if (ISBAD_FSALIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
        
    off_t new_off = 0;
    switch (whence)
    {
    case SEEK_SET:
        new_off = offset;
        break;
    case SEEK_CUR:
        new_off = f_tell(&fp->file.fatfs) + offset;
        break;
    case SEEK_END:
        new_off = f_size(&fp->file.fatfs) + offset;
        break;
    default:
        break;
    }
    FRESULT fr;
    fr = f_lseek(&fp->file.fatfs, new_off);
    if (fr != FR_OK)
        return -1;
    
    return new_off;
}

/* fatfs的抽象层 */
fsal_t fatfs_fsal = {
    .mkfs       = __mkfs,
    .mount      = __mount,
    .unmount    = __unmount,
    .open       = __open,
    .close      = __close,
    .read       = __read,
    .write      = __write,
    .lseek      = __lseek,
    .extention  = (void *)&fatfs_extention,
};
