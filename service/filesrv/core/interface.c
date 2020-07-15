#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <types.h>
#include <fsal/fsal.h>
#include <core/if.h>
#include <core/filesrv.h>

#define SRVBUF_256      256
#define SRVBUF_4K       4096
#define SRVBUF_128K     131072

unsigned char *srvbuf256;
unsigned char *srvbuf4k;
unsigned char *srvbuf128k;

static int __open(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    int flags = GETSRV_DATA(arg, 2, int);
    void *path = GETSRV_DATA(arg, 1, void *);
    //srvprint("open path %s.\n", path);
    int fi = fsif.open(path, flags);
    if (fi < 0) {
        printf("[%s] open path %s failed!\n", SRV_NAME, path);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }

    SETSRV_RETVAL(arg, fi);
    return 0;
}

static int __close(srvarg_t *arg)
{
    if (fsif.close(GETSRV_DATA(arg, 1, int)) < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, 0);
    return 0;
}

static int __read(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    int len = MIN(GETSRV_SIZE(arg, 2), SRVBUF_128K);

    int readbytes = fsif.read(fi, srvbuf128k, len);
    if (readbytes <= 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }

    SETSRV_DATA(arg, 2, srvbuf128k);
    SETSRV_SIZE(arg, 2, readbytes);
    SETSRV_RETVAL(arg, readbytes);
    return 0;
}

static int __write(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    size_t nbytes = GETSRV_SIZE(arg, 2);
    int len = MIN(nbytes, SRVBUF_128K);
    
    /* 读入数据 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 2, srvbuf128k);
        SETSRV_SIZE(arg, 2, len);
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }

    int writebytes = fsif.write(fi, srvbuf128k, len);
    if (writebytes <= 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, writebytes);
    return 0;
}

static int __lseek(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    off_t offset = GETSRV_DATA(arg, 2, off_t);
    int whence = GETSRV_DATA(arg, 3, int);
    
    off_t pos = fsif.lseek(fi, offset, whence);
    if (pos < 0) {
        SETSRV_RETVAL(arg, 0);
        return -1;
    }
    SETSRV_RETVAL(arg, pos);
    return 0;
}

static int __access(srvarg_t *arg)
{
    /* 需要读取参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    void *filenpath = GETSRV_DATA(arg, 1, void *);
    //srvprint("access %s\n", filenpath);
    int mode = GETSRV_DATA(arg, 2, int);
    if (mode == F_OK) {
        int fi = fsif.open(filenpath, O_RDONLY);
        if (fi < 0) {
            SETSRV_RETVAL(arg, -1);
            return -1;
        }
        /* file exist */
        fsif.close(fi);
        SETSRV_RETVAL(arg, 0);   
        return 0; 
    }
    return 0;
}

static int __opendir(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    char *path = GETSRV_DATA(arg, 1, char *);
    int di = fsif.opendir(path);
    if (di < 0) {
        printf("[%s] open dir %s failed!\n", SRV_NAME, path);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, di);
    return 0;
}


static int __closedir(srvarg_t *arg)
{
    int didx = GETSRV_DATA(arg, 1, int);
    int retval = fsif.closedir(didx);
    if (retval < 0) {
        printf("[%s] close dir %d failed!\n", SRV_NAME, didx);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __readdir(srvarg_t *arg)
{
    int didx = GETSRV_DATA(arg, 1, int);
    int len = MIN(GETSRV_SIZE(arg, 2), SRVBUF_4K);

    int readbytes = fsif.readdir(didx, srvbuf4k);
    if (readbytes < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_DATA(arg, 2, srvbuf4k);
    SETSRV_SIZE(arg, 2, len);

    SETSRV_RETVAL(arg, readbytes);
    return 0;
}

static int __rewinddir(srvarg_t *arg)
{
    int didx = GETSRV_DATA(arg, 1, int);
    int retval = fsif.rewinddir(didx);
    if (retval < 0) {
        printf("[%s] rewinddir failed!\n", SRV_NAME);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __mkdir(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    char *path = GETSRV_DATA(arg, 1, char *);
    mode_t mode = GETSRV_DATA(arg, 2, mode_t);
    int retval = fsif.mkdir(path, mode);
    if (retval < 0) {
        printf("[%s] make dir %s failed!\n", SRV_NAME, path);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __unlink(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg)) {
            return -1;
        }
            
    }
    char *path = GETSRV_DATA(arg, 1, char *);
    int retval = fsif.unlink(path);
    if (retval < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __rmdir(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    char *path = GETSRV_DATA(arg, 1, char *);
    int retval = fsif.rmdir(path);
    if (retval < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __rename(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        SETSRV_DATA(arg, 2, srvbuf4k);
        SETSRV_SIZE(arg, 2, MIN(GETSRV_SIZE(arg, 2), SRVBUF_4K));
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    char *source = GETSRV_DATA(arg, 1, char *);
    char *target = GETSRV_DATA(arg, 2, char *);
    
    int retval = fsif.rename(source, target);
    if (retval < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __ftruncate(srvarg_t *arg)
{
    int fidx = GETSRV_DATA(arg, 1, int);
    off_t off = GETSRV_DATA(arg, 2, off_t);
    
    int retval = fsif.ftruncate(fidx, off);
    if (retval < 0) {
        printf("[%s] truncate failed!\n", SRV_NAME);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __fsync(srvarg_t *arg)
{
    int fidx = GETSRV_DATA(arg, 1, int);
    int retval = fsif.fsync(fidx);
    if (retval < 0) {
        printf("[%s] sync failed!\n", SRV_NAME);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __state(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg)) {
            return -1;
        }
    }
    char *path = GETSRV_DATA(arg, 1, char *);
    int len = MIN(GETSRV_SIZE(arg, 2), SRVBUF_4K);

    int retval = fsif.state(path, srvbuf4k);
    if (retval < 0) {
        printf("[%s] sync failed!\n", SRV_NAME);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    /* 传输数据给用户 */
    SETSRV_DATA(arg, 2, srvbuf4k);
    SETSRV_SIZE(arg, 2, len);

    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __chmod(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg)) {
            return -1;
        }
    }
    char *path = GETSRV_DATA(arg, 1, char *);
    mode_t mode = GETSRV_DATA(arg, 2, mode_t);

    int retval = fsif.chmod(path, mode);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __fchmod(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    mode_t mode = GETSRV_DATA(arg, 2, mode_t);

    int retval = fsif.fchmod(fi, mode);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __utime(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg)) {
            return -1;
        }
    }
    char *path = GETSRV_DATA(arg, 1, char *);
    time_t actime = GETSRV_DATA(arg, 2, time_t);
    time_t modtime = GETSRV_DATA(arg, 3, time_t);
    int retval = fsif.utime(path, actime, modtime);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __feof(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    int retval = fsif.feof(fi);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __ferror(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    int retval = fsif.ferror(fi);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __ftell(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    off_t retval = fsif.ftell(fi);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __fsize(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    size_t retval = fsif.fsize(fi);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __rewind(srvarg_t *arg)
{
    int fi = GETSRV_DATA(arg, 1, int);
    int retval = fsif.rewind(fi);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __mount(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        SETSRV_DATA(arg, 2, srvbuf4k);
        SETSRV_SIZE(arg, 2, MIN(GETSRV_SIZE(arg, 2), SRVBUF_4K));
        SETSRV_DATA(arg, 3, srvbuf128k);
        SETSRV_SIZE(arg, 3, MIN(GETSRV_SIZE(arg, 3), SRVBUF_128K));
        if (srvcall_fetch(SRV_FS, arg)) {
            return -1;
        }
    }
    char *source = GETSRV_DATA(arg, 1, char *);
    char *target = GETSRV_DATA(arg, 2, char *);
    char *fstype = GETSRV_DATA(arg, 3, char *);
    unsigned long flags = GETSRV_DATA(arg, 4, unsigned long);
    int retval = fsif.mount(source, target, fstype, flags);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __unmount(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg)) {
            return -1;
        }
    }
    char *source = GETSRV_DATA(arg, 1, char *);
    unsigned long flags = GETSRV_DATA(arg, 2, unsigned long);
    int retval = fsif.unmount(source, flags);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __mkfs(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        SETSRV_DATA(arg, 2, srvbuf4k);
        SETSRV_SIZE(arg, 2, MIN(GETSRV_SIZE(arg, 2), SRVBUF_4K));
        if (srvcall_fetch(SRV_FS, arg)) {
            return -1;
        }
    }
    char *source = GETSRV_DATA(arg, 1, char *);
    char *fstype = GETSRV_DATA(arg, 2, char *);
    unsigned long flags = GETSRV_DATA(arg, 3, unsigned long);
    int retval = fsif.mkfs(source, fstype, flags);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}


static int __chdir(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf4k);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_4K));
        if (srvcall_fetch(SRV_FS, arg)) {
            return -1;
        }
    }
    char *path = GETSRV_DATA(arg, 1, char *);
    int retval = fsif.chdir(path);
    if (retval < 0) {
        printf("[%s] %s failed!\n", SRV_NAME, __func__);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __ioctl(srvarg_t *arg)
{
    int fi              = GETSRV_DATA(arg, 1, int);
    int cmd             = GETSRV_DATA(arg, 2, int);
    unsigned long ioarg   = GETSRV_DATA(arg, 3, unsigned long);
    
    int retval = fsif.ioctl(fi, cmd, ioarg);
    if (retval < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __fcntl(srvarg_t *arg)
{
    int fi              = GETSRV_DATA(arg, 1, int);
    int cmd             = GETSRV_DATA(arg, 2, int);
    long farg   = GETSRV_DATA(arg, 3, long);
    
    int retval = fsif.fcntl(fi, cmd, farg);
    if (retval < 0) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, retval);
    return 0;
}

static int __fstat(srvarg_t *arg)
{
    int fi      = GETSRV_DATA(arg, 1, int);
    int len     = MIN(GETSRV_SIZE(arg, 2), SRVBUF_256);

    int retval = fsif.fstat(fi, srvbuf256);
    if (retval < 0) {
        printf("[%s] sync failed!\n", SRV_NAME);
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    /* 传输数据给用户 */
    SETSRV_DATA(arg, 2, srvbuf256);
    SETSRV_SIZE(arg, 2, len);

    SETSRV_RETVAL(arg, retval);
    return 0;
}


/* 调用表 */
srvcall_func_t filesrv_call_table[] = {
    __open,
    __close,
    __read,
    __write,
    __lseek,
    __access,
    __opendir,
    __closedir,
    __readdir,
    __rewinddir,
    __mkdir,
    __unlink,
    __rmdir,
    __rename,
    __ftruncate,
    __fsync,
    __state,
    __chmod,
    __fchmod,
    __utime,
    __feof,
    __ferror,
    __ftell,
    __fsize,
    __rewind,
    __mount,
    __unmount,
    __mkfs,
    __chdir,
    __ioctl,
    __fcntl,
    __fstat,
};

int init_srv_interface()
{
    srvbuf256 = malloc(SRVBUF_256);
    if (srvbuf256 == NULL) {
        return -1;
    }
    memset(srvbuf256, 0, SRVBUF_256);
    srvbuf4k = malloc(SRVBUF_4K);
    if (srvbuf4k == NULL) {
        return -1;
    }
    memset(srvbuf4k, 0, SRVBUF_4K);
    srvbuf128k = malloc(SRVBUF_128K);
    if (srvbuf128k == NULL) {
        return -1;
    }
    memset(srvbuf128k, 0, SRVBUF_128K);

    return 0;
}