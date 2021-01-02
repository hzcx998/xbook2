#include <xbook/fsal.h>
#include <xbook/fatfs.h>
#include <xbook/dir.h>
#include <xbook/path.h>
#include <xbook/file.h>

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
                keprint("%s/%s\n", path, de.d_name);
                i = strlen(path);
                sprintf(&path[i], "/%s", de.d_name);
                if (fsal_list_dir(path) < 0)
                    break;
                path[i] = 0;
            } else {
                keprint("%s/%s  size=%d\n", path, de.d_name, de.d_size);
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
        keprint("fsal : mount device %s to path %s success.\n", name, ROOT_DIR_PATH);
        break;
    }
    if (i >= 4) {
        keprint("fsal : mount path %s failed!\n", ROOT_DIR_PATH);
        return -1;
    }
    return 0;
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

    /* 创建核心目录 */
    if (kfile_mkdir(HOME_DIR_PATH, 0) < 0)
        warnprint("fsal create dir %s failed or dir existed!\n", HOME_DIR_PATH);
    if (kfile_mkdir(ACCOUNT_DIR_PATH, 0) < 0)
        warnprint("fsal create dir %s failed or dir existed!\n", ACCOUNT_DIR_PATH);
        
    char path[MAX_PATH] = {0};
    strcpy(path, "/root");
    fsal_list_dir(path);
    return 0;
}
