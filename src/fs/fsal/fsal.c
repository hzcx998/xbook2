#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <fsal/dir.h>

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>

#include <xbook/memalloc.h>
#include <xbook/debug.h>
#include <xbook/fs.h>
#include <xbook/schedule.h>

// #define DEBUG_FSAL

fsal_file_t *fsal_file_table;

int fsal_file_table_init()
{
    fsal_file_table = mem_alloc(FSAL_FILE_OPEN_NR * sizeof(fsal_file_t));
    if (fsal_file_table == NULL) 
        return -1;
    memset(fsal_file_table, 0, FSAL_FILE_OPEN_NR * sizeof(fsal_file_t));
    return 0;
}

fsal_file_t *fsal_file_alloc()
{
    int i;
    for (i = 0; i < FSAL_FILE_OPEN_NR; i++) {
        if (!fsal_file_table[i].flags) {
            memset(&fsal_file_table[i], 0, sizeof(fsal_file_t));
            fsal_file_table[i].flags = FSAL_FILE_FLAG_USED;
            return &fsal_file_table[i];
        }
    }
    return NULL;
}

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
            if (fsif.readdir(dir, &de) < 0)
                break;
            if (de.d_attr & DE_DIR) {
                printk("%s/%s\n", path, de.d_name);
                i = strlen(path);
                sprintf(&path[i], "/%s", de.d_name);
                if (fsal_list_dir(path) < 0)
                    break;
                path[i] = 0;
            } else {
                printk("%s/%s  size=%d\n", path, de.d_name, de.d_size);
            }
        }
        fsif.closedir(dir);
    }
    return dir;
}

int fsal_disk_mount_init()
{
    char name[32];
    int i;
    for (i = 0; i < 4; i++) {
        memset(name, 0, 32);
        strcpy(name, "disk");
        char s[2] = {0, 0};
        s[0] = i + '0';
        strcat(name, s);
        if (fsif.mount(name, ROOT_DIR_PATH, "fat32", 0) < 0) {
            continue;
        }
        printk("fsal : mount device %s to path %s success.\n", name, ROOT_DIR_PATH);
        break;
    }
    if (i >= 4) {
        printk("fsal : mount path %s failed!\n", ROOT_DIR_PATH);
        return -1;
    }
    return 0;
}

int fs_fd_init(task_t *task)
{
    task->fileman = mem_alloc(sizeof(file_man_t));
    if (task->fileman == NULL) {
        return -1;
    }
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        task->fileman->fds[i].handle = -1;
        task->fileman->fds[i].flags = 0;
        task->fileman->fds[i].offset = 0;
    }
    memset(task->fileman->cwd, 0, MAX_PATH);
    strcpy(task->fileman->cwd, "/");
    return 0;
}

int fs_fd_exit(task_t *task)
{
    if (!task->fileman)
        return -1;
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++)
        fsif_degrow(i);
    
    mem_free(task->fileman);
    task->fileman = NULL;
    return 0;
}

int fs_fd_copy(task_t *src, task_t *dest)
{
    if (!src->fileman || !dest->fileman) {
        return -1;
    }
    memcpy(dest->fileman->cwd, src->fileman->cwd, MAX_PATH);
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        if (src->fileman->fds[i].flags != 0) {
            dest->fileman->fds[i].handle = src->fileman->fds[i].handle;
            dest->fileman->fds[i].flags = src->fileman->fds[i].flags;
            dest->fileman->fds[i].offset = src->fileman->fds[i].offset;
            fsif_grow(i);
        }
    }
    return 0;
}

/**
 * fs_fd_reinit - 重新初始化只保留前3个fd
 */
int fs_fd_reinit(task_t *cur)
{
    if (!cur->fileman) {
        return -1;
    }
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        if (cur->fileman->fds[i].flags != 0) {
            if (i >= 3)
                sys_close(i);
        }
    }
    return 0;
}

int fsal_fd_alloc()
{
    task_t *cur = task_current;
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        if (cur->fileman->fds[i].flags == 0) {
            cur->fileman->fds[i].flags = 1;
            cur->fileman->fds[i].handle = -1;
            cur->fileman->fds[i].offset = 0;
            return i;
        }
    }
    return -1;
}

int fsal_fd_free(int fd)
{
    task_t *cur = task_current;
    if (OUT_RANGE(fd, 0, LOCAL_FILE_OPEN_NR))
        return -1;
    if (cur->fileman->fds[fd].flags == 0) {
        return -1;
    }
    cur->fileman->fds[fd].handle = -1;
    cur->fileman->fds[fd].flags = 0;
    cur->fileman->fds[fd].offset = 0;
    return 0;
}

/**
 * local_fd_install - 安装到进程本地文件描述符表
 */
int local_fd_install(int resid, unsigned int flags)
{
    if (OUT_RANGE(resid, 0, FSAL_FILE_OPEN_NR))
        return -1;
    int fd = fsal_fd_alloc();
    if (fd < 0)
        return -1;
    task_t *cur = task_current;
    cur->fileman->fds[fd].handle = resid;
    cur->fileman->fds[fd].flags |= flags;
    return fd;
}

/**
 * local_fd_install_to - 安装到进程本地文件描述符表，并指明要安装到的fd
 */
int local_fd_install_to(int resid, int newfd, unsigned int flags)
{
    if (OUT_RANGE(resid, 0, FSAL_FILE_OPEN_NR))
        return -1;
    if (OUT_RANGE(newfd, 0, LOCAL_FILE_OPEN_NR))
        return -1;
    task_t *cur = task_current;
    cur->fileman->fds[newfd].handle = resid;
    cur->fileman->fds[newfd].flags = FILE_FD_ALLOC;
    cur->fileman->fds[newfd].flags |= flags;
    cur->fileman->fds[newfd].offset = 0;
    return newfd;
}

int local_fd_uninstall(int local_fd)
{
    if (OUT_RANGE(local_fd, 0, LOCAL_FILE_OPEN_NR))
        return -1;
    return fsal_fd_free(local_fd);
}

file_fd_t *fd_local_to_file(int local_fd)
{
    if (OUT_RANGE(local_fd, 0, LOCAL_FILE_OPEN_NR))
        return NULL;

    task_t *cur = task_current;
    return &cur->fileman->fds[local_fd];
}

int handle_to_local_fd(int handle, unsigned int flags)
{
    task_t *cur = task_current;
    file_fd_t *fdptr;
    int i;
    for (i = 0; i < LOCAL_FILE_OPEN_NR; i++) {
        fdptr = &cur->fileman->fds[i];
        if ((fdptr->handle == handle) && (fdptr->flags & flags)) {
            return i;   /* find the local fd */
        }
    }
    return -1;
}

int fsal_init()
{
    if (fsal_file_table_init() < 0) {
        return -1;
    }
    if (fsal_dir_table_init() < 0) {
        return -1;
    }
    if (fsal_path_init() < 0) {
        return -1;
    }
    if (fsal_disk_mount_init() < 0) {
        return -1;
    }
    char path[MAX_PATH] = {0};
    strcpy(path, "/root");
    fsal_list_dir(path);
    return 0;
}
