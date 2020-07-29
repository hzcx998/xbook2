#include <list.h>
#include <xbook/diskman.h>
#include <xbook/resource.h>
#include <xbook/kmalloc.h>
#include <string.h>
#include <stdio.h>

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

        if (sys_devscan(p, type, &devent))
            break;
        /* 添加到磁盘数组 */
        /* 创建一个设备信息 */
        disk = kmalloc(sizeof(disk_info_t));
        if (disk == NULL)
            return -1;
        /* 填信息并添加到链表 */
        disk->devent = devent;
        disk->handle = -1;
        atomic_set(&disk->ref, 0);
        /* 设置虚拟磁盘名字 */
        sprintf(disk->virname, "disk%d", next_disk_solt);
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
        pr_info("[diskman]: probe device:%s -> vir:%s type:%d\n",
            disk->devent.de_name, disk->virname, disk->devent.de_type);
    }
}

/**
 * disk_res_find - 查找磁盘
 * 
 */
int disk_res_find(char *name)
{
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (!strcmp(disk->virname, name)) {
            return disk->solt;
        }
    }
    return -1;
}

/**
 * disk_res_find - 查找磁盘
 * 
 */
disk_info_t *disk_res_find_info(char *name)
{
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (!strcmp(disk->virname, name)) {
            return disk;
        }
    }
    return NULL;
}

static int __open(int solt)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (disk->solt == solt) {
            if (atomic_get(&disk->ref) == 0) {
                disk->handle = device_open(disk->devent.de_name, 0);
                if (disk->handle < 0)
                    return -1;
                printk("[diskman]: %s: get disk handle %d\n", __func__, disk->handle);
                /* 添加到插槽缓存 */
                disk_solt_cache[solt] = disk->handle;
            }
            atomic_inc(&disk->ref);
            
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
            if (atomic_get(&disk->ref) == 1) {
                /* 关闭设备 */
                if (device_close(disk->handle) != 0) 
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
            } else if (atomic_get(&disk->ref) == 0) {
                pr_dbg("[diskman]: close device %d without open!\n", solt);
                return -1;
            }
            atomic_dec(&disk->ref);
            return 0;
        }
    }
    return -1;
}


static int __read(int solt, off_t off, void *buffer, size_t size)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (device_read(SOLT_TO_HANDLE(solt), buffer, size, off) < 0)
        return -1;
    return 0;
}

static int __write(int solt, off_t off, void *buffer, size_t size)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (device_write(SOLT_TO_HANDLE(solt), buffer, size, off) < 0)
        return -1;
    return 0;
}

static int __ioctl(int solt, unsigned int cmd, unsigned long arg)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (device_devctl(SOLT_TO_HANDLE(solt), cmd, arg) < 0)
        return -1;
    return 0;
}

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
