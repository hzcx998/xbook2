#include <stdlib.h>
#include <stdio.h>
#include <core/filesrv.h>
#include <drivers/disk.h>
#include <sys/res.h>

#include <ffconf.h>

/* 探测设备，储存起来 */

LIST_HEAD(disk_list_head);

static int next_disk_solt = 0;

static int disk_solt_cache[DISK_SOLT_NR];

#define IS_BAD_SOLT(solt) \
        (solt < 0 || solt >= DISK_SOLT_NR)

#define SOLT_TO_HANDLE(solt) disk_solt_cache[solt]

disk_drvier_t drv_disk;

/**
 * disk_probe_device - 探测设备
 * @type: 设备的类型
 * 
 */
int disk_probe_device(device_type_t type)
{
    /* 磁盘设备 */
    devent_t *p = NULL;
    devent_t devent;
    disk_info_t *disk;
    do {
        if (dev_scan(p, type, &devent))
            break;
#if DEBUG_LOCAL == 1
        printf("%s: %s: probe device %s\n", SRV_NAME, __func__, devent.de_name);
#endif    
        /* 添加到磁盘数组 */
        /* 创建一个设备信息 */
        disk = malloc(sizeof(disk_info_t));
        if (disk == NULL)
            return -1;
        /* 填信息并添加到链表 */
        disk->devent = devent;
        disk->handle = -1;
        disk->solt = next_disk_solt++;
        list_add_tail(&disk->list, &disk_list_head);
        
        p = &devent;
    } while (1);

    return 0;
}

/**
 * disk_print - 打印磁盘信息
 * 
 */
void disk_info_print()
{
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        printf("%s: %s: probe device:%s type:%d\n", SRV_NAME, __func__,
            disk->devent.de_name, disk->devent.de_type);
    }
}

static int __open(int solt)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (disk->solt == solt) {
            disk->handle = res_open(disk->devent.de_name, RES_DEV, 0);
            if (disk->handle < 0)
                return -1;
            /* 添加到插槽缓存 */
            disk_solt_cache[solt] = disk->handle;
            return 0;
        }
    }
    return -1;
}

static int __close(int solt)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (disk->solt == solt) {
            
            /* 关闭设备 */
            if (res_close(disk->handle) != 0) 
                return -1;

            /* 从缓存中删除 */
            int i;
            for (i = 0; i < DISK_SOLT_NR; i++) {
                if (disk_solt_cache[i] == disk->handle) {
                    disk_solt_cache[i] = -1;
                    break;
                } 
            }
            disk->handle = -1;
            return 0;
        }
    }
    return -1;
}


static int __read(int solt, off_t off, void *buffer, size_t size)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (res_read(SOLT_TO_HANDLE(solt), off, buffer, size) < 0)
        return -1;
    return 0;
}

static int __write(int solt, off_t off, void *buffer, size_t size)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (res_write(SOLT_TO_HANDLE(solt), off, buffer, size) < 0)
        return -1;
    return 0;
}

static int __ioctl(int solt, unsigned int cmd, unsigned long arg)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (res_ioctl(SOLT_TO_HANDLE(solt), cmd, arg) < 0)
        return -1;
    return 0;
}

/* 文件系统驱动映射表 */
int fatfs_drv_map[FF_VOLUMES] = {
    1,0,2,3,4,5,6,7,8,9
};

int init_disk_driver()
{
    if (disk_probe_device(DEVICE_TYPE_DISK) < 0)
        return -1;
    if (disk_probe_device(DEVICE_TYPE_VIRTUAL_DISK) < 0)
        return -1;
    int i;
    for (i = 0; i < DISK_SOLT_NR; i++)
        disk_solt_cache[i] = -1;

    disk_info_print();

    drv_disk.open = __open;
    drv_disk.close = __close;
    drv_disk.read = __read;
    drv_disk.write = __write;
    drv_disk.ioctl = __ioctl;

    return 0;
}
