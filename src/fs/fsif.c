#include <xbook/fsal.h>
#include <xbook/fd.h>
#include <xbook/fs.h>
#include <xbook/task.h>
#include <xbook/debug.h>
#include <xbook/driver.h>
#include <xbook/schedule.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <xbook/fifo.h>
#include <xbook/pipe.h>
#include <xbook/safety.h>
#include <xbook/account.h>
#include <xbook/dir.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

// #define DEBUG_FSIF

#define FSIF_USER_CHECK

#define FSIF_RW_CHUNK_SIZE  (PAGE_SIZE * 2)
#define FSIF_RW_BUF_SIZE    512

/**
 * 不从用户态复制路径，可以在内核中使用
 */
int __sys_open(char *path, int flags)
{
    if (!fsif.open)
        return -ENOSYS;
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    if (account_selfcheck_permission((char *)abs_path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    int handle = fsif.open((void *)abs_path, flags);
    if (handle < 0)
        return -ENOFILE;
    return local_fd_install(handle, FILE_FD_NORMAL);
}

int sys_open(const char *path, int flags)
{
    if (!path)
        return -EINVAL; 
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)path, MAX_PATH) < 0) {
        return -EINVAL;
    }
    return __sys_open(_path, flags);
}

static char *fsif_dirfd_path(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return NULL;
    if (!ffd->fsal->dirfd_path)
        return NULL;
    return ffd->fsal->dirfd_path(ffd->handle);
}

/**
 * 打开一个文件
 * 如果pathname是绝对路径，那么dirfd被忽略，功能和open一样。
 * 如果pathname是想对路径，那么就看dirfd是什么，如果是AT_FDCWD，
 * 那么就会把pathname和cwd进行合并，得出最终的路径。不然，就会把
 * dirfd对应的路径和pathname进行合并，得出最后的路径。
 */
int sys_openat(int dirfd, const char *pathname, int flags, mode_t mode)
{
    if (!pathname)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)pathname, MAX_PATH) < 0) {
        return -EINVAL;
    }
    // dbgprintln("[fs] sys_openat: dirfd %d, pathname %s, flags %d", dirfd, _path, flags);
    if (_path[0] == '/' && _path[1] == '\0') {
        dbgprintln("[fs] sys_openat: can't open root dir '/' directly");
        return -EINVAL;
    }
    if (_path[0] == '/' && _path[1] != '\0') {
        return __sys_open(_path, flags);
    }
    if (dirfd == AT_FDCWD) {    /* 在进程的cwd目录后面 */
        return __sys_open(_path, flags);
    }
    char *dirpath = fsif_dirfd_path(dirfd);
    if (dirpath == NULL) {
        dbgprintln("[fs] sys_openat: dirfd %d not a director", dirfd);
        return -ENFILE; /* fd没有对应目录 */
    }
    char oldcwd[MAX_PATH] = {0};    /* save old cwd as dirpath */
    if (kfile_getcwd(oldcwd, MAX_PATH) < 0) {
        dbgprintln("[fs] sys_openat: get cwd error");
        return -EPERM;
    }
    task_t *cur = task_current;
    task_set_cwd(cur, dirpath); /* set cwd as dirpath */
    int newfd = __sys_open(_path, flags); /* do open file */
    if (newfd < 0) {
        dbgprintln("[fs] sys_openat: open file %s error", _path);
    }
    task_set_cwd(cur, oldcwd); /* restore cwd as old cwd */
    return newfd;
}

int sys_close(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->close)
        return -ENOSYS;
    if (ffd->fsal->close(ffd->handle) < 0) {
        return -1;
    }
    return local_fd_uninstall(fd);
}

static int read_large(file_fd_t *ffd, void *buffer, size_t nbytes)
{
    char *_mbuf = mem_alloc(FSIF_RW_CHUNK_SIZE);
    if (_mbuf == NULL) {
        return -ENOMEM;
    }
    int total = 0;
    char *p = (char *)buffer;
    size_t chunk = nbytes % FSIF_RW_CHUNK_SIZE;
    while (nbytes > 0) {
        int rd = ffd->fsal->read(ffd->handle, _mbuf, chunk);
        if (rd < 0) {
            errprintln("[fs] sys_read: handle %d do read failed!", ffd->handle);
            total = -EIO;
            break;
        }
        if (mem_copy_to_user(p, _mbuf, chunk) < 0) {
            errprintln("[fs] sys_read: copy buf %p to user failed!", p);
            total = -EINVAL;
            break;
        }
        // dbgprintln("[fs] sys_write: chunk %d wr %d\n", chunk, wr);
        p += chunk;
        total += rd;
        nbytes -= chunk;
        chunk = FSIF_RW_CHUNK_SIZE;
    }
    mem_free(_mbuf);
    return total;
}

int sys_read(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        errprint("[fs] sys_read: fd %d err!\n", fd);
        return -EINVAL;
    }
    if (!ffd->fsal->read)
        return -ENOSYS;
    // dbgprint("[fs] read fd %d, bytes %d\n", fd, nbytes);
    if (nbytes > FSIF_RW_BUF_SIZE) {
        return read_large(ffd, buffer, nbytes);
    } else {
        char _buf[FSIF_RW_BUF_SIZE] = {0};
        int rd = ffd->fsal->read(ffd->handle, _buf, nbytes);
        if (rd > 0) {
            if (mem_copy_to_user(buffer, _buf, rd) < 0) {
                errprintln("[fs] sys_read: copy buf %p to user failed!", buffer);
                return -EINVAL;
            }
        }
        return rd;
    }
}

int sys_fastread(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        errprint("[FS]: %s: fd %d err!\n", __func__, fd);
        return -EINVAL;
    }
    if (!ffd->fsal->fastread)
        return -ENOSYS;
    return ffd->fsal->fastread(ffd->handle, buffer, nbytes);
}

int sys_fastwrite(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        errprint("[FS]: %s: fd %d err!\n", __func__, fd);
        return -EINVAL;
    }
    if (!ffd->fsal->fastwrite)
        return -ENOSYS;
    return ffd->fsal->fastwrite(ffd->handle, buffer, nbytes);
}

static int write_large(file_fd_t *ffd, void *buffer, size_t nbytes)
{
    char *_mbuf = mem_alloc(FSIF_RW_CHUNK_SIZE);
    if (_mbuf == NULL) {
        return -ENOMEM;
    }
    int total = 0;
    char *p = (char *)buffer;
    size_t chunk = nbytes % FSIF_RW_CHUNK_SIZE;
    while (nbytes > 0) {
        if (mem_copy_from_user(_mbuf, p, chunk) < 0) {
            errprintln("[fs] sys_write: copy buf %p from user failed!", p);
            total = -EINVAL;
            break;
        }
        int wr = ffd->fsal->write(ffd->handle, _mbuf, chunk);
        if (wr < 0) {
            errprintln("[fs] sys_write: handle %d do write failed!", ffd->handle);
            total = -EIO;
            break;
        }
        // dbgprintln("[fs] sys_write: chunk %d wr %d\n", chunk, wr);
        p += chunk;
        total += wr;
        nbytes -= chunk;
        chunk = FSIF_RW_CHUNK_SIZE;
    }
    mem_free(_mbuf);
    return total;
}

int sys_write(int fd, void *buffer, size_t nbytes)
{
    if (fd < 0 || !nbytes || !buffer)
        return -EINVAL;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        errprint("[FS]: %s: fd %d err! handle=%d flags=%x\n", __func__, 
            fd, ffd->handle, ffd->flags);
        return -EINVAL;
    }
    if (!ffd->fsal->write)
        return -ENOSYS;
    // dbgprint("[fs] sys_write fd %d, bytes %d\n", fd, nbytes);
    if (nbytes > FSIF_RW_BUF_SIZE) {
        return write_large(ffd, buffer, nbytes);
    } else {
        char _buf[FSIF_RW_BUF_SIZE] = {0};
        if (mem_copy_from_user(_buf, buffer, nbytes) < 0) {
            errprintln("[fs] sys_write: copy buf %p from user failed!", buffer);
            return -EINVAL;
        }
        //keprintln("buf: %s", _buf);
        return ffd->fsal->write(ffd->handle, _buf, nbytes);
    }
}

int sys_ioctl(int fd, int cmd, void *arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->ioctl)
        return -ENOSYS;
    return ffd->fsal->ioctl(ffd->handle, cmd, arg);
}

int sys_fastio(int fd, int cmd, void *arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fastio)
        return -ENOSYS;
    return ffd->fsal->fastio(ffd->handle, cmd, arg);
}

int sys_fcntl(int fd, int cmd, long arg)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    int newfd = -1;
    switch (cmd) {
    case F_DUPFD: /* 复制一个基于arg（basefd）的最小的fd */
    {
        if (!ffd->fsal->incref)
            return -ENOSYS;
        if (ffd->fsal->incref(ffd->handle) < 0)
            return -EINVAL;
        newfd = local_fd_install_based(ffd->handle, ffd->flags & FILE_FD_TYPE_MASK, arg);
        return newfd;
    }
    case F_GETFD:
        return (ffd->flags & FILE_FD_CLOEXEC) ? FD_CLOEXEC : FD_NCLOEXEC;
    case F_SETFD:
        if (arg & FD_CLOEXEC)
            ffd->flags |= FILE_FD_CLOEXEC;
        else
            ffd->flags &= ~FILE_FD_CLOEXEC;
        break;
    case F_GETFL:
        /* TODO: return file flags */
        
        break;
    case F_SETFL:
        /* TODO: set file flags */
        /* set flags real */
        if (!ffd->fsal->fcntl)
            return -ENOSYS;
        return ffd->fsal->fcntl(ffd->handle, cmd, arg);
    default:
        break;
    }
    return -1;
}

int sys_lseek(int fd, off_t offset, int whence)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->lseek)
        return -ENOSYS;
    return ffd->fsal->lseek(ffd->handle, offset, whence);
}

#if defined(CONFIG_NEWSYSCALL)
void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return (void *)-1;
    if (!ffd->fsal->mmap)
        return (void *)-1;
    return ffd->fsal->mmap(ffd->handle, addr, length, prot, flags, offset);
}
#else
void *__sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return (void *)-1;
    if (!ffd->fsal->mmap)
        return (void *)-1;
    return ffd->fsal->mmap(ffd->handle, addr, length, prot, flags, offset);
}

void *sys_mmap(mmap_args_t *args)
{
    mmap_args_t _args;
    if (mem_copy_from_user(&_args, args, sizeof(mmap_args_t)) < 0)
        return (void *)-1;
    return __sys_mmap(_args.addr, _args.length, _args.prot, _args.flags,
            _args.fd, _args.offset);
}
#endif

int sys_access(const char *path, int mode)
{
    if (!path)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    if (account_selfcheck_permission((char *)_path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_path[MAX_PATH] = {0};
    build_path(_path, abs_path);
    return fsif.access(abs_path, mode);
}

int __sys_unlink(const char *path, int flags)
{
    if (!path)
        return -EINVAL;
    if (account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    dbgprintln("unlink: %s", abs_path);
    return fsif.unlink((char *) abs_path);
}

int sys_unlink(const char *path, int flags)
{
    if (!path)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    return __sys_unlink(_path, flags);
}


/**
 * 删除一个文件
 * 如果pathname是绝对路径，那么dirfd被忽略，功能和unlink一样。
 * 如果pathname是想对路径，那么就看dirfd是什么，如果是AT_FDCWD，
 * 那么就会把pathname和cwd进行合并，得出最终的路径。不然，就会把
 * dirfd对应的路径和pathname进行合并，得出最后的路径。
 */
int sys_unlinkat(int dirfd, const char *pathname, int flags)
{
    if (!pathname)
        return -EINVAL;
    
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)pathname, MAX_PATH) < 0) {
        return -EINVAL;
    }
    // dbgprintln("[fs] sys_ulinkat: dirfd %d, pathname %s, flags %x", dirfd, _path, flags);
    if (_path[0] == '/' && _path[1] == '\0') {
        dbgprintln("[fs] sys_ulinkat: can't make root dir '/' directly");
        return -EINVAL;
    }
    if (_path[0] == '/' && _path[1] != '\0') {
        return __sys_unlink(_path, flags);
    }
    if (dirfd == AT_FDCWD) {    /* 在进程的cwd目录后面 */
        return __sys_unlink(_path, flags);
    }
    char *dirpath = fsif_dirfd_path(dirfd);
    if (dirpath == NULL) {
        dbgprintln("[fs] sys_ulinkat: dirfd %d not a director", dirfd);
        return -ENFILE; /* fd没有对应目录 */
    }
    char oldcwd[MAX_PATH] = {0};    /* save old cwd as dirpath */
    if (kfile_getcwd(oldcwd, MAX_PATH) < 0) {
        dbgprintln("[fs] sys_ulinkat: get cwd error");
        return -EPERM;
    }
    task_t *cur = task_current;
    task_set_cwd(cur, dirpath); /* set cwd as dirpath */
    int newfd = __sys_unlink(_path, flags); /* do open file */
    if (newfd < 0) {
        dbgprintln("[fs] sys_ulinkat: open file %s error", _path);
    }
    task_set_cwd(cur, oldcwd); /* restore cwd as old cwd */
    return newfd;
}

int sys_ftruncate(int fd, off_t offset)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->ftruncate)
        return -ENOSYS;
    return ffd->fsal->ftruncate(ffd->handle, offset);
}

int sys_fsync(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fsync)
        return -ENOSYS;
    return ffd->fsal->fsync(ffd->handle);
}

long sys_tell(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->ftell)
        return -ENOSYS;
    return ffd->fsal->ftell(ffd->handle);
}

long sys_fsize(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fsize)
        return -ENOSYS;
    return ffd->fsal->fsize(ffd->handle);
}

int sys_stat(const char *path, struct stat *buf)
{
    if (!path || !buf)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    if (account_selfcheck_permission((char *)_path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_path[MAX_PATH] = {0};
    build_path(_path, abs_path);
    struct stat _buf;
    int err = fsif.state((char *) abs_path, &_buf);
    if (err >= 0) {
        if (mem_copy_to_user(buf, &_buf, sizeof(struct stat)) < 0)
            return -EINVAL;
    }
    return err;
}

int sys_fstat(int fd, struct kstat *buf)
{ 
    if (!buf)
        return -EINVAL;
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fstat)
        return -ENOSYS;
    struct kstat _buf;
    int err = ffd->fsal->fstat(ffd->handle, &_buf);
    if (err >= 0) {
        if (mem_copy_to_user(buf, &_buf, sizeof(struct kstat)) < 0)
            return -EINVAL;
    }
    return err;
}

int sys_chmod(const char *path, mode_t mode)
{
    if (!path)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    if (account_selfcheck_permission((char *)_path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_path[MAX_PATH] = {0};
    build_path(_path, abs_path);
    return fsif.chmod((char *) abs_path, mode);
}

int sys_fchmod(int fd, mode_t mode)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (!ffd->fsal->fchmod)
        return -ENOSYS;
    return ffd->fsal->fchmod(ffd->handle, mode);
}

int __sys_mkdir(const char *path, mode_t mode)
{
    if (!path)
        return -EINVAL;
    if (account_selfcheck_permission((char *)path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_path[MAX_PATH] = {0};
    build_path(path, abs_path);
    return fsif.mkdir((char *) abs_path, mode);
}

int sys_mkdir(const char *path, mode_t mode)
{
    if (!path)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    return __sys_mkdir(_path, mode);
}

/**
 * 创建一个目录
 * 如果pathname是绝对路径，那么dirfd被忽略，功能和mkdir一样。
 * 如果pathname是想对路径，那么就看dirfd是什么，如果是AT_FDCWD，
 * 那么就会把pathname和cwd进行合并，得出最终的路径。不然，就会把
 * dirfd对应的路径和pathname进行合并，得出最后的路径。
 */
int sys_mkdirat(int dirfd, const char *pathname, mode_t mode)
{
    if (!pathname)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)pathname, MAX_PATH) < 0) {
        return -EINVAL;
    }
    //dbgprintln("[fs] sys_mkdirat: dirfd %d, pathname %s, mode %x", dirfd, _path, mode);
    if (_path[0] == '/' && _path[1] == '\0') {
        dbgprintln("[fs] sys_mkdirat: can't make root dir '/' directly");
        return -EINVAL;
    }
    if (_path[0] == '/' && _path[1] != '\0') {
        return __sys_mkdir(_path, mode);
    }
    if (dirfd == AT_FDCWD) {    /* 在进程的cwd目录后面 */
        return __sys_mkdir(_path, mode);
    }
    char *dirpath = fsif_dirfd_path(dirfd);
    if (dirpath == NULL) {
        dbgprintln("[fs] sys_mkdirat: dirfd %d not a director", dirfd);
        return -ENFILE; /* fd没有对应目录 */
    }
    char oldcwd[MAX_PATH] = {0};    /* save old cwd as dirpath */
    if (kfile_getcwd(oldcwd, MAX_PATH) < 0) {
        dbgprintln("[fs] sys_mkdirat: get cwd error");
        return -EPERM;
    }
    task_t *cur = task_current;
    task_set_cwd(cur, dirpath); /* set cwd as dirpath */
    int newfd = __sys_mkdir(_path, mode); /* do open file */
    if (newfd < 0) {
        dbgprintln("[fs] sys_mkdirat: open file %s error", _path);
    }
    task_set_cwd(cur, oldcwd); /* restore cwd as old cwd */
    return newfd;
}

int sys_rmdir(const char *path)
{
    if (!path)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *)path, MAX_PATH) < 0)
        return -EINVAL;
    if (account_selfcheck_permission((char *)_path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_path[MAX_PATH] = {0};
    build_path(_path, abs_path);
    return fsif.rmdir((char *) abs_path);
}

int sys_rename(const char *source, const char *target)
{
    if (!source || !target)
        return -EINVAL;
    char _source[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_source, (void *)source, MAX_PATH) < 0)
        return -EINVAL;
    char _target[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_target, (void *) target, MAX_PATH) < 0)
        return -EINVAL;
    if (account_selfcheck_permission((char *)_source, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    if (account_selfcheck_permission((char *)_target, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_source[MAX_PATH] = {0};
    build_path(_source, abs_source);
    char abs_target[MAX_PATH] = {0};
    build_path(_target, abs_target);
    return fsif.rename((char *) abs_source, (char *) abs_target);
}

int sys_chdir(const char *path)
{
    if (!path)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    task_t *cur = task_current;
    if (!cur->fileman)
        return -EINVAL;
    char abs_source[MAX_PATH] = {0};
    build_path(_path, abs_source);
    // dbgprintln("[fs] %s: dir _path=%s abs=%s", __func__, _path, abs_source);
    dir_t dir = fsif.opendir(abs_source);
    if (dir < 0) {
        return -ENOFILE;
    }
    sys_closedir(dir);
    return task_set_cwd(cur, abs_source);
}
#if defined(CONFIG_NEWSYSCALL)
char *sys_getcwd(char *buf, int bufsz)
{
    if (!buf)
        return NULL;
    task_t *cur = task_current;
    if (!cur->fileman)
        return NULL;
    if (mem_copy_to_user(buf, cur->fileman->cwd, min((bufsz == 0) ? MAX_PATH : bufsz,
        MAX_PATH)) < 0)
        return NULL;
    return buf;
}
#else
int sys_getcwd(char *buf, int bufsz)
{
    if (!buf)
        return -EINVAL;
    task_t *cur = task_current;
    if (!cur->fileman)
        return -EINVAL;
    if (mem_copy_to_user(buf, cur->fileman->cwd, min((bufsz == 0) ? MAX_PATH : bufsz,
        MAX_PATH)) < 0)
        return -EINVAL;
    return 0;
}
#endif

dir_t sys_opendir(const char *path)
{
    if (!path)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, (void *) path, MAX_PATH) < 0)
        return -EINVAL;
    if (account_selfcheck_permission((char *)_path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_path[MAX_PATH] = {0};
    build_path(_path, abs_path);
    return fsif.opendir((char *) abs_path);
}

int sys_closedir(dir_t dir)
{
    return fsif.closedir(dir);
}

int sys_readdir(dir_t dir, struct dirent *dirent)
{
    if (!dirent)
        return -EINVAL;
    
    struct dirent _dirent;
    int err = fsif.readdir(dir, &_dirent);
    if (err >= 0) {
        if (mem_copy_to_user(dirent, &_dirent, sizeof(struct dirent)) < 0)
            return -EINVAL;
    }
    return err;
}

int sys_rewinddir(dir_t dir)
{
    return fsif.rewinddir(dir);
}

int fsif_incref(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -1;
    if (!ffd->fsal->incref)
        return -ENOSYS;
    return ffd->fsal->incref(ffd->handle);
}

int fsif_decref(int fd)
{
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd))
        return -1;
    if (!ffd->fsal->decref)
        return -ENOSYS;
    return ffd->fsal->decref(ffd->handle);
}

/**
 * 复制一个文件描述符
 * @oldfd: 需要复制的fd
 * 
 * 复制fd，把对应的资源的引用计数+1，然后安装到一个最小的fd中。
 * 
 * 成功返回fd，失败返回-1
 */
int sys_dup(int oldfd)
{
    file_fd_t *ffd = fd_local_to_file(oldfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    int newfd = -1;
    if (!ffd->fsal->incref)
        return -ENOSYS;
    if (ffd->fsal->incref(ffd->handle) < 0)
        return -EINVAL;
    newfd = local_fd_install(ffd->handle, ffd->flags & FILE_FD_TYPE_MASK);
    return newfd;
}

/**
 * 复制一个文件描述符
 * @oldfd: 需要复制的fd
 * @newfd: 需要复制到的fd
 * 
 * 复制fd，把对应的资源的引用计数+1，然后安装到newfd中，如果newfd已经打开，则需要先关闭
 * 如果oldfd和newfd一样，则直接返回newfd
 * 
 * 成功返回newfd，失败返回-1
 */
int sys_dup2(int oldfd, int newfd)
{
    file_fd_t *ffd = fd_local_to_file(oldfd);
    if (FILE_FD_IS_BAD(ffd))
        return -EINVAL;
    if (oldfd == newfd) /* 一样则直接返回 */
        return newfd;
    /* 查看新fd，看是否已经打开，如果是，则先关闭。 */
    file_fd_t *newffd = fd_local_to_file(newfd);
    if (newffd != NULL && newffd->handle >= 0 && newffd->flags != 0) {
        if (sys_close(newfd) < 0)
            return -1;   
    }
    if (!ffd->fsal->incref)
        return -ENOSYS;
    if (ffd->fsal->incref(ffd->handle) < 0)
        return -EINVAL; 
    /* 从newfd开始查找安装位置，并把oldfd安装进去 */   
    newfd = local_fd_install_based(ffd->handle, ffd->flags & FILE_FD_TYPE_MASK, newfd);
    return newfd;
}

int sys_pipe(int fd[2])
{
    if (!fd)
        return -EINVAL;
    pipe_t *pipe = create_pipe();
    if (pipe == NULL)
        return -EINVAL;
    /* read */
    int rfd = local_fd_install(pipe->id, FILE_FD_PIPE0);
    if (rfd < 0) {
        destroy_pipe(pipe);
        return -EAGAIN;
    }
    /* write */
    int wfd = local_fd_install(pipe->id, FILE_FD_PIPE1);
    if (wfd < 0) {
        local_fd_uninstall(rfd);
        destroy_pipe(pipe);
        return -EAGAIN;
    }
    int _fd[2];
    _fd[0] = rfd;
    _fd[1] = wfd;
    if (mem_copy_to_user(fd, _fd, sizeof(int) * 2) < 0)
        return -EINVAL;
    return 0;
}

int sys_mount(
    char *source,         /* 需要挂载的资源 */
    char *target,         /* 挂载到的目标位置 */
    char *fstype,         /* 文件系统类型 */
    unsigned long mountflags    /* 挂载标志 */
) {
    if (!source || !target || !fstype)
        return -EINVAL;
    char _source[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_source, (void *)source, MAX_PATH) < 0)
        return -EINVAL;
    char _target[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_target, target, MAX_PATH) < 0)
        return -EINVAL;
    char _fstype[32] = {0};
    if (mem_copy_from_user_str(_fstype, fstype, 32) < 0)
        return -EINVAL;
    
    if (account_selfcheck_permission((char *)_source, PERMISION_ATTR_DEVICE) < 0) {
        return -EPERM;
    }
    if (account_selfcheck_permission((char *)_target, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_source[MAX_PATH] = {0};
    build_path(_source, abs_source);
    char abs_target[MAX_PATH] = {0};
    build_path(_target, abs_target);
    return fsif.mount(abs_source, abs_target, _fstype, mountflags);
}

int sys_unmount(char *path, unsigned long flags)
{
    if (!path)
        return -EINVAL;
    char _path[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_path, path, MAX_PATH) < 0)
        return -EINVAL;
    if (account_selfcheck_permission((char *)_path, PERMISION_ATTR_FILE) < 0) {
        return -EPERM;
    }
    char abs_path[MAX_PATH] = {0};
    build_path(_path, abs_path);
    return fsif.unmount(abs_path, abs_path, flags);
}

int sys_mkfs(char *source,         /* 需要创建FS的设备 */
    char *fstype,         /* 文件系统类型 */
    unsigned long flags   /* 标志 */
) {
    if (!source || !fstype)
        return -EINVAL;
    char _source[MAX_PATH] = {0};
    if (mem_copy_from_user_str(_source, source, MAX_PATH) < 0)
        return -EINVAL;
    char _fstype[32] = {0};
    if (mem_copy_from_user_str(_fstype, fstype, 32) < 0)
        return -EINVAL;
    if (account_selfcheck_permission((char *)source, PERMISION_ATTR_DEVICE) < 0) {
        return -EPERM;
    }
    char abs_source[MAX_PATH] = {0};
    build_path(_source, abs_source);
    return fsif.mkfs(abs_source, _fstype, flags);
}

int sys_probedev(const char *name, char *buf, size_t buflen)
{
    if (!name || !buf)
        return -EINVAL;
    char _name[DEVICE_NAME_LEN] = {0};
    if (mem_copy_from_user(_name, (void *) name, 32) < 0)
        return -EINVAL;
    char _buf[DEVICE_NAME_LEN] = {0};
    int err = device_probe_unused(_name, _buf, buflen);
    if (mem_copy_to_user(buf, _buf, buflen) < 0)
        return -EINVAL;
    return err;
}


static int getdents_large(file_fd_t *ffd, void *dirp, size_t nbytes)
{
    char *_mbuf = mem_alloc(FSIF_RW_CHUNK_SIZE);
    if (_mbuf == NULL) {
        return -ENOMEM;
    }
    int total = 0;
    char *p = (char *)dirp;
    size_t chunk = nbytes % FSIF_RW_CHUNK_SIZE;
    while (nbytes > 0) {
        int rd = ffd->fsal->getdents(ffd->handle, _mbuf, chunk);
        if (rd < 0) {
            errprintln("[fs] getdents_large: handle %d do read failed!", ffd->handle);
            total = -EIO;
            break;
        }
        if (rd == 0) {  /* read done, no left */
            break;
        }
        if (mem_copy_to_user(p, _mbuf, chunk) < 0) {
            errprintln("[fs] getdents_large: copy buf %p to user failed!", p);
            total = -EINVAL;
            break;
        }
        p += chunk;
        total += rd;
        nbytes -= chunk;
        chunk = FSIF_RW_CHUNK_SIZE;
    }
    mem_free(_mbuf);
    return total;
}

int sys_getdents(int fd, void *dirp, unsigned long len)
{
    if (fd < 0 || !dirp || !len) {
        errprint("[fs] sys_getdents: invalid args: dirfd %d dirp=%p len=%d\n", fd, dirp, len);
        return -EINVAL;
    }
    file_fd_t *ffd = fd_local_to_file(fd);
    if (FILE_FD_IS_BAD(ffd)) {
        errprint("[fs] sys_getdents: dirfd %d err!\n", fd);
        return -EINVAL;
    }
    /* 使用底层实现 */
    if (!ffd->fsal->getdents)
        return -ENOSYS;
    
    if (len > FSIF_RW_BUF_SIZE) {
        return getdents_large(ffd, dirp, len);
    } else {
        char _buf[FSIF_RW_BUF_SIZE] = {0};
        int rd = ffd->fsal->getdents(ffd->handle, _buf, len);
        if (rd > 0) {
            if (mem_copy_to_user(dirp, _buf, rd) < 0) {
                errprintln("[fs] sys_getdents: copy buf %p to user failed!", dirp);
                return -EINVAL;
            }
        }
        return rd;
    }

    char *dirpath = fsif_dirfd_path(fd);
    if (dirpath == NULL) {
        dbgprintln("[fs] sys_getdents: dirfd %d not a director", fd);
        return -ENFILE; /* fd没有对应目录 */
    }
    /* 读取目录内容，然后放到dirp64结构体中 */
    dbgprintln("[fs] sys_getdents: dirpath %s", dirpath);
    int dir = fsif.opendir(dirpath);
    if (dir < 0) {
        dbgprintln("[fs] sys_getdents: open dir %s error", dirpath);
        return dir;
    }
    struct dirent dirent;
    int rdbytes = 0;    /* 读取到的字节数 */
    long n = len;

    char _buf[FSIF_RW_BUF_SIZE] = {0};
    unsigned char *buf = (unsigned char *)_buf;
    struct linux_dirent64 *dest;
    while (n > 0) {
        int err = fsif.readdir(dir, &dirent);
        if (err < 0) {
            break;
        }
        dest = (struct linux_dirent64 *)buf;
        /* copy data */
        strcpy(dest->d_name, dirent.d_name);
        if (dirent.d_attr & DE_DIR) {
            dest->d_type = DT_DIR;
        } else if (dirent.d_attr & DE_CHAR) {
            dest->d_type = DT_CHR;
        } else if (dirent.d_attr & DE_BLOCK) {
            dest->d_type = DT_BLK;
        } else {
            dest->d_type = DT_REG;
        }
        dest->d_reclen = sizeof(struct linux_dirent64);
        dest->d_reclen += strlen(dest->d_name) + 1;
        /* 4 bytes align */
        dest->d_reclen = (dest->d_reclen + 4) & (~(4 - 1));

        dest->d_ino = 0;

        buf += dest->d_reclen;
        rdbytes += dest->d_reclen;
        
        dest->d_off = rdbytes;
    }
    fsif.closedir(dir);
    if (rdbytes > 0) {
        if (mem_copy_to_user(dirp, _buf, rdbytes) < 0)
            return -ENOBUFS;
    }

    return rdbytes;
}