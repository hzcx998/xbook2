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
#include <cpio.h>

#ifdef GRUB2
#include <arch/module.h>
#include <arch/page.h>
#endif /* GRUB2 */

#include <xbook/memalloc.h>
#include <xbook/debug.h>
#include <xbook/fs.h>
#include <xbook/schedule.h>

// #define DEBUG_FSAL

// #define LIST_ALL_FILE

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

int fsal_disk_mount(char *pathname, int max_try)
{
    /* 挂载根磁盘 */
    char name[32];
    int i;
    for (i = 0; i < max_try; i++) {
        memset(name, 0, 32);
        strcpy(name, pathname);
        char s[2] = {0, 0};
        s[0] = i + 'a';
        strcat(name, s);
        if (fsif.mount(name, ROOT_DIR_PATH, "fat32", 0) < 0) {
            continue;
        }
        keprint("fsal : mount device %s to path %s success.\n", name, ROOT_DIR_PATH);
        break;
    }
    if (i >= max_try) {
        keprint("fsal : mount path %s to %s failed!\n", pathname, ROOT_DIR_PATH);
        return -1;
    }
    return 0;
}
#ifdef GRUB2
static void cpio_extract_from_memory(void *archive, const char* dir) {
    int i;
    struct cpio_info info;
    char *filename;
    unsigned long file_sz;
    void* file_buf;
    int extract_file;
    char *path;
    int dir_len = strlen(dir);

    cpio_info(archive, &info);
    path = (char*)mem_alloc(sizeof(char) * (info.max_path_sz + dir_len + 1));

    for (i = 0; i < info.file_count; ++i) {
        file_buf = cpio_get_entry(archive, i, (const char**)&filename, &file_sz);
        strcpy(path, dir);
        strcpy(path + dir_len, filename);
        // Default directory instead of file
        if (file_sz == 0) {
            kfile_mkdir(path, 0);
        } else {
            extract_file = kfile_open(filename, O_CREAT | O_RDWR);
            kfile_write(extract_file, file_buf, file_sz);
            kfile_close(extract_file);
        }
    } keprint("\n");

    mem_free(path);
}
#endif /* GRUB2 */

int fsal_disk_mount_init()
{
#ifndef CONFIG_LIVECD
    if (!fsal_disk_mount("/dev/hd", 4))
        return 0;
    if (!fsal_disk_mount("/dev/sd", 16))
        return 0;
#endif /* CONFIG_LIVECD */
#ifdef GRUB2
    if (fsif.mount("/dev/ram0", ROOT_DIR_PATH, "fat32", MT_REMKFS) > -1) {
        int i;
        struct modules_info_block *modules_info;
        void *initrd_buf = NULL;

        keprint("fsal : mount device initrd to path %s success.\n" ROOT_DIR_PATH);
        modules_info = (struct modules_info_block *)(KERN_BASE_VIR_ADDR + MODULE_INFO_ADDR);

        for (i = 0; i < modules_info->modules_num; ++i) {
            if (modules_info->modules[i].type == MODULE_INITRD) {
                initrd_buf = (void*)(KERN_BASE_VIR_ADDR + modules_info->modules[i].start);
                break;
            }
        }

        if (initrd_buf == NULL) {
            goto fail;
        }

        cpio_extract_from_memory(initrd_buf, "/");

        return 0;
    }
#endif /* GRUB2 */
fail:
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
    /* 挂载根目录 */
    if (fsal_disk_mount_init() < 0) {
        return -1;
    }
    
    /* 创建核心目录 */
    if (kfile_mkdir(HOME_DIR_PATH, 0) < 0)
        warnprint("fsal create dir %s failed or dir existed!\n", HOME_DIR_PATH);
    if (kfile_mkdir(ACCOUNT_DIR_PATH, 0) < 0)
        warnprint("fsal create dir %s failed or dir existed!\n", ACCOUNT_DIR_PATH);
    if (kfile_mkdir(DEV_DIR_PATH, 0) < 0)
        warnprint("fsal create dir %s failed or dir existed!\n", DEV_DIR_PATH);
    
    /* ramfs */
    #if defined(RAMFS_DIR_PATH)
    if (kfile_mkdir(RAMFS_DIR_PATH, 0) < 0)
        warnprint("fsal create dir %s failed or dir existed!\n", RAMFS_DIR_PATH);
    if (fsif.mkfs("ram0", "fat16", 0) < 0) {
        keprint("fsal : mkfs on device %s failed!\n", "ram0");
    }
    if (fsif.mount("/dev/ram0", RAMFS_DIR_PATH, "fat16", 0) < 0) {
        keprint("fsal : mount path %s failed!\n", RAMFS_DIR_PATH);
    }
    #endif  /* RAMFS_DIR_PATH */
    /* 挂载设备目录，不会真正挂载到磁盘上，是挂载到内存中的 */
    if (fsif.mount("/dev/ram0", DEV_DIR_PATH, "devfs", 0) < 0) {
        keprint("fsal : mount path %s failed!\n", DEV_DIR_PATH);
        return -1;
    }
    
    /* 挂载FIFO目录，不会真正挂载到磁盘上，是挂载到内存中的 */
    if (fsif.mount("/dev/ram0", FIFO_DIR_PATH, "fifofs", 0) < 0) {
        keprint("fsal : mount path %s failed!\n", FIFO_DIR_PATH);
        return -1;
    }
    
    #if defined(LIST_ALL_FILE)
    char path[MAX_PATH] = {0};
    strcpy(path, "/root");
    fsal_list_dir(path);
    #endif
    
    return 0;
}
