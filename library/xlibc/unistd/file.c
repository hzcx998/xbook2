#include <unistd.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/srvcall.h>
#include <srv/filesrv.h>
#include <sys/dir.h>
#include <sys/filedes.h>

struct _filedes __filedes_table[_MAX_FILEDES_NR] = {{0, -1}, }; 

struct _filedes *__alloc_filedes()
{
    int i;
    for (i = 0; i < _MAX_FILEDES_NR; i++) {
        if (__filedes_table[i].flags == 0) {
            __filedes_table[i].flags = _FILE_USING;
            __filedes_table[i].handle = -1;
            return &__filedes_table[i];
        }
    }
    return NULL;
}

void __free_filedes(struct _filedes *_fil)
{
    _FILE_FLAGS(_fil) = 0;
    _FILE_HANDLE(_fil) = -1;
}

int open(const char *path, int flags)
{
    if (path == NULL)
        return -1;
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    char *p = (char *) full_path;
  
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
            __free_filedes(_fil);
            return -1;
        }
        _FILE_HANDLE(_fil) = GETSRV_RETVAL(&srvarg, int);
        _FILE_FLAGS(_fil) |= _FILE_NORMAL;
        return _FILE_TO_FD(_fil);
    }
    return -1;
}

static int fileclose(int fd)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_CLOSE, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        __free_filedes(_fil);
        return 0;
    }
    return -1;
}

int close(int fd)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    if (_FILE_FLAGS(_fil) & _FILE_NORMAL) {
        return fileclose(fd);
    } else if (_FILE_FLAGS(_fil) & _FILE_SOCKET) {
        return sockclose(fd);
    }
    return -1;
}

static int __read(int fd, void *buffer, size_t nbytes)
{
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_READ, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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


int fileread(int fd, void *buffer, size_t nbytes)
{
    if (_IS_BAD_FD(fd))
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

int read(int fd, void *buffer, size_t nbytes)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    if (_FILE_FLAGS(_fil) & _FILE_NORMAL) {
        return fileread(fd, buffer, nbytes);
    } else if (_FILE_FLAGS(_fil) & _FILE_SOCKET) {
        return sockread(fd, buffer, nbytes);
    }
    return -1;
}

static int __write(int fd, void *buffer, size_t nbytes)
{
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_WRITE, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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

int filewrite(int fd, void *buffer, size_t nbytes)
{
    if (_IS_BAD_FD(fd))
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

int write(int fd, void *buffer, size_t nbytes)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    if (_FILE_FLAGS(_fil) & _FILE_NORMAL) {
        return filewrite(fd, buffer, nbytes);
    } else if (_FILE_FLAGS(_fil) & _FILE_SOCKET) {
        return sockwrite(fd, buffer, nbytes);
    }
    return -1;
}

int fileioctl(int fd, int cmd, unsigned long arg)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_IOCTL, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
    SETSRV_ARG(&srvarg, 2, cmd, 0);
    SETSRV_ARG(&srvarg, 3, arg, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }

    return -1;
}

int ioctl(int fd, int cmd, unsigned long arg)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    if (_FILE_FLAGS(_fil) & _FILE_NORMAL) {
        return fileioctl(fd, cmd, arg);
    } else if (_FILE_FLAGS(_fil) & _FILE_SOCKET) {
        return sockioctl(fd, cmd, (void *) arg);
    }
    return -1;
}

int fcntl(int fd, int cmd, long arg)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FCNTL, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
    SETSRV_ARG(&srvarg, 2, cmd, 0);
    SETSRV_ARG(&srvarg, 3, arg, 0);
    SETSRV_RETVAL(&srvarg, -1);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return GETSRV_RETVAL(&srvarg, int);
    }

    return -1;
}


int lseek(int fd, off_t offset, int whence)
{
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_LSEEK, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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
    char full_path[MAX_PATH] = {0};
    build_path(filenpath, full_path);

    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_ASSERT, 0);
    SETSRV_ARG(&srvarg, 1, full_path, strlen(full_path) + 1);
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
    char full_path[MAX_PATH] = {0};
    build_path(path, full_path);

    char *p = (char *) full_path;
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
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FTRUNCATE, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FSYNC, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FCHMOD, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FEOF, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FERROR, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FTELL, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_FSIZE, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
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
    if (_IS_BAD_FD(fd))
        return -1;
    struct _filedes *_fil = _FD_TO_FILE(fd);
    
    if (_INVALID_FILE(_fil))
        return -1;
    DEFINE_SRVARG(srvarg);
    SETSRV_ARG(&srvarg, 0, FILESRV_REWIND, 0);
    SETSRV_ARG(&srvarg, 1, _FILE_HANDLE(_fil), 0);
    if (!srvcall(SRV_FS, &srvarg)) {
        if (GETSRV_RETVAL(&srvarg, int) == -1) {
            return -1;
        }
        return 0;
    }
    return -1;
}
