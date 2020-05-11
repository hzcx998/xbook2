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
#define SRVBUF_128K       131072


unsigned char *srvbuf256;
unsigned char *srvbuf4k;
unsigned char *srvbuf128k;

static int do_open(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_check(arg)) {
        arg->data[1] = (unsigned long) srvbuf256;
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }
    filesrv_file_t *file = filesrv_alloc_file();
    if (file == NULL)
        return -1;

    FRESULT fr;
    /* 现在已经获取了完整的数据 */
    printf("%s: data1 %x\n", SRV_NAME, arg->data[1]);
    printf("path: %s\n", (char *) arg->data[1]);
    printf("%s: data2 %x\n", SRV_NAME, arg->data[2]);
    BYTE mode = 0;  /* 文件打开模式 */
    int flags = arg->data[2];
    
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

    fr = f_open(&file->fil, (char *) arg->data[1], mode);
    if (fr != FR_OK) {
        arg->retval = 0;  /* null */
        return -1;
    } else {
        arg->retval = (unsigned long) &file->fil;
    }
    return 0;
}

static int do_close(srvarg_t *arg)
{
    FRESULT fr;
    printf("%s: srvcall [close]\n", SRV_NAME);
    printf("%s: data1 %x\n", SRV_NAME, arg->data[1]);
    fr = f_close((FIL *)arg->data[1]);
    if (fr != FR_OK) {
        arg->retval = -1;  /* null */
        return -1;
    } 
    arg->retval = 0;
    return filesrv_free_file2((FIL *)arg->data[1]);
}


static int do_read(srvarg_t *arg)
{
    FRESULT fr;
    UINT br;
    FIL *fp = (FIL *) arg->data[1];
    printf("%s: srvcall [read]\n", SRV_NAME);
    printf("%s: data1 %x data2 %x\n", SRV_NAME, fp, arg->data[2]);
    int len = MIN(arg->size[2], SRVBUF_128K);
    printf("%s: len %x\n", SRV_NAME, len);
    
    fr = f_read(fp, srvbuf128k, len, &br);
    if (fr != FR_OK) {
        printf("%s: f_read failed!\n", SRV_NAME);
        arg->retval = -1;  /* null */
        return -1;
    }
    printf("%s: read len %x\n", SRV_NAME, br);
    
    arg->data[2] = (unsigned long) srvbuf128k;
    arg->size[2] = br;
    
    arg->retval = br;
    return 0;
}

static int do_write(srvarg_t *arg)
{
    FRESULT fr;
    UINT bw;
    FIL *fp = (FIL *) arg->data[1];
    void *buffer = (void *) arg->data[2];
    size_t nbytes = (size_t) arg->size[2];

    printf("%s: srvcall [write]\n", SRV_NAME);
    printf("%s: fp=%x buf=%x nbytes=%d\n", SRV_NAME, fp, buffer, nbytes);

    int len = MIN(nbytes, SRVBUF_128K);
    
    /* 需要检测参数 */
    if (!srvcall_check(arg)) {
        arg->data[2] = (unsigned long) srvbuf128k;
        arg->size[2] = len;  /* 获取数据长度 */
        
        if (srvcall_fetch(SRV_FS, arg))
            return -1;
    }

    fr = f_write(fp, srvbuf128k, len, &bw);
    if (fr != FR_OK) {
        arg->retval = -1;  /* null */
        return -1;
    }
    arg->retval = bw;
    return 0;
}


static int do_lseek(srvarg_t *arg)
{
    FRESULT fr;
    printf("%s: srvcall [write]\n", SRV_NAME);
    FIL *fp = (FIL *) arg->data[1];
    off_t offset = arg->data[2];
    int whence = arg->data[3];
    printf("%s: fp=%x off=%d whence=%d\n", SRV_NAME, fp, offset, whence);
    
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
        arg->retval = 0;
        return -1;
    }
    arg->retval = new_off;
    return 0;
}


/* 调用表 */
filesrv_func_t filesrv_call_table[] = {
    do_open,
    do_close,
    do_read,
    do_write,
    do_lseek
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