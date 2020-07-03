#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <fsal/dir.h>
#include <ff.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <core/filesrv.h>
#include <unistd.h>
#include <drivers/disk.h>
#include <sys/stat.h>

#define DEBUG_LOCAL 0


typedef struct {
    FATFS *fsobj;        /* 挂载对象指针 */
} fatfs_extention_t;

fatfs_extention_t fatfs_extention;

int fatfs_drv_map[FF_VOLUMES] = {
    1,0,2,3,4,5,6,7,8,9
};

/**
 * 挂载文件系统
*/
static int __mount(char *source, char *target, char *fstype, unsigned long flags)
{
    /* 根据资源设备查找对应的磁盘路径 */
    int solt = disk_res_find(source);
    if (solt < 0)
        return -1;
#if DEBUG_LOCAL == 1
    printf("[%s] %s: find device solt %d.\n", SRV_NAME, __func__, solt);
#endif
    /* 转换成fatfs的物理驱动器 */
    int pdrv, i;
    for (i = 0; i < FF_VOLUMES; i++) {
        if (fatfs_drv_map[i] == solt) {
            break;
        }
    }
    /* not found */
    if (i >= FF_VOLUMES) {
        return -1;
    }
    /* 获取一个FATFS的物理磁盘驱动器 */
    pdrv = i;
    /* 构建挂载路径 */
    char path[3] = {pdrv + '0', ':', 0};

#if DEBUG_LOCAL == 1
    printf("[%s] %s: find fatfs drive[%d] %s \n", SRV_NAME, __func__, pdrv, path);
#endif

    /* 判断是否已经有文件系统在此系统上了 */
    int res = res_open(source, RES_DEV, 0);
    if (res < 0) {
        return -1;
    }
    unsigned char buf[512];
    if (res_read(res, 0, buf, SECTOR_SIZE) < 0) {
        res_close(res);
        return -1;
    }
    res_close(res);

    char remkfs = 1;    /* 重新构建文件系统 */
    /* 查看是否已经有文件系统 */
    if (buf[510] == 0x55 && buf[511] == 0xaa) {
        remkfs = 0; /* 不需要创建文件系统 */
#if DEBUG_LOCAL == 1
        printf("[%s] %s: had filesystem on disk %s \n", SRV_NAME, __func__, source);
#endif
        /* 强制要求格式化磁盘 */
        if (flags & MT_REMKFS) {
            remkfs = 1;
        }
    }
    
    const TCHAR *p;
    if (remkfs) {   /* 创建文件系统 */
        BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
        p = (const TCHAR *) path;
        /* 根据传入的文件系统类型选择参数 */
        MKFS_PARM parm = {FM_ANY, 0, 0, 0, 0};
        if (!strcmp(fstype, "fat12") || !strcmp(fstype, "fat16")) {
            parm.fmt = FM_FAT;
        } else if (!strcmp(fstype, "fat32")) {
            parm.fmt = FM_FAT32;
        } else if (!strcmp(fstype, "exfat")) {
            parm.fmt = FM_EXFAT;
        }
#if DEBUG_LOCAL == 1
        printf("[%s] %s: make new fs %s on disk %s \n", SRV_NAME, __func__, fstype, source);
#endif            
        /* 在磁盘上创建文件系统 */
        res = f_mkfs(p, (const MKFS_PARM *)&parm, work, sizeof(work));
        if (res != FR_OK) {
            printf("%s: make fs on drive %s failed with resoult code %d.\n", SRV_NAME, p, res);
            return -1;
        }
    }

    FATFS *fsobj;           /* Filesystem object */
    fsobj = malloc(sizeof(FATFS));
    if (fsobj == NULL) 
        return -1;
    memset(fsobj, 0, sizeof(FATFS));
    FRESULT fr;
    p = (const TCHAR *) path;

    BYTE delayed = !(flags & MT_DELAYED);  /* 延时挂载 */
    fr = f_mount(fsobj, p, delayed);
    if (fr != FR_OK) {
        printf("%s: %s: mount on path %s failed, code %d!\n", SRV_NAME, __func__, p, fr);
        free(fsobj);
        return -1;
    }
    fatfs_extention.fsobj = fsobj;
#if DEBUG_LOCAL == 1
    printf("[%s] %s: mount disk %s to %s success.\n", SRV_NAME, __func__, source, target);
#endif
    if (fsal_path_insert((void *)p, *target, &fatfs_fsal)) {
        printf("%s: %s: insert path %s failed!\n", SRV_NAME, __func__, p);
        free(fsobj);
        return -1;
    }
#if DEBUG_LOCAL == 1
    printf("[%s] %s: insert path  %s to %s success.\n", SRV_NAME, __func__, p, target);
#endif
    return 0;
}

/**
 * 挂载文件系统
*/
static int __unmount(char *path, unsigned long flags)
{
    /* 在末尾填0，只保留磁盘符和分隔符 */
    path[2] = '\0';
    
    FRESULT res;
    const TCHAR *p = (const TCHAR *) path;
    fatfs_extention_t *ext = &fatfs_extention;
#if DEBUG_LOCAL == 1
    printf("[%s] %s: unmount path %s \n", SRV_NAME, __func__, p);
#endif
    res = f_unmount(p);
    if (res != FR_OK) {
        printf("%s: %s: unmount on path %s failed, code %d.\n", SRV_NAME, __func__, p, res);
        return -1;
    }
#if DEBUG_LOCAL == 1
    printf("[%s] %s: fatfs unmount path %s success.\n", SRV_NAME, __func__, p);
#endif
    if (fsal_path_remove((void *) p)) {
        printf("%s: %s: remove path %s failed!\n", SRV_NAME, __func__, p);
        return -1;
    }

    free(ext->fsobj);
    ext->fsobj = NULL;

    return 0;
}

/**
 * 挂载文件系统
*/
static int __mkfs(char *source, char *fstype, unsigned long flags)
{
    /* 根据资源设备查找对应的磁盘路径 */
    int solt = disk_res_find(source);
    if (solt < 0)
        return -1;
#if DEBUG_LOCAL == 1
    printf("[%s] %s: find device solt %d.\n", SRV_NAME, __func__, solt);
#endif
    /* 转换成fatfs的物理驱动器 */
    int pdrv, i;
    for (i = 0; i < FF_VOLUMES; i++) {
        if (fatfs_drv_map[i] == solt) {
            break;
        }
    }
    /* not found */
    if (i >= FF_VOLUMES) {
        return -1;
    }
    /* 获取一个FATFS的物理磁盘驱动器 */
    pdrv = i;
    /* 构建挂载路径 */
    char path[3] = {pdrv + '0', ':', 0};

#if DEBUG_LOCAL == 1
    printf("[%s] %s: find fatfs drive[%d] %s \n", SRV_NAME, __func__, pdrv, path);
#endif
    FRESULT res;        /* API result code */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
    const TCHAR *p = (const TCHAR *) path;
    /* 根据传入的文件系统类型选择参数 */
    MKFS_PARM parm = {FM_ANY, 0, 0, 0, 0};
    if (!strcmp(fstype, "fat12") || !strcmp(fstype, "fat16")) {
        parm.fmt = FM_FAT;
    } else if (!strcmp(fstype, "fat32")) {
        parm.fmt = FM_FAT32;
    } else if (!strcmp(fstype, "exfat")) {
        parm.fmt = FM_EXFAT;
    }
#if DEBUG_LOCAL == 1
    printf("[%s] %s: make new fs %s on disk %s \n", SRV_NAME, __func__, fstype, source);
#endif
    /* 在磁盘上创建文件系统 */
    res = f_mkfs(p, (const MKFS_PARM *)&parm, work, sizeof(work));
    if (res != FR_OK) {
        printf("[%s]: %s: make fs on drive %s failed with resoult code %d.\n", SRV_NAME, __func__, p, res);
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

static int __mkdir(char *path, mode_t mode)
{
    //printf("fatfs: make dir %s\n", path);
    FRESULT res;
    res = f_mkdir(path);
    if (res != FR_OK) {
        //printf("fatfs: %s: fresult code:%d\n", __func__, res);
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

static int __ftruncate(int idx, off_t offset)
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

static int __fsync(int idx)
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
    stat_t *stat = (stat_t *)buf;
    
    mode_t mode = S_IREAD | S_IWRITE;
    /* 解析属性 */
    if (finfo.fattrib & AM_RDO)
        mode &= ~S_IWRITE;

    stat->st_mode = mode;
    stat->st_size = finfo.fsize;
    stat->st_atime = (finfo.fdate << 16) | finfo.ftime;
    stat->st_ctime = stat->st_mtime = stat->st_atime;
    
    return 0;
}

static int __chmod(char *path, mode_t mode)
{
    return 0;
}

static int __fchmod(int idx, mode_t mode)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    return 0;
}

static int __utime(char *path, time_t actime, time_t modtime)
{
    FRESULT fres;
    FILINFO finfo;
    finfo.fdate = (modtime >> 16) & 0xffff;
    finfo.ftime = modtime & 0xffff;
    fres = f_utime(path, &finfo);
    if (fres != FR_OK)
        return -1;
    return 0;
}

/**
 * __feof - 检测文件到达末尾
 * 
 * 执行失败返回-1，已经到达末尾返回1，没有到达末尾返回0
 */
static int __feof(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    return f_eof(&fp->file.fatfs);
}

/**
 * __ferror - 文件已经出错
 * 
 * 执行失败返回-1，出错则返回1，没有出错返回0
 */
static int __ferror(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    return f_error(&fp->file.fatfs);
}

/**
 * __ftell - 返回当前读写位置
 * 
 * 执行失败返回-1，成功返回位置
 */
static off_t __ftell(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    return f_tell(&fp->file.fatfs);
}

/**
 * __fsize - 返回文件的大小
 * 
 * 执行失败返回0，成功返回文件大小
 */
static size_t __fsize(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    return f_size(&fp->file.fatfs);
}


/**
 * __rewind - 重置文件的读写位置
 * 
 * 执行失败返回-1，成功返回0
 */
static int __rewind(int idx)
{
    if (ISBAD_FSAL_FIDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_I2F(idx);
    if (!fp->flags)   /* file not used! */
        return -1;
    return f_rewind(&fp->file.fatfs);
}

/**
 * __rewinddir - 重置目录的读写位置
 * 
 * 执行失败返回-1，成功返回0
 */
static int __rewinddir(int idx)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)   /* file not used! */
        return -1;
    return f_rewinddir(&pdir->dir.fatfs);
}

/**
 * __rmdir - 删除目录
 * 
 * 执行失败返回-1，成功返回0
 */
static int __rmdir(char *path)
{
    FRESULT res;
    res = f_rmdir(path);
    if (res != FR_OK) {
        return -1;
    }
    return 0;
}

/**
 * __chdir - 改变工作目录
 * 
 * 检测目录是否存在，以让客户端程序改变本地的工作目录
 * 
 * 执行失败返回-1，成功返回0
 */
static int __chdir(char *path)
{
    FRESULT res;
    DIR dir;
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res != FR_OK) {
        return -1;
    }
    f_closedir(&dir);
    return 0;
}

/**
 * __ioctl - 对文件进行输入输出
 * 
 * FATFS不支持，默认返回0
 * 
 * 执行失败返回-1，成功返回0
 */
static int __ioctl(int fd, int cmd, unsigned long arg)
{
    return 0;
}

/**
 * __fcntl - 对文件进行设定
 * 
 * FATFS不支持，默认返回0
 * 
 * 执行失败返回-1，成功返回0
 */
static int __fcntl(int fd, int cmd, long arg)
{
    return 0;
}

/* fatfs 支持的文件系统类型 */
static char *fatfs_sub_table[] = {
    "fat12",
    "fat16",
    "fat32",
    "exfat",
    NULL           /* 最后一个成员必须是NULL */
};

/* fatfs的抽象层 */
fsal_t fatfs_fsal = {
    .list       = LIST_HEAD_INIT(fatfs_fsal.list),
    .name       = "fatfs",
    .subtable   = fatfs_sub_table,
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
    .chdir      = __chdir,
    .ioctl      = __ioctl,
    .fcntl      = __fcntl,
    .extention  = (void *)&fatfs_extention,
};
