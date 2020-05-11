#include "filesrv.h"
#include <ff.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

char argbuf[32];
static int do_open(srvarg_t *arg)
{
    /* 需要检测参数 */
    if (!srvcall_check(arg)) {
        arg->data[1] = (unsigned long) &argbuf[0];
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
    
    fr = f_open(&file->fil, (char *) arg->data[1], FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
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

/* 调用表 */
filesrv_func_t filesrv_call_table[] = {
    do_open,
    do_close
};
