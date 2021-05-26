#include <xbook/fsal.h>
#include <xbook/fatfs.h>
#include <xbook/dir.h>
#include <xbook/file.h>
#include <xbook/path.h>
#include "../fatfs/ff.h"
#include <xbook/diskman.h>
#include <xbook/memalloc.h>
#include <xbook/walltime.h>
#include <const.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/dir.h>
#include <sys/stat.h>

// #define DEBUG_FATFS

#define PDRV_TO_PATH(n) (n + '0')
#define PATH_TO_PDRV(n) (n - '0')

typedef struct {
    FATFS *fsobj[FF_VOLUMES];       /* 挂载对象指针 */
    uint16_t mount_time[FF_VOLUMES];  /* 挂载时的时间 */
    uint16_t mount_date[FF_VOLUMES];  /* 挂载时的日期 */
} fatfs_extention_t;

typedef struct {
    DIR dir;
} fatfs_dir_extention_t;

typedef struct {
    FIL file;
    char *dir_path;     /* 目录路径：只有被当做目录打开时才有效，默认为NULL */
} fatfs_file_extention_t;

fatfs_extention_t fatfs_extention;

int fatfs_drv_map[FF_VOLUMES] = {
    0,1,2,3,4,5,6,7,8,9
};

static int disk_has_fs(unsigned char *buf)
{
    int i;
    #if 0
    for (i = 0; i < SECTOR_SIZE; i ++) {
        if (0 == i % 16) {
            dbgprint("\n");
        }
        dbgprint("%x ", buf[i]);
    }
    dbgprint("\n");
    #endif
    for (i = 0; i < SECTOR_SIZE; i++)
        if (buf[i])
            return 1;
    return 0;
}

static int fsal_fatfs_mount(char *source, char *target, char *fstype, unsigned long flags)
{
    //dbgprint("%s: %s: source %s target %s type %s.\n", FS_MODEL_NAME,__func__, source, target, fstype);

    int solt = disk_info_find_with_path(source);
    if (solt < 0) {
        dbgprint("%s: %s: not find device %s.\n", FS_MODEL_NAME,__func__, source);
        return -1;
    }
    /* 查看该设备是否已经映射过了 */
    if (fsal_path_find_device(source)) {
        errprint("device %s had mounted!\n", source);
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
    /* 构建挂载路径, [0-9]: */
    char path[3] = {PDRV_TO_PATH(pdrv), ':', 0};

    int res = diskman.open(solt);
    if (res < 0) {
        dbgprint("%s: %s: not find device %s.\n", FS_MODEL_NAME,__func__, source);
        return -1;
    }
    unsigned char buf[SECTOR_SIZE];
    if (diskman.read(solt, 0, buf, SECTOR_SIZE) < 0) {
        diskman.close(solt);
        return -1;
    }
    diskman.close(solt);
    char remkfs = 1;
    if (disk_has_fs(buf)) {
        remkfs = 0;
        /* 强制要求格式化磁盘 */
        if (flags & MT_REMKFS) {
            remkfs = 1;
        }
    } else {
        if (!(flags & MT_REMKFS)) {
            dbgprint("%s: %s: no fs on the disk %s.\n", FS_MODEL_NAME,__func__, source);
            return -1;
        }
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
            noteprint("%s: make fs on drive %s failed with resoult code %d.\n", FS_MODEL_NAME, p, res);
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
        dbgprint("%s: %s: mount on path %s failed, code %d!\n", FS_MODEL_NAME,__func__, p, fr);
        mem_free(fsobj);
        return -1;
    }
    fatfs_extention.fsobj[pdrv] = fsobj;
    fatfs_extention.mount_time[pdrv] = WTM_WR_TIME(walltime.hour, walltime.minute, walltime.second);
    fatfs_extention.mount_date[pdrv] = WTM_WR_DATE(walltime.year, walltime.month, walltime.day);
        
    if (fsal_path_insert(source, (void *)p, target, &fatfs_fsal)) {
        dbgprint("%s: %s: insert path %s failed!\n", FS_MODEL_NAME,__func__, p);
        mem_free(fsobj);
        return -1;
    }
    return 0;
}

static int fsal_fatfs_unmount(char *path, unsigned long flags)
{
    //dbgprint("%s: %s: path %s.\n", FS_MODEL_NAME,__func__, path);

    /* 检查路径是否为物理路径或者虚拟路径 */
    if (fsal_path_find(path, 0) < 0 && fsal_path_find_device(path) < 0) {
        errprint("unmount: path %s not mount!\n", path);
        return -1;
    }

    /* 在末尾填0，只保留磁盘符和分隔符 */
    path[2] = '\0';
    FRESULT res;
    const TCHAR *p = (const TCHAR *) path;
    fatfs_extention_t *ext = &fatfs_extention;
    res = f_unmount(p);
    if (res != FR_OK) {
        dbgprint("%s: %s: unmount on path %s failed, code %d.\n", FS_MODEL_NAME,__func__, p, res);
        return -1;
    }
    if (fsal_path_remove((void *) p)) {
        dbgprint("%s: %s: remove path %s failed!\n", FS_MODEL_NAME,__func__, p);
        return -1;
    }
    int pdrv = PATH_TO_PDRV(path[0]);
    mem_free(ext->fsobj[pdrv]);
    ext->fsobj[pdrv] = NULL;
    return 0;
}

static int fsal_fatfs_mkfs(char *source, char *fstype, unsigned long flags)
{
    int solt = disk_info_find_with_path(source);
    if (solt < 0) {
        errprint("fasl: mkfs: not found device %s!\n", source);
        return -1;
    }
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
    char path[3] = {PDRV_TO_PATH(pdrv), ':', 0};
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
        dbgprint("%s:: %s: make fs on drive %s failed with resoult code %d.\n", FS_MODEL_NAME,__func__, p, res);
        return -1;
    }
    return 0;
}

static int fsal_fatfs_open(void *path, int flags)
{
    fsal_file_t *fp = fsal_file_alloc();
    if (fp == NULL)
        return -ENOMEM;
    // errprint("fatfs: open path %s flags %d\n", path, flags);
    const TCHAR *p = (const TCHAR *) path;
    fp->extension = mem_alloc(sizeof(fatfs_file_extention_t));
    if (!fp->extension) {
        errprint("fatfs: alloc file %s extension for open failed!\n", p);
        fsal_file_free(fp);
        return -ENOMEM;
    }
    fatfs_file_extention_t *extension = (fatfs_file_extention_t *) fp->extension;
    extension->dir_path = NULL;     /* 普通文件时为NULL，为目录时才有效 */

    fp->fsal = &fatfs_fsal;

    BYTE mode = 0;  /* 文件打开模式 */
    if (flags & O_RDONLY) {
        mode |= FA_READ;
    } else if (flags & O_WRONLY) {
        mode |= FA_WRITE;
    } else if (flags & O_RDWR) {
        mode |= FA_READ | FA_WRITE;
    }
    if (flags & O_EXCL) {
        mode |= FA_CREATE_NEW;
    } else if (flags & O_TRUNC) {
        mode |= FA_CREATE_ALWAYS;
    } else if (flags & O_APPEND) {
        mode |= FA_OPEN_APPEND;
    } else if (flags & O_CREAT) {
        mode |= FA_OPEN_ALWAYS;
    }

    /* 如果要打开一个目录，那么就检测目录是否存在，然后返回指向这个目录的文件就行了 */
    if (flags & O_DIRECTORY) {
        /* 检测目录是否存在 */
        FRESULT res;
        DIR dir;
        res = f_opendir(&dir, p); /* Open the directory */
        if (res != FR_OK) {
            errprintln("[fs] fatfs open: dir %s not exist", p);
            mem_free(fp->extension);
            fsal_file_free(fp);
            return -ENOFILE;
        }
        res = f_closedir(&dir);
        if (res != FR_OK) {
            errprintln("[fs] fatfs open: close dir %s error", p);
            mem_free(fp->extension);
            fsal_file_free(fp);
            return -EPERM;
        }
        extension->dir_path = mem_alloc(MAX_PATH);
        if (extension->dir_path == NULL) {
            errprintln("[fs] fatfs open: memory alloc for dir path %s error", p);
            mem_free(fp->extension);
            fsal_file_free(fp);
            return -ENOMEM;
        }
        strncpy(extension->dir_path, p, MAX_PATH);   /* save path */
    } else {
        FRESULT fres;
        fres = f_open((FIL *)&extension->file, p, mode);
        if (fres != FR_OK) {
            // errprint("fatfs: open file %s failed! errcode:%d, flags:%x\n", p, fres, flags);
            mem_free(fp->extension);
            fsal_file_free(fp);
            return -ENOFILE;
        }
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
    fatfs_file_extention_t *extension = (fatfs_file_extention_t *) fp->extension;
    if (extension->dir_path != NULL) {  /* Is director */
        mem_free(extension->dir_path);
        dbgprintln("[fs] fatfs close: path %s", extension->dir_path);
    } else {
        FRESULT fres;
        fres = f_close((FIL *)fp->extension);
        if (fres != FR_OK) {    
            errprint("[fatfs]: close file failed!\n");
            return -1;
        }
    }
    if (fp->extension)
        mem_free(fp->extension);
    fp->extension = NULL;
    if (fsal_file_free(fp) < 0)
        return -1;
    return 0;
}

static char *fsal_fatfs_dirfd_path(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return NULL;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))
        return NULL;
    fatfs_file_extention_t *extension = (fatfs_file_extention_t *) fp->extension;
    return extension->dir_path;
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
        fr = f_read((FIL *)fp->extension, p, chunk, &br);
        if (fr != FR_OK && !readbytes) { // first time read get error
            errprint("fatfs: f_read: err code %d\n", fr);
            return -1;
        } else  if (fr != FR_OK ) { // next time read over
            errprint("fatfs: f_read: err code %d, rd=%d br=%d\n", fr, readbytes, br);
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
    fr = f_write((FIL *)fp->extension, buf, size, &bw);
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
        new_off = f_tell((FIL *)fp->extension) + offset;
        break;
    case SEEK_END:
        new_off = f_size((FIL *)fp->extension) + offset;
        break;
    default:
        break;
    }
    FRESULT fr;
    fr = f_lseek((FIL *)fp->extension, new_off);
    if (fr != FR_OK)
        return -1;
    
    return new_off;
}

static int fsal_fatfs_opendir(char *path)
{
    fsal_dir_t *pdir = fsal_dir_alloc();
    if (!pdir)
        return -1;
    pdir->extension = mem_alloc(sizeof(fatfs_dir_extention_t));
    if (!pdir->extension) {
        fsal_dir_free(pdir);
        return -ENOMEM;
    }

    pdir->fsal = &fatfs_fsal;
    FRESULT res;
    res = f_opendir((DIR *)pdir->extension, path);                       /* Open the directory */
    if (res != FR_OK) {
        mem_free(pdir->extension);
        fsal_dir_free(pdir);
        return -1;
    }
    return FSAL_D2I(pdir);
}

static int fsal_fatfs_closedir(int idx)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)
        return -1;
    
    FRESULT fres;
    fres = f_closedir((DIR *)pdir->extension);
    if (fres != FR_OK) {
        return -1;
    }
    if (pdir->extension)
        mem_free(pdir->extension);
    pdir->extension = NULL;
    if (fsal_dir_free(pdir) < 0)
        return -1;
    return 0;
}

static int fsal_fatfs_readdir(int idx, void *buf)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)   
        return -1;
    
    FRESULT fres;
    FILINFO finfo;
    fres = f_readdir((DIR *)pdir->extension, &finfo);
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
    
    off_t old = f_tell((FIL *)fp->extension);

    FRESULT fres;
    fres = f_lseek((FIL *)fp->extension, offset);
    if (fres != FR_OK) {
        return -1;
    }
    fres = f_truncate((FIL *)fp->extension);
    if (fres != FR_OK) {
        f_lseek((FIL *)fp->extension, old);
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
    fres = f_sync((FIL *)fp->extension);
    if (fres != FR_OK) {
        return -1;
    }
    return 0;
}

static int is_root_dir(char *path)
{
    char *p = path;
    int n = 0;
    while (*p) {
        switch (n) {
        case 0:
            if (*p >= '0' && *p <= '9')
                n++;
            break;
        case 1:
            if (*p == ':')
                n++;
            break;
        default:
            n = -1;
            break;
        }
        p++;
    }
    return (n == 2);
}

static int fsal_fatfs_state(char *path, void *buf)
{
    FRESULT fres;
    FILINFO finfo;
    /* 对根目录进行特殊处理 */
    if (is_root_dir(path)) {
        int pdrv = PATH_TO_PDRV(path[0]);
        finfo.fsize = 0;
        finfo.fdate = fatfs_extention.mount_date[pdrv];
        finfo.ftime = fatfs_extention.mount_time[pdrv];
        finfo.fattrib = (AM_RDO | AM_DIR);
    } else {
        fres = f_stat(path, &finfo);
        if (fres != FR_OK) {
            // keprint("state: path %s error with status %d\n", path, fres);
            return -EINVAL;
        }
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
    return -ENOSYS;
}

static int fsal_fatfs_chmod(char *path, mode_t mode)
{
    /* NOTICE: FATFS暂时不支持 */
    return -ENOSYS;
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
    return f_eof((FIL *)fp->extension);
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
    return f_error((FIL *)fp->extension);
}

static off_t fsal_fatfs_ftell(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    return f_tell((FIL *)fp->extension);
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
    return f_size((FIL *)fp->extension);
}

static int fsal_fatfs_rewind(int idx)
{
    if (FSAL_BAD_FILE_IDX(idx))
        return -1;
    fsal_file_t *fp = FSAL_IDX2FILE(idx);
    if (FSAL_BAD_FILE(fp))   
        return -1;
    return f_rewind((FIL *)fp->extension);
}

static int fsal_fatfs_rewinddir(int idx)
{
    if (FSAL_IS_BAD_DIR(idx))
        return -1;
    fsal_dir_t *pdir = FSAL_I2D(idx);
    if (!pdir->flags)   
        return -1;
    return f_rewinddir((DIR *)pdir->extension);
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

static int fsal_fatfs_ioctl(int fd, int cmd, void *arg)
{
    /* NOTICE: FATFS暂时不支持 */
    return -ENOSYS;
}

static int fsal_fatfs_fcntl(int fd, int cmd, long arg)
{
    /* NOTICE: FATFS暂时不支持 */
    return -ENOSYS;
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
    .dirfd_path =fsal_fatfs_dirfd_path,
    .extention  = (void *)&fatfs_extention,
};
