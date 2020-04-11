#include <xbook/resource.h>
#include <xbook/device.h>
#include <xbook/task.h>
#include <xbook/debug.h>
#include <xbook/sharemem.h>
#include <xbook/sem.h>
#include <xbook/msgqueue.h>
#include <sys/res.h>
#include <sys/ipc.h>

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
int install_res(unsigned long flags, unsigned long handle)
{
    resource_t *res = current_task->res;
    res_item_t *item;
    int i;
    for (i = 0; i < RES_NR; i++) {
        item = &res->table[i];
        if (item->flags == 0) { /* not used */
            item->flags = flags;
            item->handle = handle;
            return i;
        }
    }
    return -1;
}

/**
 * uninstall_res - 从进程空间卸载资源
 * 
 * @idx: res index
 * 
 */
int uninstall_res(int idx)
{
    if (IS_BAD_RES(idx)) {
        return -1;
    }
    resource_t *res = current_task->res;
    res_item_t *item = &res->table[idx];
    
    item->flags = 0;
    item->handle = 0;
    return 0;
}

int __getres_device(char *name, unsigned long resflg)
{
    dev_t handle = get_devno_by_name(name);
    if (!handle) {   /* not found a devno */
        return -1;
    }
    #if DEBUG_RES == 1
        printk(KERN_DEBUG "__getres_device: get devno %x\n", handle);
    #endif
    if (dev_open(handle, resflg & RES_LOCAL_MASK)) {
        return -1;
    }
    #if DEBUG_RES == 1
        printk(KERN_DEBUG "__getres_device: open done, ready install res.\n");
    #endif
    
    /* 打开资源成功，安装到进程中 */
    int res = install_res(resflg & RES_GLOBAL_MASK, handle);
    if (res == -1) { /* 失败后要关闭设备 */  
        #if DEBUG_RES == 1
            printk(KERN_DEBUG "__getres_device: install res failed!\n");
        #endif
        dev_close(handle);
        return -1;
    }
    return res;
}

int __getres_ipc(char *name, unsigned long resflg, unsigned long arg)
{
    int res = -1;
    unsigned long handle;
    if (resflg & IPC_SHM) { /* 共享内存 */
        handle = share_mem_get(name, arg, resflg & RES_LOCAL_MASK);
        if (handle == -1)
            return -1;
        /* 消除从类型中的其它类型 */
        resflg = (resflg & (RES_LOCAL_MASK | RES_MASTER_MASK)) | IPC_SHM;
        /* 打开资源成功，安装到进程中 */
        res = install_res(resflg, handle);
        if (res == -1) { /* 失败后要关闭设备 */  
            #if DEBUG_RES == 1
                printk(KERN_DEBUG "__getres_ipc: install res failed!\n");
            #endif
            share_mem_put(handle);
            return -1;
        }
    } else if (resflg & IPC_SEM) { /* 信号量 */
        handle = sem_get(name, arg, resflg & RES_LOCAL_MASK);
        if (handle == -1)
            return -1;
        /* 消除从类型中的其它类型 */
        resflg = (resflg & (RES_LOCAL_MASK | RES_MASTER_MASK)) | IPC_SEM;
        /* 打开资源成功，安装到进程中 */
        res = install_res(resflg, handle);
        if (res == -1) { /* 失败后要关闭设备 */  
            #if DEBUG_RES == 1
                printk(KERN_DEBUG "__getres_ipc: install res failed!\n");
            #endif
            sem_put(handle);
            return -1;
        }
    } else if (resflg & IPC_MSG) { /* 信号量 */
        handle = msg_queue_get(name, resflg & RES_LOCAL_MASK);
        if (handle == -1)
            return -1;
        /* 消除从类型中的其它类型 */
        resflg = (resflg & (RES_LOCAL_MASK | RES_MASTER_MASK)) | IPC_MSG;
        /* 打开资源成功，安装到进程中 */
        res = install_res(resflg, handle);
        if (res == -1) { /* 失败后要关闭设备 */  
            #if DEBUG_RES == 1
                printk(KERN_DEBUG "__getres_ipc: install res failed!\n");
            #endif
            msg_queue_put(handle);
            return -1;
        }
    }
    return res;
}

int __writeres_ipc(res_item_t *item, off_t off, void *buffer, size_t count)
{
    void *shmaddr;
    /* 根据从类型进行不同的操作 */
    switch (item->flags & RES_SLAVER_MASK)
    {
    case IPC_SHM:
        shmaddr = share_mem_map(item->handle, buffer);
        if (shmaddr == NULL) {
            return -1;
        }
        /* 如果buf为空，就需要保存映射后的地址到count里面 */
        if (buffer == NULL)
            *(size_t *)count = (size_t )shmaddr;
        break;
    case IPC_SEM:
        if (sem_down(item->handle, off))
            return -1;
        break;
    case IPC_MSG:
        if (msg_queue_send(item->handle, buffer, count, off))
            return -1;
        break;
    default:
        return -1;
    }
    return 0;
}

int __readres_ipc(res_item_t *item, off_t off, void *buffer, size_t count)
{
    long *msgtype = (long *) buffer;
    int read_bytes;
    /* 根据从类型进行不同的操作 */
    switch (item->flags & RES_SLAVER_MASK)
    {
    case IPC_SHM:
        if (share_mem_unmap(buffer))
            return -1;
        break;
    case IPC_SEM:
        if (sem_up(item->handle))
            return -1;
        break;
    case IPC_MSG:
        read_bytes = msg_queue_recv(item->handle, buffer, count, *msgtype, off);
        return read_bytes;
    default:
        return -1;
    }
    return 0;
}
/**
 * sys_getres - 获取资源
 * @resname: 资源名
 * @resflg: 资源标志，由本地标志和全局标志组成。
 *          本地标志：0-15位，全局标志16-31位。
 * @arg: 参数
 * 
 * @return: 成功返回资源，失败返回-1
 */
int sys_getres(char *resname, unsigned long resflg, unsigned long arg)
{
    #if DEBUG_RES == 1
        printk(KERN_DEBUG "sys_getres: resname=%s resflg=%x\n", resname, resflg);
    #endif
    int res = -1;
    if (resflg & RES_DEV) {     /* 设备类型资源 */
        /* 消除主类型中的其它类型 */
        resflg = (resflg & (RES_LOCAL_MASK | RES_SLAVER_MASK)) | RES_DEV;
        res = __getres_device(resname, resflg);
    } else if (resflg & RES_IPC) {
        /* 消除主类型中的其它类型 */
        resflg = (resflg & (RES_LOCAL_MASK | RES_SLAVER_MASK)) | RES_IPC;
        res = __getres_ipc(resname, resflg, arg);
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
    res_item_t *item = res_to_item(res);
    if (item == NULL)
        return -1;
    
    switch (item->flags & RES_MASTER_MASK)
    {
    case RES_DEV:
        if (dev_close(item->handle)) {
            return -1;
        }
        break;
    case RES_IPC:
        /* 根据从类型进行不同的操作 */
        switch (item->flags & RES_SLAVER_MASK)
        {
        case IPC_SHM:
            if (share_mem_put(item->handle))
                return -1;
            break;
        case IPC_SEM:
            if (sem_put(item->handle))
                return -1;
            break;
        case IPC_MSG:
            if (msg_queue_put(item->handle))
                return -1;
            break;
        default:
            return -1;
        }
        break;
    default:
        return -1;  /* error type */
    }
#if DEBUG_RES == 1
    printk(KERN_DEBUG "sys_putres: uninstall res, the handle %x.\n", item->handle);
#endif    
    uninstall_res(res);

    return 0;
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
    switch (item->flags & RES_MASTER_MASK)
    {
    case RES_DEV:
        #if DEBUG_RES == 2
            printk(KERN_DEBUG "sys_readres: devno=%x.\n", item->handle);
        #endif   
        if (dev_read(item->handle, off, buffer, count)) {
            return -1;
        }
        break;
    case RES_IPC:
        if (__readres_ipc(item, off, buffer, count))
            return -1;
        break;
    default:
        return -1;  /* error type */
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
    printk(KERN_DEBUG "sys_writeres: devno=%x.\n", item->handle);
    #endif    
    switch (item->flags & RES_MASTER_MASK)
    {
    case RES_DEV:
        #if DEBUG_RES == 2
            printk(KERN_DEBUG "sys_writeres: devno=%x.\n", item->handle);
        #endif   
        if (dev_write(item->handle, off, buffer, count)) {
            return -1;
        }
        break;
    case RES_IPC:
        if (__writeres_ipc(item, off, buffer, count))
            return -1;
        break;
    default:
        return -1;  /* error type */
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
    printk(KERN_DEBUG "sys_ctlres: devno=%x.\n", item->handle);
#endif  
    switch (item->flags & RES_MASTER_MASK)
    {
    case RES_DEV:
        #if DEBUG_RES == 2
            printk(KERN_DEBUG "sys_ctlres: devno=%x.\n", item->handle);
        #endif   
        if (dev_ioctl(item->handle, cmd, arg)) {
            return -1;
        }
        break;
    case RES_IPC:
    default:
        return -1;  /* error type */
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
void resource_copy(resource_t *dst, resource_t *src)
{
    if (src == NULL || dst == NULL)
        return;
    int i;
    res_item_t *item;
    for (i = 0; i < RES_NR; i++) {
        item = &src->table[i];
        if (item->flags) { /* 项在使用中 */
            switch (item->flags & RES_MASTER_MASK)
            {
            case RES_DEV:
                dst->table[i] = *item;  /* 复制项 */
                /* 设备增长 */
                dev_grow(item->handle);
                break;
            case RES_IPC:
                /* 不复制ipc资源 */
                /* code */
                break;
            default:
                break;
            }
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
    res_item_t *item;
    for (i = 0; i < RES_NR; i++) {
        item = &res->table[i];
        if (item->flags) {      
#if DEBUG_RES == 1
    printk(KERN_DEBUG "resource_release: index=%d handle=%x flags=%x.\n", i, item->handle, item->flags);
#endif
            switch (item->flags & RES_MASTER_MASK)
            {
            case RES_DEV:
                /* 设备增长 */
                dev_close(res->table[i].handle);
                break;
            case RES_IPC:
                /* 根据从类型进行不同的操作 */
                switch (item->flags & RES_SLAVER_MASK)
                {
                case IPC_SHM:
                    share_mem_put(item->handle);
                    break;
                case IPC_SEM:
                    sem_put(item->handle);
                    break;
                case IPC_MSG:
                    msg_queue_put(item->handle);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
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
    res_item_t *item;
    for (i = 0; i < RES_NR; i++) {
        item = &res->table[i];
        switch (item->flags & RES_MASTER_MASK)
        {
        case RES_DEV:
            /* 设备增长 */
            printk(KERN_DEBUG "dump_resource: index=%d devno=%x\n",
                i, item->handle);
            break;
        case RES_IPC:
            printk(KERN_DEBUG "dump_resource: index=%d ipcno=%x\n",
                i, item->handle);
            /* code */
            break;
        default:
            break;
        }
    }
}

void resource_init(resource_t *res)
{
    if (res == NULL)
        return;
    int i;
    for (i = 0; i < RES_NR; i++) {
        res->table[i].flags = 0;
        res->table[i].handle = 0;
    }
}