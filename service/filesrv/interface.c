#include "filesrv.h"
#include <ff.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <types.h>

#define SRVBUF_256      256
#define SRVBUF_4K       4096
#define SRVBUF_128K     131072

unsigned char *srvbuf256;
unsigned char *srvbuf4k;
unsigned char *srvbuf128k;

static int do_open(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    filesrv_file_t *file = filesrv_alloc_file();
    if (file == NULL) {
        printf("%s: %s: alloc file failed!\n", SRV_NAME, __func__);    
        return -1;
    }
    FRESULT fr;
    /* 现在已经获取了完整的数据 */
    BYTE mode = 0;  /* 文件打开模式 */
    int flags = GETSRV_DATA(arg, 2, int);
    
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

    fr = f_open(&file->fil, GETSRV_DATA(arg, 1, const char *), mode);
    if (fr != FR_OK) {
        // arg->retval = 0;  /* null */
        SETSRV_RETVAL(arg, 0);
        return -1;
    } else {
        SETSRV_RETVAL(arg, &file->fil);
        //arg->retval = (unsigned long) &file->fil;
    }
    return 0;
}

static int do_close(srvarg_t *arg)
{
    FRESULT fr;
    fr = f_close(GETSRV_DATA(arg, 1, FIL *));
    if (fr != FR_OK) {
        arg->retval = -1;  /* null */
        return -1;
    } 
    arg->retval = 0;
    return filesrv_free_file2(GETSRV_DATA(arg, 1, FIL *));
}


static int do_read(srvarg_t *arg)
{
    FRESULT fr;
    UINT br;
    FIL *fp = GETSRV_DATA(arg, 1, FIL *);
    int len = MIN(GETSRV_SIZE(arg, 2), SRVBUF_128K);
    fr = f_read(fp, srvbuf128k, len, &br);
    if (fr != FR_OK) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_DATA(arg, 2, srvbuf128k);
    SETSRV_SIZE(arg, 2, br);
    SETSRV_RETVAL(arg, br);
    return 0;
}

static int do_write(srvarg_t *arg)
{
    FRESULT fr;
    UINT bw;
    FIL *fp = GETSRV_DATA(arg, 1, FIL *);
    size_t nbytes = GETSRV_SIZE(arg, 2);
    int len = MIN(nbytes, SRVBUF_128K);
    
    /* 需要检测参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 2, srvbuf128k);
        SETSRV_SIZE(arg, 2, len);
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }

    fr = f_write(fp, srvbuf128k, len, &bw);
    if (fr != FR_OK) {
        SETSRV_RETVAL(arg, -1);
        return -1;
    }
    SETSRV_RETVAL(arg, bw);
    return 0;
}


static int do_lseek(srvarg_t *arg)
{
    FRESULT fr;
    FIL *fp = GETSRV_DATA(arg, 1, FIL *);
    off_t offset = GETSRV_DATA(arg, 2, off_t);
    int whence = GETSRV_DATA(arg, 3, int);
    off_t new_off = 0;
    switch (whence)
    {
    case SEEK_SET:
        new_off = offset;
        break;
    case SEEK_CUR:
        new_off = f_tell(fp) + offset;
        break;
    case SEEK_END:
        new_off = f_size(fp) + offset;
        break;    
    default:
        break;
    }
    fr = f_lseek(fp, new_off);
    if (fr != FR_OK) {
        SETSRV_RETVAL(arg, 0);
        return -1;
    }
    SETSRV_RETVAL(arg, new_off);
    return 0;
}

static int do_assert(srvarg_t *arg)
{
    //printf("%s: do_assert\n", SRV_NAME);

    /* 需要读取参数 */
    if (!srvcall_inbuffer(arg)) {
        SETSRV_DATA(arg, 1, srvbuf256);
        SETSRV_SIZE(arg, 1, MIN(GETSRV_SIZE(arg, 1), SRVBUF_256));
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    const char *filenpath = GETSRV_DATA(arg, 1, const char *);
    int mode = GETSRV_DATA(arg, 2, int);
    if (!mode) {
        FRESULT fr;
        FIL fil;
        fr = f_open(&fil, filenpath, FA_OPEN_EXISTING | FA_READ);
        if (fr != FR_OK) { /* file not exist */
            //printf("%s: file not exist.\n", SRV_NAME);
        
            SETSRV_RETVAL(arg, -1);
            return -1;
        }
        //printf("%s: file exist.\n", SRV_NAME);
        /* file exist */
        f_close(&fil);
        SETSRV_RETVAL(arg, 0);   
        return 0; 
    }
    return -1;
}

/* 调用表 */
filesrv_func_t filesrv_call_table[] = {
    do_open,
    do_close,
    do_read,
    do_write,
    do_lseek,
    do_assert,
};

int filesrv_init_interface()
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