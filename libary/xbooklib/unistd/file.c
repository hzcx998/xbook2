#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <sys/srvcall.h>
#include <srv/filesrv.h>

/* 任务可以打开的文件数量 */
#define _MAX_FILEDES_NR     32

struct _filedes {
    int flags;      /* 文件描述符的标志 */
    int fileidx;    /* 文件索引 */
};

struct _filedes __filedes_table[_MAX_FILEDES_NR] = {{0, -1}, }; 

#if 0
/* default work dir */
static char __current_work_dir[MAX_PATH_LEN] = {'c', ':', 0, };
#endif
static struct _filedes *__alloc_filedes()
{
    int i;
    for (i = 0; i < _MAX_FILEDES_NR; i++) {
        if (__filedes_table[i].flags == 0) {
            __filedes_table[i].flags = 1;
            __filedes_table[i].fileidx = -1;
            return &__filedes_table[i];
        }
    }
    return NULL;
}

static void __free_filedes(struct _filedes *_fil)
{
    _fil->flags = 0;
    _fil->fileidx = -1;
}

int open(const char *path, int flags)
{
    if (path == NULL)
        return -1;
    
    char *p = (char *) path;
#if 0    
    /* 对路径处理，传递的必须是绝对路径 */
    char *p = (char *) path;
    char *q = strchr(p, ':');
    char full_path[MAX_PATH_LEN] = {0};
    if (q == NULL) { /* 相对路径 */
        
        strcpy(full_path, __current_work_dir);
        if (*p != '/') {    /* 第一个字符不是'/'就需要追加一个 */
            strcat(full_path, "/");
        }   
        /* 追加路径 */
        strcat(full_path, path);
    } else {    /* 绝对路径 */
        strcpy(full_path, path);
    }
#endif    
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_OPEN, 0);
    SETSRV_ARG(&srvarg, 1, p, strlen(p) + 1);
    SETSRV_ARG(&srvarg, 2, flags, 0);
    SETSRV_RETVAL(&srvarg, -1);

    /* 文件描述符地址 */
    struct _filedes *_fil = __alloc_filedes();
    if (_fil == NULL) {
        return -1;        
    }
    
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        _fil->fileidx = GETSRV_RETVAL(&srvarg, int);
        return _fil - __filedes_table;
    }
    return -1;
}

int close(int fd)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_CLOSE, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        __free_filedes(_fil);
        return 0;
    }
    return -1;
}

static int __read(int fd, void *buffer, size_t nbytes)
{
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_READ, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    SETSRV_ARG(&srvarg, 2, buffer, nbytes);
    SETSRV_IO(&srvarg, (SRVIO_USER << 2));
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }
    return -1;
}


int read(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    if (buffer == NULL)
        return -1;

    char *buf = (char *) buffer;
    int count = (int )nbytes;
    int chunk = MIN(count, FILESRV_BUF_MAX_SIZE);
    int read_bytes = 0;
    int read_total = 0;
    do {
        read_bytes = __read(fd, buf, chunk);
        if (read_bytes <= 0) {  /* 读取失败 */
            return -1;
        }
        read_total += read_bytes;
        if (read_bytes < chunk) {   /* 读取完成，实际文件比传入的参数小 */
            break;
        }
        /* 读取一次成功，继续读取 */
        buf += chunk;
        nbytes -= chunk;
        count -= chunk;
        chunk = MIN(count, FILESRV_BUF_MAX_SIZE);
    } while (count > 0);
    return read_total;
}

static int __write(int fd, void *buffer, size_t nbytes)
{
    
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_WRITE, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    SETSRV_ARG(&srvarg, 2, buffer, nbytes);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }
    return -1;
}

int write(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    if (buffer == NULL)
        return -1;

    char *buf = (char *) buffer;
    int count = (int )nbytes;
    int chunk = MIN(count, FILESRV_BUF_MAX_SIZE);
    int write_bytes = 0;
    int write_total = 0;
    do {
        write_bytes = __write(fd, buf, chunk);
        if (write_bytes <= 0) {  /* 读取失败 */
            return -1;
        }
        write_total += write_bytes;
        if (write_bytes < chunk) {   /* 写入完成，实际文件比传入的参数小 */
            break;
        }
        /* 读取一次成功，继续读取 */
        buf += chunk;
        nbytes -= chunk;
        count -= chunk;
        chunk = MIN(count, FILESRV_BUF_MAX_SIZE);
    } while (count > 0);
    return write_total;
}

int lseek(int fd, off_t offset, int whence)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_LSEEK, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    SETSRV_ARG(&srvarg, 2, offset, 0);
    SETSRV_ARG(&srvarg, 3, whence, 0);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }
    return -1;
}

int access(const char *filenpath, int mode)
{
    if (filenpath == NULL)
        return -1;
    
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_ASSERT, 0);
    SETSRV_ARG(&srvarg, 1, filenpath, strlen(filenpath) + 1);
    SETSRV_ARG(&srvarg, 2, mode, 0);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }
    return -1;
}

int unlink(const char *path)
{
    if (path == NULL)
        return -1;

    char *p = (char *) path;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_UNLINK, 0);
    SETSRV_ARG(&srvarg, 1, p, strlen(p) + 1);
    SETSRV_RETVAL(&srvarg, -1);

    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int ftruncate(int fd, off_t offset)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FTRUNCATE, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    SETSRV_ARG(&srvarg, 2, offset, 0);
    
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int fsync(int fd)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FSYNC, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int fchmod(int fd, mode_t mode)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FCHMOD, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    SETSRV_ARG(&srvarg, 2, mode, 0);
    
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

int _eof(int fd)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FEOF, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }
    return -1;
}

int _error(int fd)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FERROR, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}

long tell(int fd)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FTELL, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }
    return -1;
}

size_t _size(int fd)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FSIZE, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }
    return -1;
}

int rewind(int fd)
{
    if (fd < 0 || fd >= _MAX_FILEDES_NR)
        return -1;
    struct _filedes *_fil = __filedes_table + fd;
    
    if (_fil->flags == 0)
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_REWIND, 0);
    SETSRV_ARG(&srvarg, 1, _fil->fileidx, 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}
