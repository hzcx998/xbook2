#include <xbook/list.h>
#include <xbook/diskman.h>
#include <xbook/memalloc.h>
#include <string.h>
#include <stdio.h>

LIST_HEAD(disk_list_head);
static int next_disk_solt = 0;
static int disk_solt_cache[DISK_MAN_SOLT_NR];

#define IS_BAD_SOLT(solt) \
        (solt < 0 || solt >= DISK_MAN_SOLT_NR)

#define SOLT_TO_HANDLE(solt) disk_solt_cache[solt]

DEFINE_MUTEX_LOCK(disk_manager_mutex);

/* 磁盘管理器，对磁盘的操作都封装在里面 */
disk_manager_t diskman;

int disk_manager_probe_device(device_type_t type)
{
    devent_t *p = NULL;
    devent_t devent;
    disk_info_t *disk;
    do {
        if (sys_devscan(p, type, &devent))
            break;
        disk = mem_alloc(sizeof(disk_info_t));
        if (disk == NULL)
            return -1;
        disk->devent = devent;
        disk->handle = -1;
        atomic_set(&disk->ref, 0);
        /* 设置虚拟磁盘名字 */
        mutex_lock(&disk_manager_mutex);
        sprintf(disk->virname, "disk%d", next_disk_solt);
        disk->solt = next_disk_solt++;
        list_add_tail(&disk->list, &disk_list_head);
        mutex_unlock(&disk_manager_mutex);
        p = &devent;
    } while (1);
    return 0;
}

void disk_info_print()
{
    mutex_lock(&disk_manager_mutex);
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        infoprint("[diskman]: probe device:%s -> vir:%s type:%d\n",
            disk->devent.de_name, disk->virname, disk->devent.de_type);
    }
    mutex_unlock(&disk_manager_mutex);
}

int disk_info_find(char *name)
{
    mutex_lock(&disk_manager_mutex);
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (!strcmp(disk->virname, name)) {
            mutex_unlock(&disk_manager_mutex);
            return disk->solt;
        }
    }
    mutex_unlock(&disk_manager_mutex);
    return -1;
}

disk_info_t *disk_info_find_info(char *name)
{
    mutex_lock(&disk_manager_mutex);
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (!strcmp(disk->virname, name)) {
            mutex_unlock(&disk_manager_mutex);
            return disk;
        }
    }
    mutex_unlock(&disk_manager_mutex);
    return NULL;
}

static int disk_manager_open(int solt)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    mutex_lock(&disk_manager_mutex);
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (disk->solt == solt) {
            if (atomic_get(&disk->ref) == 0) {
                disk->handle = device_open(disk->devent.de_name, 0);
                if (disk->handle < 0) {
                    mutex_unlock(&disk_manager_mutex);
                    return -1;
                }
                disk_solt_cache[solt] = disk->handle;
            }
            atomic_inc(&disk->ref);
            mutex_unlock(&disk_manager_mutex);
            return 0;
        }
    }
    mutex_unlock(&disk_manager_mutex);
    return -1;
}

static int disk_manager_close(int solt)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    mutex_lock(&disk_manager_mutex);
    disk_info_t *disk;
    list_for_each_owner (disk, &disk_list_head, list) {
        if (disk->solt == solt) {
            if (atomic_get(&disk->ref) == 1) {
                if (device_close(disk->handle) != 0) {
                    mutex_unlock(&disk_manager_mutex);
                    return -1;
                }
                int i;
                for (i = 0; i < DISK_MAN_SOLT_NR; i++) {
                    if (disk_solt_cache[i] == disk->handle) {
                        disk_solt_cache[i] = -1;
                        break;
                    } 
                }
                disk->handle = -1;
            } else if (atomic_get(&disk->ref) == 0) {
                dbgprint("[diskman]: close device %d without open!\n", solt);
                mutex_unlock(&disk_manager_mutex);
                return -1;
            }
            atomic_dec(&disk->ref);
            mutex_unlock(&disk_manager_mutex);
            return 0;
        }
    }
    mutex_unlock(&disk_manager_mutex);
    return -1;
}

static int disk_manager_read(int solt, off_t off, void *buffer, size_t size)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (device_read(SOLT_TO_HANDLE(solt), buffer, size, off) < 0)
        return -1;
    return 0;
}

static int disk_manager_write(int solt, off_t off, void *buffer, size_t size)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (device_write(SOLT_TO_HANDLE(solt), buffer, size, off) < 0)
        return -1;
    return 0;
}

static int disk_manager_ioctl(int solt, unsigned int cmd, unsigned long arg)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (device_devctl(SOLT_TO_HANDLE(solt), cmd, arg) < 0)
        return -1;
    return 0;
}

int disk_manager_init()
{
    if (disk_manager_probe_device(DEVICE_TYPE_DISK) < 0)
        return -1;
    if (disk_manager_probe_device(DEVICE_TYPE_VIRTUAL_DISK) < 0)
        return -1;
    int i;
    for (i = 0; i < DISK_MAN_SOLT_NR; i++)
        disk_solt_cache[i] = -1;

    disk_info_print();
    diskman.open = disk_manager_open;
    diskman.close = disk_manager_close;
    diskman.read = disk_manager_read;
    diskman.write = disk_manager_write;
    diskman.ioctl = disk_manager_ioctl;
    return 0;
}
