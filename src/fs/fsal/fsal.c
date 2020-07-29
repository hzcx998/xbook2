#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <fsal/dir.h>

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>

#include <xbook/kmalloc.h>
#include <xbook/debug.h>
#include <xbook/fs.h>

#define DEBUG_LOCAL 0


/* 文件表指针 */
fsal_file_t *fsal_file_table;

/**
 * init_fsal_file_table - 初始化文件表
 * 
 * 分配文件表内存，并清空
 */
int init_fsal_file_table()
{
    fsal_file_table = kmalloc(FSAL_FILE_OPEN_NR * sizeof(fsal_file_t));
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
        //printk("opendir %s ok.\n", path);
        while (1) {
            /* 读取目录项 */
            if (fsif.readdir(dir, &de) < 0)
                break;
            
            if (de.d_attr & DE_DIR) {   /* 是目录，就需要递归扫描 */
                printk("%s/%s\n", path, de.d_name);
                /* 构建新的路径 */
                i = strlen(path);
                sprintf(&path[i], "/%s", de.d_name);
                if (fsal_list_dir(path) < 0)
                    break;
                path[i] = 0;
            } else {    /* 直接列出文件 */
                printk("%s/%s  size=%d\n", path, de.d_name, de.d_size);
            }
        }
        fsif.closedir(dir);
    }
    //printk("opendir %s failed!.\n", path);
    return dir;
}

int init_disk_mount()
{
    /* 挂载文件系统 */
    if (fsif.mount(ROOT_DISK_NAME, ROOT_DIR_PATH, "fat32", 0) < 0) {
        printk("[%s] %s: mount failed!\n", FS_MODEL_NAME, __func__);
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
    
    char path[MAX_PATH] = {0};
    strcpy(path, "/root");
    fsal_list_dir(path);

    return 0;
}


int fs_fd_init(task_t *task)
{
    
    task->fileman = kmalloc(sizeof(file_man_t));
    if (task->fileman == NULL) {
        return -1;
    }
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        task->fileman->fds[i] = -1; /* no file */
    }
    memset(task->fileman->cwd, 0, MAX_PATH);
    strcpy(task->fileman->cwd, "/");

    return 0;
}

int fs_fd_exit(task_t *task)
{
    if (!task->fileman)
        return -1;
    kfree(task->fileman);
    task->fileman = NULL;
    return 0;
}

int fs_fd_copy(task_t *src, task_t *dest)
{
    if (!src->fileman || !dest->fileman) {
        return -1;
    }
    #if DEBUG_LOCAL == 1
    printk("[fs]: fd copy from %s to %s\n", src->name, dest->name);
    #endif
    /* 复制工作目录 */
    memcpy(dest->fileman->cwd, src->fileman->cwd, MAX_PATH);
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        if (src->fileman->fds[i] >= 0) {
            dest->fileman->fds[i] = src->fileman->fds[i]; 
            #if DEBUG_LOCAL == 1
            printk("[fs]: fds[%d]=%d\n", i, src->fileman->fds[i]);
            #endif
        }
    }
    return 0;
}

int fd_alloc()
{
    task_t *cur = current_task;
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        if (cur->fileman->fds[i] == -1) {
            cur->fileman->fds[i] = 0;     /* alloced */
            return i;
        }
    }
    return -1;
}

int fd_free(int fd)
{
    task_t *cur = current_task;
    if (OUT_RANGE(fd, 0, LOCAL_FILE_OPEN_NR))
        return -1;
    if (cur->fileman->fds[fd] == -1) {
        return -1;
    }
    cur->fileman->fds[fd] = -1;
    return 0;
}

int local_fd_install(int global_fd)
{
    if (OUT_RANGE(global_fd, 0, FSAL_FILE_OPEN_NR))
        return -1;

    int fd = fd_alloc();
    if (fd < 0)
        return -1;
    task_t *cur = current_task;
    cur->fileman->fds[fd] = global_fd;
    return fd;
}

int local_fd_uninstall(int local_fd)
{
    if (OUT_RANGE(local_fd, 0, LOCAL_FILE_OPEN_NR))
        return -1;
    return fd_free(local_fd);
}

int fd_local_to_global(int local_fd)
{
    if (OUT_RANGE(local_fd, 0, LOCAL_FILE_OPEN_NR))
        return -1;

    task_t *cur = current_task;
    return cur->fileman->fds[local_fd];
}