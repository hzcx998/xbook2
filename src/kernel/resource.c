#include <xbook/resource.h>
#include <xbook/device.h>
#include <xbook/task.h>
#include <xbook/debug.h>

/* debug: 1 enable, 0 disable */
#define DEBUG_RES 0

res_item_t *res_to_item(int res)
{
    task_t *cur = current_task;
    if (cur->res == NULL)
        return NULL;
    if (IS_BAD_RES(res))
        return NULL;
    
    return &cur->res->table[res];
}

/**
 * install_res - 安装资源到进程空间
 * 
 * @devno: 设备号
 * 
 */
int install_res(dev_t devno)
{
    resource_t *res = current_task->res;
    res_item_t *item;
    int i;
    for (i = 0; i < RES_NR; i++) {
        item = &res->table[i];
        if (item->devno == 0) { /* not used */
            item->devno = devno;
            item->off = 0;
            return i;
        }
    }
    return -1;
}

/**
 * uninstall_res - 从进程空间卸载资源
 * 
 * @devno: 设备号
 * 
 */
dev_t uninstall_res(int idx)
{
    if (IS_BAD_RES(idx)) {
        return 0;
    }
    resource_t *res = current_task->res;
    res_item_t *item = &res->table[idx];
    dev_t devno = item->devno;
    item->devno = 0;
    return devno;
    
}

/**
 * sys_getres - 获取资源
 * @resname: 资源名
 * @resflg: 资源标志
 * 
 * @return: 成功返回资源，失败返回-1
 */
int sys_getres(char *resname, unsigned long resflg)
{
#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_getres: resname=%s resflg=%x\n", resname, resflg);
#endif
    dev_t devno = get_devno_by_name(resname);
    if (!devno) {   /* not found a devno */
        return -1;
    }

#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_getres: get devno %x\n", devno);
#endif
    if (dev_open(devno, resflg)) {
        return -1;
    }

#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_getres: open done, ready install res.\n");
#endif
    /* 打开资源成功，加载到进程中 */
    int res = install_res(devno);
    if (res == -1) { /* 失败后要关闭设备 */  
#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_getres: install res failed!\n");
#endif
        dev_close(devno);
        return -1;
    }

#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_getres: install res success.\n");
#endif
    return res;
}

/**
 * sys_putres - 释放资源
 * @res: 资源
 * 
 * @return: 成功返回0，失败返回-1
 */
int sys_putres(int res)
{
#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_putres: res index %d\n", res);
#endif    
    dev_t devno = uninstall_res(res);
#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_putres: uninstall res, the devno %x.\n", devno);
#endif    
    if (!devno)
        return -1;
    return dev_close(devno);
}

/**
 * sys_readres - 读取资源
 * @res: 资源
 * @off: 数据便宜（对于磁盘而言才有意义）
 * @buffer: 缓冲区
 * @count: 数据量
 * 
 * @return: 成功返回0，失败返回-1
 */
int sys_readres(int res, off_t off, void *buffer, size_t count)
{
#if DEBUG_RES == 2
    printk(KERN_DEBUG "sys_readres: res index %d buffer=%x count=%d.\n",
        res, buffer, count);
#endif   
    res_item_t *item = res_to_item(res);
    if (item == NULL)
        return -1;
#if DEBUG_RES == 2
    printk(KERN_DEBUG "sys_readres: devno=%x.\n", item->devno);
#endif    
    if (!item->devno) /* devno err */
        return -1;
    if (dev_read(item->devno, off, buffer, count)) {
        return -1;
    }
#if DEBUG_RES == 2
    printk(KERN_DEBUG "sys_readres: read done!\n");
#endif    
    return 0;
}

/**
 * sys_writeres - 写入资源
 * @res: 资源
 * @off: 数据便宜（对于磁盘而言才有意义）
 * @buffer: 缓冲区
 * @count: 数据量，磁盘是扇区数
 * 
 * @return: 成功返回0，失败返回-1
 */
int sys_writeres(int res, off_t off, void *buffer, size_t count)
{
#if DEBUG_RES == 2
    printk(KERN_DEBUG "sys_writeres: res index %d buffer=%x count=%d.\n",
        res, buffer, count);
#endif    
    res_item_t *item = res_to_item(res);
    if (item == NULL)
        return -1;
#if DEBUG_RES == 2
    printk(KERN_DEBUG "sys_writeres: devno=%x.\n", item->devno);
#endif    
    if (!item->devno) /* devno err */
        return -1;
    if (dev_write(item->devno, off, buffer, count)) {
        return -1;
    }
#if DEBUG_RES == 2
    printk(KERN_DEBUG "sys_writeres: write done!\n");
#endif    
    return 0;
}


/**
 * sys_ctlres - 控制资源
 * @res: 资源
 * @buffer: 缓冲区
 * @count: 数据量，磁盘是扇区数
 * 
 * @return: 成功返回0，失败返回-1
 */
int sys_ctlres(int res, unsigned int cmd, unsigned long arg)
{
#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_ctlres: res index %d cmd=%d arg=%x.\n",
        res, cmd, arg);
#endif  
    res_item_t *item = res_to_item(res);
    if (item == NULL)
        return -1;
#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_ctlres: devno=%x.\n", item->devno);
#endif  
    if (!item->devno) /* devno err */
        return -1;
    if (dev_ioctl(item->devno, cmd, arg)) {
        return -1;
    }
#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_ctlres: control done!\n");
#endif  
    return 0;
}

/**
 * resource_copy - 复制资源
 * 
 * @res: 资源
 */
void resource_copy(resource_t *res)
{
    if (res == NULL)
        return;
    int i;
    for (i = 0; i < RES_NR; i++) {
        if (res->table[i].devno > 0) { /* 表项在使用中才能生长 */
            /* 设备增长 */
            dev_grow(res->table[i].devno);
        }
    }
}
/**
 * resource_release - 释放资源
 * 
 * @res: 资源
 */
void resource_release(resource_t *res)
{
    if (res == NULL)
        return;
    int i;
    for (i = 0; i < RES_NR; i++) {
        if (res->table[i].devno > 0) { /* 表项在使用中才能关闭 */
            /* 关闭设备 */
            dev_close(res->table[i].devno);
        }
    }
}

/**
 * dump_resource - 调试
 * 
 * @res: 资源
 */
void dump_resource(resource_t *res)
{
    if (res == NULL)
        return;
    int i;
    for (i = 0; i < RES_NR; i++) {
        if (res->table[i].devno != 0) {
            printk(KERN_DEBUG "dump_resource: index=%d devno=%x\n",
                i, res->table[i].devno);
        }
    }
}

void resource_init(resource_t *res)
{
    if (res == NULL)
        return;
    int i;
    for (i = 0; i < RES_NR; i++) {
        res->table[i].devno = 0;
        res->table[i].off = 0;
    }
}