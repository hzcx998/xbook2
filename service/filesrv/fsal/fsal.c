#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <filesrv.h>

int init_fsal()
{
    /* 尝试挂载文件系统 */
    if (fatfs_fsal.mount("0:", 0) < 0) {
        printf("[%s] %s: mount fatfs failed!\n", SRV_NAME, __func__);
        return -1;
    }
    return 0;
}
