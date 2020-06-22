#include <ff.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
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
    if (readbytes < 0) {
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
    if (writebytes < 0) {
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

static int __assert(srvarg_t *arg)
{
    //printf("%s: __assert\n", SRV_NAME);

    /* 需要读取参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    void *filenpath = GETSRV_DATA(arg, 1, void *);
    int mode = GETSRV_DATA(arg, 2, int);
    if (!mode) {
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
    return -1;
}

/* 调用表 */
srvcall_func_t filesrv_call_table[] = {
    __open,
    __close,
    __read,
    __write,
    __lseek,
    __assert,
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