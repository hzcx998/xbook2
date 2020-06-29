#include <stdlib.h>
#include <stdio.h>
#include <core/netsrv.h>
#include <drivers/netcard.h>
#include <sys/res.h>
#include <string.h>

/* 探测设备，储存起来 */

LIST_HEAD(netcard_list_head);

static int next_netcard_solt = 0;

static int netcard_solt_cache[NETCARD_SOLT_NR];

#define IS_BAD_SOLT(solt) \
        (solt < 0 || solt >= NETCARD_SOLT_NR)

#define SOLT_TO_HANDLE(solt) netcard_solt_cache[solt]

netcard_drvier_t drv_netcard;

/**
 * netcard_probe_device - 探测设备
 * @type: 设备的类型
 * 
 */
int netcard_probe_device(device_type_t type)
{
    /* 磁盘设备 */
    devent_t *p = NULL;
    devent_t devent;
    netcard_info_t *netcard;
    do {
        if (dev_scan(p, type, &devent))
            break;
#if DEBUG_LOCAL == 1
        printf("%s: %s: probe device %s\n", SRV_NAME, __func__, devent.de_name);
#endif    
        /* 添加到磁盘数组 */
        /* 创建一个设备信息 */
        netcard = malloc(sizeof(netcard_info_t));
        if (netcard == NULL)
            return -1;
        /* 填信息并添加到链表 */
        netcard->devent = devent;
        netcard->handle = -1;
        netcard->solt = next_netcard_solt++;
        list_add_tail(&netcard->list, &netcard_list_head);
        
        p = &devent;
    } while (1);

    return 0;
}

/**
 * netcard_print - 打印磁盘信息
 * 
 */
void netcard_info_print()
{
    netcard_info_t *netcard;
    list_for_each_owner (netcard, &netcard_list_head, list) {
        srvprint("probe device:%s type:%d\n",
            netcard->devent.de_name, netcard->devent.de_type);
    }
}

/**
 * netcard_res_find - 查找磁盘
 * 
 */
int netcard_res_find(char *name)
{
    netcard_info_t *netcard;
    list_for_each_owner (netcard, &netcard_list_head, list) {
        if (!strcmp(netcard->devent.de_name, name)) {
            return netcard->solt;
        }
    }
    return -1;
}



static int __open(int solt)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    netcard_info_t *netcard;
    list_for_each_owner (netcard, &netcard_list_head, list) {
        if (netcard->solt == solt) {
            netcard->handle = res_open(netcard->devent.de_name, RES_DEV, 0);
            if (netcard->handle < 0)
                return -1;
            // srvprint("open solt %d handle %d\n", solt, netcard->handle);
            /* 添加到插槽缓存 */
            netcard_solt_cache[solt] = netcard->handle;
            return 0;
        }
    }
    return -1;
}

static int __close(int solt)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    netcard_info_t *netcard;
    list_for_each_owner (netcard, &netcard_list_head, list) {
        if (netcard->solt == solt) {
            
            /* 关闭设备 */
            if (res_close(netcard->handle) != 0) 
                return -1;

            /* 从缓存中删除 */
            int i;
            for (i = 0; i < NETCARD_SOLT_NR; i++) {
                if (netcard_solt_cache[i] == netcard->handle) {
                    netcard_solt_cache[i] = -1;
                    break;
                } 
            }
            netcard->handle = -1;
            return 0;
        }
    }
    return -1;
}


static int __read(int solt, void *buffer, size_t size)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    int len = res_read(SOLT_TO_HANDLE(solt), 0, buffer, size);
    if (len <= 0)
        return -1;
    return len;
}

static int __write(int solt, void *buffer, size_t size)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    int len = res_write(SOLT_TO_HANDLE(solt), 0, buffer, size) < 0;
    if (len <= 0)
        return -1;
    return len;
}

static int __ioctl(int solt, unsigned int cmd, unsigned long arg)
{
    if (IS_BAD_SOLT(solt))
        return -1;
    if (res_ioctl(SOLT_TO_HANDLE(solt), cmd, arg) < 0)
        return -1;
    return 0;
}

int init_netcard_driver()
{
    if (netcard_probe_device(DEVICE_TYPE_PHYSIC_NETCARD) < 0)
        return -1;

    if (netcard_probe_device(DEVICE_TYPE_NETWORK) < 0)
        return -1;
    int i;
    for (i = 0; i < NETCARD_SOLT_NR; i++)
        netcard_solt_cache[i] = -1;

    netcard_info_print();

    drv_netcard.open = __open;
    drv_netcard.close = __close;
    drv_netcard.read = __read;
    drv_netcard.write = __write;
    drv_netcard.ioctl = __ioctl;

    return 0;
}
