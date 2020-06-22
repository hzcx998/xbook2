#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <fsal/dir.h>
#include <ff.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <core/filesrv.h>
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
    res = f_unmount(p);
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
    if (ISBAD_FSAL_FIDX(idx))
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
    if (ISBAD_FSAL_FIDX(idx))
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
    if (ISBAD_FSAL_FIDX(idx))
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
    if (ISBAD_FSAL_FIDX(idx))
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

static int __opendir(char *path)
{
    fsal_dir_t *pdir = fsal_dir_alloc();
    if (pdir == NULL)
        return -1;
    /* 指向FATFS的抽象层 */
    pdir->fsal = &fatfs_fsal;

    FRESULT res;
    res = f_opendir(&pdir->dir.fatfs, path);                       /* Open the directory */
    if (res != FR_OK) {
        fsal_dir_free(pdir);
        return -1;
    }

    return FSAL_D2I(pdir);
}

static int __closedir(int idx)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)   /* file not used! */
        return -1;
    /* 保存文件标志 */
    int flags = pdir->flags;
    if (fsal_dir_free(pdir) < 0)
        return -1;

    FRESULT fres;
    fres = f_closedir(&pdir->dir.fatfs);
    if (fres != FR_OK) {
        pdir->flags = flags; /* 恢复文件信息 */
        return -1;
    }
    return 0;
}

static int __readdir(int idx, void *buf)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)   /* file not used! */
        return -1;
    
    FRESULT fres;
    FILINFO finfo;
    fres = f_readdir(&pdir->dir.fatfs, &finfo);
    if (fres != FR_OK) {
        return -1;
    }
    /* 名字为空 */
    if (finfo.fname[0] == '\0') 
        return -1;

    /* 往通用目录结构里面填充数据 */
    dirent_t *dire = (dirent_t *)buf;
    
    /* 解析属性 */
    dire->d_attr = 0;
    if (finfo.fattrib & AM_RDO)
        dire->d_attr |= DE_RDONLY;
    if (finfo.fattrib & AM_HID)
        dire->d_attr |= DE_HIDDEN;
    if (finfo.fattrib & AM_SYS)
        dire->d_attr |= DE_SYSTEM;
    if (finfo.fattrib & AM_DIR)
        dire->d_attr |= DE_DIR;
    if (finfo.fattrib & AM_ARC)
        dire->d_attr |= DE_ARCHIVE;

    dire->d_size = finfo.fsize;
    dire->d_time = finfo.ftime;
    dire->d_date = finfo.fdate;
    memcpy(dire->d_name, finfo.fname, DIR_NAME_LEN);
    dire->d_name[DIR_NAME_LEN - 1] = '\0';
    return 0;
}

static int __mkdir(char *path)
{
    FRESULT res;
    res = f_mkdir(path);
    if (res != FR_OK) {
        return -1;
    }
    return 0;
}

static int __unlink(char *path)
{
    FRESULT res;
    res = f_unlink(path);
    if (res != FR_OK) {
        return -1;
    }
    return 0;
}

static int __rename(char *old_path, char *new_path)
{
    FRESULT res;
    res = f_rename(old_path, new_path);
    if (res != FR_OK) {
        return -1;
    }
    return 0;
}

static int __truncate(int idx, off_t offset)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    
    off_t old = f_tell(&fp->file.fatfs);

    FRESULT fres;
    /* 先seek到偏移位置 */
    fres = f_lseek(&fp->file.fatfs, offset);
    if (fres != FR_OK) {
        return -1;
    }
    /* 再截断 */
    fres = f_truncate(&fp->file.fatfs);
    if (fres != FR_OK) {
        /* 恢复原来的位置 */
        f_lseek(&fp->file.fatfs, old);
        return -1;
    }
    return 0;
}

static int __sync(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    
    FRESULT fres;
    /* 先seek到偏移位置 */
    fres = f_sync(&fp->file.fatfs);
    if (fres != FR_OK) {
        return -1;
    }
    return 0;
}

static int __state(char *path, void *buf)
{
    FRESULT fres;
    FILINFO finfo;
    /* 先seek到偏移位置 */
    fres = f_stat(path, &finfo);
    if (fres != FR_OK) {
        return -1;
    }
    
    /* 往通用目录结构里面填充数据 */
    fstate_t *state = (fstate_t *)buf;
    
    /* 解析属性 */
    state->d_attr = 0;
    if (finfo.fattrib & AM_RDO)
        state->d_attr |= DE_RDONLY;
    if (finfo.fattrib & AM_HID)
        state->d_attr |= DE_HIDDEN;
    if (finfo.fattrib & AM_SYS)
        state->d_attr |= DE_SYSTEM;
    if (finfo.fattrib & AM_DIR)
        state->d_attr |= DE_DIR;
    if (finfo.fattrib & AM_ARC)
        state->d_attr |= DE_ARCHIVE;

    state->d_size = finfo.fsize;
    state->d_time = finfo.ftime;
    state->d_date = finfo.fdate;
    memcpy(state->d_name, finfo.fname, DIR_NAME_LEN);
    state->d_name[DIR_NAME_LEN - 1] = '\0';
    return 0;
}

static int __chmod(char *path, mode_t mode)
{
    return -1;
}

static int __utime(char *path, uint32_t time)
{
    FRESULT fres;
    FILINFO finfo;
    finfo.fdate = (time >> 16) & 0xffff;
    finfo.ftime = time & 0xffff;
    fres = f_utime(path, &finfo);
    if (fres = FR_OK)
        return -1;
    return 0;
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
    .extention  = (void *)&fatfs_extention,
};
