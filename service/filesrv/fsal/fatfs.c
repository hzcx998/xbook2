#include <fsal/fsal.h>
#include <ff.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <filesrv.h>


typedef struct {
    FATFS *fsobj;        /* 挂载对象指针 */
} fatfs_extention_t;

fatfs_extention_t fatfs_extention;

/**
 * 挂载文件系统
*/
static int __mount(void *path, int flags)
{
    FATFS *fsobj;           /* Filesystem object */
    fsobj = malloc(sizeof(FATFS));
    if (fsobj == NULL) 
        return -1;
    memset(fsobj, 0, sizeof(FATFS));
    FRESULT res;
    const TCHAR *p = (const TCHAR *) path;
    res = f_mount(fsobj, p, 1);
    if (res != FR_OK) {
        printf("%s: %s: mount on path %s failed, code %d.\n", SRV_NAME, __func__, p, res);
        free(fsobj);
        return -1;
    }
    fatfs_extention.fsobj = fsobj;
    return 0;
}

/* fatfs的抽象层 */
fsal_t fatfs_fsal = {
    .mount = __mount,
    .extention = (void *)&fatfs_extention,
};
