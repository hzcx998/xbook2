#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <fsal/dir.h>
#include <fsal/file.h>
#include <fsal/path.h>
#include "../fatfs/ff.h"
#include <xbook/diskman.h>
#include <xbook/memalloc.h>
#include <const.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>

// #define DEBUG_FATFS

typedef struct {
    FATFS *fsobj;        /* 挂载对象指针 */
} fatfs_extention_t;

fatfs_extention_t fatfs_extention;

int fatfs_drv_map[FF_VOLUMES] = {
    0,1,2,3,4,5,6,7,8,9
};

static int fsal_fatfs_mount(char *source, char *target, char *fstype, unsigned long flags)
{
    int solt = disk_info_find(source);
    if (solt < 0) {
        pr_dbg("%s: %s: not find device %s.\n", FS_MODEL_NAME,__func__, source);
        return -1;
    }
    /* 转换成fatfs的物理驱动器 */
    int pdrv, i;
    for (i = 0; i < FF_VOLUMES; i++) {
        if (fatfs_drv_map[i] == solt) {
            break;
        }
    }
    if (i >= FF_VOLUMES) {
        return -1;
    }
    /* 获取一个FATFS的物理磁盘驱动器 */
    pdrv = i;
    /* 构建挂载路径 */
    char path[3] = {pdrv + '0', ':', 0};

    int res = diskman.open(solt);
    if (res < 0) {
        pr_dbg("%s: %s: not find device %s.\n", FS_MODEL_NAME,__func__, source);
        return -1;
    }
    unsigned char buf[512];
    if (diskman.read(solt, 0, buf, SECTOR_SIZE) < 0) {
        diskman.close(solt);
        return -1;
    }
    diskman.close(solt);
    char remkfs = 1;
    if (buf[510] == 0x55 && buf[511] == 0xaa) {
        remkfs = 0;
        /* 强制要求格式化磁盘 */
        if (flags & MT_REMKFS) {
            remkfs = 1;
        }
    } else {
        pr_dbg("%s: %s: no fs on the disk %s.\n", FS_MODEL_NAME,__func__, source);
        return -1;
    }
    const TCHAR *p;
    if (remkfs) { 
        BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
        p = (const TCHAR *) path;
        MKFS_PARM parm = {FM_ANY, 0, 0, 0, 0};
        if (!strcmp(fstype, "fat12") || !strcmp(fstype, "fat16")) {
            parm.fmt = FM_FAT;
        } else if (!strcmp(fstype, "fat32")) {
            parm.fmt = FM_FAT32;
        } else if (!strcmp(fstype, "exfat")) {
            parm.fmt = FM_EXFAT;
        } 
        res = f_mkfs(p, (const MKFS_PARM *)&parm, work, sizeof(work));
        if (res != FR_OK) {
            pr_notice("%s: make fs on drive %s failed with resoult code %d.\n", FS_MODEL_NAME, p, res);
            return -1;
        }
    }

    FATFS *fsobj;           /* Filesystem object */
    fsobj = mem_alloc(sizeof(FATFS));
    if (fsobj == NULL) 
        return -1;
    memset(fsobj, 0, sizeof(FATFS));
    FRESULT fr;
    p = (const TCHAR *) path;

    BYTE delayed = !(flags & MT_DELAYED);  /* 延时挂载 */
    fr = f_mount(fsobj, p, delayed);
    if (fr != FR_OK) {
        pr_dbg("%s: %s: mount on path %s failed, code %d!\n", FS_MODEL_NAME,__func__, p, fr);
        mem_free(fsobj);
        return -1;
    }
    fatfs_extention.fsobj = fsobj;
    if (fsal_path_insert((void *)p, target, &fatfs_fsal)) {
        pr_dbg("%s: %s: insert path %s failed!\n", FS_MODEL_NAME,__func__, p);
        mem_free(fsobj);
        return -1;
    }
    return 0;
}

static int fsal_fatfs_unmount(char *path, unsigned long flags)
{
    /* 在末尾填0，只保留磁盘符和分隔符 */
    path[2] = '\0';
    FRESULT res;
    const TCHAR *p = (const TCHAR *) path;
    fatfs_extention_t *ext = &fatfs_extention;
    res = f_unmount(p);
    if (res != FR_OK) {
        pr_dbg("%s: %s: unmount on path %s failed, code %d.\n", FS_MODEL_NAME,__func__, p, res);
        return -1;
    }
    if (fsal_path_remove((void *) p)) {
        pr_dbg("%s: %s: remove path %s failed!\n", FS_MODEL_NAME,__func__, p);
        return -1;
    }
    mem_free(ext->fsobj);
    ext->fsobj = NULL;
    return 0;
}

static int fsal_fatfs_mkfs(char *source, char *fstype, unsigned long flags)
{
    int solt = disk_info_find(source);
    if (solt < 0)
        return -1;
    int pdrv, i;
    for (i = 0; i < FF_VOLUMES; i++) {
        if (fatfs_drv_map[i] == solt) {
            break;
        }
    }
    if (i >= FF_VOLUMES) {
        return -1;
    }
    pdrv = i;
    char path[3] = {pdrv + '0', ':', 0};
    FRESULT res;        /* API result code */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
    const TCHAR *p = (const TCHAR *) path;
    MKFS_PARM parm = {FM_ANY, 0, 0, 0, 0};
    if (!strcmp(fstype, "fat12") || !strcmp(fstype, "fat16")) {
        parm.fmt = FM_FAT;
    } else if (!strcmp(fstype, "fat32")) {
        parm.fmt = FM_FAT32;
    } else if (!strcmp(fstype, "exfat")) {
        parm.fmt = FM_EXFAT;
    }
    res = f_mkfs(p, (const MKFS_PARM *)&parm, work, sizeof(work));
    if (res != FR_OK) {
        pr_dbg("%s:: %s: make fs on drive %s failed with resoult code %d.\n", FS_MODEL_NAME,__func__, p, res);
        return -1;
    }
    return 0;
}

static int fsal_fatfs_open(void *path, int flags)
{
    fsal_file_t *fp = fsal_file_alloc();
    if (fp == NULL)
        return -1;
    fp->fsal = &fatfs_fsal;
    const TCHAR *p = (const TCHAR *) path;

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
    } else if (flags & O_APPEND) {
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
    return FSAL_FILE2IDX(fp);
}

static int fsal_fatfs_close(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))
        return -1;
    FRESULT fres;
    fres = f_close(&fp->file.fatfs);
    if (fres != FR_OK) {    
        pr_err("[fatfs]: close file failed!\n");
        return -1;
    }
    if (fsal_file_free(fp) < 0)
        return -1;
    return 0;
}

/**
 * The data of multiple sectors read once may not be read correctly. In this case,
 *  the data is read in units of 1 sector
 */
static int fsal_fatfs_read(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))
        return -1;
    
    FRESULT fr;
    UINT br;
    UINT readbytes = 0;
    
    // read 512 bytes each time
    UINT chunk;
    uint8_t *p = (uint8_t *) buf;
    chunk =  size % (SECTOR_SIZE); // read mini block
    while (size > 0) {
        br = 0;
        fr = f_read(&fp->file.fatfs, p, chunk, &br);
        if (fr != FR_OK && !readbytes) { // first time read get error
            pr_err("fatfs: f_read: err code %d\n", fr);
            return -1;
        } else  if (fr != FR_OK ) { // next time read over
            pr_err("fatfs: f_read: err code %d, rd=%d br=%d\n", fr, readbytes, br);
            return readbytes + br;
        }
        p += chunk;
        size -= chunk;
        chunk =  (SECTOR_SIZE ); // read 512 bytes block
        readbytes += br;
    }
    return readbytes;
}

static int fsal_fatfs_write(int idx, void *buf, size_t size)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp)) 
        return -1;
    FRESULT fr;
    UINT bw;
    fr = f_write(&fp->file.fatfs, buf, size, &bw);
    if (fr != FR_OK)
        return -1;
    return bw;
}

static int fsal_fatfs_lseek(int idx, off_t offset, int whence)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))
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

static int fsal_fatfs_opendir(char *path)
{
    fsal_dir_t *pdir = fsal_dir_alloc();
    if (pdir == NULL)
        return -1;
    pdir->fsal = &fatfs_fsal;
    FRESULT res;
    res = f_opendir(&pdir->dir.fatfs, path);                       /* Open the directory */
    if (res != FR_OK) {
        fsal_dir_free(pdir);
        return -1;
    }
    return FSAL_D2I(pdir);
}

static int fsal_fatfs_closedir(int idx)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)
        return -1;
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

static int fsal_fatfs_readdir(int idx, void *buf)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)   
        return -1;
    
    FRESULT fres;
    FILINFO finfo;
    fres = f_readdir(&pdir->dir.fatfs, &finfo);
    if (fres != FR_OK) {
        return -1;
    }
    if (finfo.fname[0] == '\0') 
        return -1;
    dirent_t *dire = (dirent_t *)buf;
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

static int fsal_fatfs_mkdir(char *path, mode_t mode)
{
    FRESULT res;
    res = f_mkdir(path);
    if (res != FR_OK) {
        return -1;
    }
    return 0;
}

static int fsal_fatfs_unlink(char *path)
{
    FRESULT res;
    res = f_unlink(path);
    if (res != FR_OK) {
        return -1;
    }
    return 0;
}

static int fsal_fatfs_rename(char *old_path, char *new_path)
{
    FRESULT res;
    res = f_rename(old_path, new_path);
    if (res != FR_OK) {
        return -1;
    }
    return 0;
}

static int fsal_fatfs_ftruncate(int idx, off_t offset)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    
    off_t old = f_tell(&fp->file.fatfs);

    FRESULT fres;
    fres = f_lseek(&fp->file.fatfs, offset);
    if (fres != FR_OK) {
        return -1;
    }
    fres = f_truncate(&fp->file.fatfs);
    if (fres != FR_OK) {
        f_lseek(&fp->file.fatfs, old);
        return -1;
    }
    return 0;
}

static int fsal_fatfs_fsync(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    FRESULT fres;
    fres = f_sync(&fp->file.fatfs);
    if (fres != FR_OK) {
        return -1;
    }
    return 0;
}

static int fsal_fatfs_state(char *path, void *buf)
{
    FRESULT fres;
    FILINFO finfo;
    fres = f_stat(path, &finfo);
    if (fres != FR_OK) {
        return -1;
    }
    stat_t *stat = (stat_t *)buf;
    mode_t mode = S_IREAD | S_IWRITE;
    if (finfo.fattrib & AM_RDO)
        mode &= ~S_IWRITE;
    if (finfo.fattrib & AM_DIR)
        mode |= S_IFDIR;
    else
        mode |= S_IFREG;
    stat->st_mode = mode;
    stat->st_size = finfo.fsize;
    stat->st_atime = (finfo.fdate << 16) | finfo.ftime;
    stat->st_ctime = stat->st_mtime = stat->st_atime;
    return 0;
}

static int fsal_fatfs_fstat(int idx, void *buf)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    
    /* NOTICE: FATFS暂时不支持 */
    return -1;
}

static int fsal_fatfs_chmod(char *path, mode_t mode)
{
    /* NOTICE: FATFS暂时不支持 */
    return 0;
}

static int fsal_fatfs_fchmod(int idx, mode_t mode)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    return 0;
}

static int fsal_fatfs_utime(char *path, time_t actime, time_t modtime)
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
 * 执行失败返回-1，已经到达末尾返回1，没有到达末尾返回0
 */
static int fsal_fatfs_feof(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    return f_eof(&fp->file.fatfs);
}

/**
 * 执行失败返回-1，出错则返回1，没有出错返回0
 */
static int fsal_fatfs_ferror(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    return f_error(&fp->file.fatfs);
}

static off_t fsal_fatfs_ftell(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    return f_tell(&fp->file.fatfs);
}

/**
 * 执行失败返回0，成功返回文件大小
 */
static size_t fsal_fatfs_fsize(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    return f_size(&fp->file.fatfs);
}

static int fsal_fatfs_rewind(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    return f_rewind(&fp->file.fatfs);
}

static int fsal_fatfs_rewinddir(int idx)
{
    if (ISBAD_FSAL_DIDX(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)   
        return -1;
    return f_rewinddir(&pdir->dir.fatfs);
}

static int fsal_fatfs_rmdir(char *path)
{
    FRESULT res;
    res = f_rmdir(path);
    if (res != FR_OK) {
        return -1;
    }
    return 0;
}

static int fsal_fatfs_chdir(char *path)
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

static int fsal_fatfs_ioctl(int fd, int cmd, unsigned long arg)
{
    /* NOTICE: FATFS暂时不支持 */
    return 0;
}

static int fsal_fatfs_fcntl(int fd, int cmd, long arg)
{
    /* NOTICE: FATFS暂时不支持 */
    return 0;
}

static int fsal_fatfs_access(const char *path, int mode)
{
    if (mode == F_OK) {
        FRESULT fr;
        FIL fil;
        fr = f_open(&fil, path,  FA_OPEN_EXISTING | FA_READ);
        if (fr != FR_OK) {
            return -1;
        }
        f_close(&fil); 
        return 0; 
    }
    return -1;
}

/* fatfs 支持的文件系统类型 */
static char *fatfs_sub_table[] = {
    "fat12",
    "fat16",
    "fat32",
    "exfat",
    NULL           /* 最后一个成员必须是NULL */
};

fsal_t fatfs_fsal = {
    .list       = LIST_HEAD_INIT(fatfs_fsal.list),
    .name       = "fatfs",
    .subtable   = fatfs_sub_table,
    .mkfs       =fsal_fatfs_mkfs,
    .mount      =fsal_fatfs_mount,
    .unmount    =fsal_fatfs_unmount,
    .open       =fsal_fatfs_open,
    .close      =fsal_fatfs_close,
    .read       =fsal_fatfs_read,
    .write      =fsal_fatfs_write,
    .lseek      =fsal_fatfs_lseek,
    .opendir    =fsal_fatfs_opendir,
    .closedir   =fsal_fatfs_closedir,
    .readdir    =fsal_fatfs_readdir,
    .mkdir      =fsal_fatfs_mkdir,
    .unlink     =fsal_fatfs_unlink,
    .rename     =fsal_fatfs_rename,
    .ftruncate  =fsal_fatfs_ftruncate,
    .fsync      =fsal_fatfs_fsync,
    .state      =fsal_fatfs_state,
    .chmod      =fsal_fatfs_chmod,
    .fchmod     =fsal_fatfs_fchmod,
    .utime      =fsal_fatfs_utime,
    .feof       =fsal_fatfs_feof,
    .ferror     =fsal_fatfs_ferror,
    .ftell      =fsal_fatfs_ftell,
    .fsize      =fsal_fatfs_fsize,
    .rewind     =fsal_fatfs_rewind,
    .rewinddir  =fsal_fatfs_rewinddir,
    .rmdir      =fsal_fatfs_rmdir,
    .chdir      =fsal_fatfs_chdir,
    .ioctl      =fsal_fatfs_ioctl,
    .fcntl      =fsal_fatfs_fcntl,
    .fstat      =fsal_fatfs_fstat,
    .access     =fsal_fatfs_access,
    .extention  = (void *)&fatfs_extention,
};
