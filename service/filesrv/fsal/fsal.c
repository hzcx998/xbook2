#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <filesrv.h>
#include <unistd.h>
#include <spin.h>

/* 文件表指针 */
fsal_file_t *fsal_file_table;

/**
 * init_fsal_file_table - 初始化文件表
 * 
 * 分配文件表内存，并清空
 */
int init_fsal_file_table()
{
    fsal_file_table = malloc(FSAL_FILE_OPEN_NR * sizeof(fsal_file_t));
    if (fsal_file_table == NULL) 
        return -1;
    memset(fsal_file_table, 0, FSAL_FILE_OPEN_NR * sizeof(fsal_file_t));
    return 0;
}

/**
 * fsal_file_alloc - 从表中分配一个文件
 * 
 */
fsal_file_t *fsal_file_alloc()
{
    int i;
    for (i = 0; i < FSAL_FILE_OPEN_NR; i++) {
        if (!fsal_file_table[i].flags) {
            /* 清空文件表的内容 */
            memset(&fsal_file_table[i], 0, sizeof(fsal_file_t));
            
            /* 记录使用标志 */
            fsal_file_table[i].flags = FSAL_FILE_USED;

            return &fsal_file_table[i];
        }
    }
    return NULL;
}

/**
 * fsal_file_free - 释放一个文件
 * 
 */
int fsal_file_free(fsal_file_t *file)
{
    if (!file->flags)
        return -1;
    file->flags = 0;
    return 0;
}

int init_fsal()
{
    if (init_fsal_file_table() < 0) {
        return -1;
    }
    
    if (init_fsal_path_table() < 0) {
        return -1;
    }

#if 0    
    /* 尝试创建文件系统 */
    if (fatfs_fsal.mkfs("0:", 0) < 0) {
        printf("[%s] %s: make fatfs failed!\n", SRV_NAME, __func__);
        return -1;
    }
#endif    
    /* 把fatfs的0:路径挂载到c驱动器下面 */
    if (fatfs_fsal.mount("0:", 'c',0) < 0) {
        printf("[%s] %s: mount fatfs failed!\n", SRV_NAME, __func__);
        return -1;
    }
    
    fsal_path_print();
#if 0
    if (fatfs_fsal.unmount("0:", 0) < 0) {
        printf("[%s] %s: unmount fatfs failed!\n", SRV_NAME, __func__);
        return -1;
    }
    
    fsal_path_print();
    
    spin();
#endif

    int retval = fsif.open("c:/bin", O_CREAT | O_RDWR);
    if (retval < 0) {
        printf("[%s] %s: open file failed!\n", SRV_NAME, __func__);
        return -1;
    }
    printf("[%s] %s: open file %d ok.\n", SRV_NAME, __func__, retval);
#if 0 
    retval = fatfs_fsal.close(retval);
    if (retval < 0) {
        printf("[%s] %s: close file failed!\n", SRV_NAME, __func__);
        return -1;
    }
#endif
    char *str = "hello, fatfs!\n";

    int wrbytes = fsif.write(retval, str, strlen(str));
    if (wrbytes < 0)
        return -1;

    printf("[%s] %s: write bytes %d ok.\n", SRV_NAME, __func__, wrbytes);

    int seekval = fsif.lseek(retval, 0, SEEK_SET);
    printf("[%s] %s: seek pos %d ok.\n", SRV_NAME, __func__, seekval);

    char buf[16] = {0};
    int rdbytes = fsif.read(retval, buf, 16);
    if (rdbytes < 0)
        return -1;
    printf("[%s] %s: read bytes %d ok. str:%s\n", SRV_NAME, __func__, rdbytes, buf);
    
    fsif.close(retval);


    return 0;
}
