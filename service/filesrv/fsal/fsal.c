#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <fsal/dir.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <core/filesrv.h>
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

int fsal_list_dir(char* path)
{
    dirent_t de;
    int i;
    int dir = fsif.opendir(path);
    if (dir >= 0) {
        while (1) {
            /* 读取目录项 */
            if (fsif.readdir(dir, &de) < 0)
                break;
            
            if (de.d_attr & DE_DIR) {   /* 是目录，就需要递归扫描 */
                printf("%s/%s\n", path, de.d_name);
                /* 构建新的路径 */
                i = strlen(path);
                sprintf(&path[i], "/%s", de.d_name);
                if (fsal_list_dir(path) < 0)
                    break;
                path[i] = 0;
            } else {    /* 直接列出文件 */
                printf("%s/%s  size=%d\n", path, de.d_name, de.d_size);
            }
        }
        fsif.closedir(dir);
    }
    return dir;
}

int init_disk_mount()
{
    /* 挂载文件系统 */
    if (fsif.mount("ide1", "c", "fat16", 0) < 0) {
        printf("[%s] %s: mount failed!\n", SRV_NAME, __func__);
        return -1;
    }

    return 0;
}

int init_fsal()
{
    if (init_fsal_file_table() < 0) {
        return -1;
    }

    if (init_fsal_dir_table() < 0) {
        return -1;
    }
    
    if (init_fsal_path_table() < 0) {
        return -1;
    }

    if (init_disk_mount() < 0) {
        return -1;
    }

    if (init_rom_file()) {
        printf("%s: create file failed, service stopped!\n", SRV_NAME);
        return -1;
    }

    return 0;
#if 0
    /* 尝试创建文件系统 */
    if (fatfs_fsal.mkfs("0:", 0) < 0) {
        printf("[%s] %s: make fatfs failed!\n", SRV_NAME, __func__);
        return -1;
    }
#endif
    
    //fsal_mount("ide0", "c", "fat32", 0);
    //fsal_unmount("c", 0);

#if 0
    /* 把fatfs的0:路径挂载到c驱动器下面 */
    if (fatfs_fsal.mount("0:", 'c',0) < 0) {
        printf("[%s] %s: mount fatfs failed!\n", SRV_NAME, __func__);
        return -1;
    }
#endif
/*
    if (fsif.mkfs("ide1", "fat16", 0) < 0) {
        printf("[%s] %s: mkfs failed!\n", SRV_NAME, __func__);
        return -1;
    }
*/
    

    // fsal_path_print();
#if 0
    if (fsif.unmount("c:", 0) < 0) {
        printf("[%s] %s: unmount failed!\n", SRV_NAME, __func__);
        return -1;
    }

    fsal_path_print();
#endif
#if 0
    if (fatfs_fsal.unmount("0:", 0) < 0) {
        printf("[%s] %s: unmount fatfs failed!\n", SRV_NAME, __func__);
        return -1;
    }
    
    fsal_path_print();
    
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

    if (fsif.fsync(retval) < 0)
        printf("[%s] %s: sync file %d failed!\n", SRV_NAME, __func__);

    printf("[%s] %s: write bytes %d ok.\n", SRV_NAME, __func__, wrbytes);

    int seekval = fsif.lseek(retval, 0, SEEK_SET);
    printf("[%s] %s: seek pos %d ok.\n", SRV_NAME, __func__, seekval);

    char buf[16] = {0};
    int rdbytes = fsif.read(retval, buf, 16);
    if (rdbytes < 0)
        return -1;
    printf("[%s] %s: read bytes %d ok. str:%s\n", SRV_NAME, __func__, rdbytes, buf);
    /*
    if (fsif.truncate(retval, 100) < 0)
        printf("truncate failed!\n");
    */
    
    fsif.close(retval);

    char path[MAX_PATH] = {0};
    strcpy(path, "c:");
    fsal_list_dir(path);

    fsif.mkdir("c:/usr", 0);
    fsif.mkdir("c:/usr/share", 0);
    fsif.mkdir("c:/lib", 0);
    
    memset(path, 0, MAX_PATH);
    strcpy(path, "c:");
    fsal_list_dir(path);

    fsif.unlink("c:/usr/share");
    fsif.unlink("c:/usr");
    //fsif.unlink("c:/test");

    memset(path, 0, MAX_PATH);
    strcpy(path, "c:");
    fsal_list_dir(path);

    fsif.rename("c:/lib", "c:/usr2");
    fsif.rename("c:/usr2", "c:/usr3");
    
    memset(path, 0, MAX_PATH);
    strcpy(path, "c:");
    fsal_list_dir(path);

    fstate_t state;
    if (!fsif.state("c:/bin", &state)) {
        printf("file name:%s size:%d date:%x time:%x\n", state.d_name, state.d_size, state.d_date, state.d_time);
    }
    fsif.utime("c:/bin", 0, 0x12345678);
    if (!fsif.state("c:/bin", &state)) {
        printf("file name:%s size:%d date:%x time:%x\n", state.d_name, state.d_size, state.d_date, state.d_time);
    }

    int fidx = fsif.open("c:/bin", O_RDONLY);
    if (fidx < 0) {
        printf("[%s] %s: open file failed!\n", SRV_NAME, __func__);
        return -1;
    }
    printf("read file:");
    char ch;
    while (!fsif.feof(fidx)) {
        fsif.read(fidx, &ch, 1);
        printf("%c", ch);
    }
    printf("file size:%d pos:%d\n", fsif.fsize(fidx), fsif.ftell(fidx));

    fsif.read(fidx, &ch, 1);

    printf("file error:%d\n", fsif.ferror(fidx));

    fsif.rewind(fidx);
    printf("file pos:%d\n", fsif.ftell(fidx));
    
    fsif.rmdir("c:/usr3");

    memset(path, 0, MAX_PATH);
    strcpy(path, "c:");
    fsal_list_dir(path);

    //spin();


    return 0;
}
