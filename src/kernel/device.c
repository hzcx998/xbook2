#include <xbook/debug.h>
#include <xbook/device.h>
#include <xbook/string.h>

#define DEBUG_LOCAL 1

/* 设备链表 */
LIST_HEAD(device_list_head);

/* 设备缓存表，用于通过设备名找到设备号 */
device_cache_t device_cache_table[MAX_DEVICE_CACHE_NR] = {{0, 0},};

/*
构建一个散列表，用来把设备号和结构进行对应，可以提高
对设备的搜索速度
*/

/**
 * search_device - 查找一个设备
 * @name: 设备名
 * 
 * 找到返回1，没有返回0
 */
int search_device(char *name)
{
    device_t *device;
    list_for_each_owner(device, &device_list_head, list) {
        /* 如果名字相等就说明找到 */
        if (!strcmp(device->name, name)) {
            return 1;
        }
    }
    return 0;
}

/**
 * get_devno_by_name - 获取设备号
 * @name: 设备名
 * 
 * 返回设备结构体
 */
dev_t get_devno_by_name(char *name)
{
    /* 先到高速缓存中寻找 */
    device_cache_t *cache = &device_cache_table[0];
    for (cache = &device_cache_table[0]; cache < device_cache_table + MAX_DEVICE_CACHE_NR; cache++) {
        if (cache->devno && !strcmp(cache->name, name)) {   /* find */
            return cache->devno;
        }
    }
    /* 没找到，就到链表中查找 */
    device_t *device;
    list_for_each_owner(device, &device_list_head, list) {
        /* 如果名字相等就说明找到 */
        if (!strcmp(device->name, name)) {
            /* 添加到高速缓存 */
            for (cache = &device_cache_table[0]; cache < device_cache_table + MAX_DEVICE_CACHE_NR; cache++) {
                if (!cache->devno) {   /* find a free solt */
                    cache->devno = device->devno;
                    cache->name = device->name;
                    return cache->devno;
                }
            }
        }
    }
    return 0;   /* not find the devno */
}

/**
 * get_device_by_name - 获取设备
 * @name: 设备名
 * 
 * 返回设备结构体
 */
device_t *get_device_by_name(char *name)
{
    device_t *device;
    list_for_each_owner(device, &device_list_head, list) {
        /* 如果名字相等就说明找到 */
        if (!strcmp(device->name, name)) {
            return device;
        }
    }
    return NULL;
}

void dump_device(device_t *device)
{
    printk("name:%s devno:%x type:%d ops:%x ",
        device->name, device->devno, device->type, device->ops);
    printk("local:%x ref:%d\n",
        device->local, device->references);
}

/**
 * dump_devices - 打印所有设备信息
 */
void dump_devices()
{
    device_t *device;
    list_for_each_owner(device, &device_list_head, list) {
        dump_device(device);
    }
}

/**
 * is_bad_device - 检测是否为错误的设备
 * @devno: 设备的id
 * 
 * 如果是错误id就返回1，否则返回0
 */
static int is_bad_device(int devno)
{
    if (!devno)
        return -1;
    return 0;
}

/**
 * get_device_by_id - 通过设备ID获取设备项
 * devno: 设备id
 * 
 * 如果以后不用数组来转换，直接修改这里面就行了
 */
device_t *get_device_by_id(dev_t devno)
{
    /* 先用散列表的形式搜索 */

    /* 再用链表搜索的方式 */
    device_t *device;
    list_for_each_owner(device, &device_list_head, list) {
        if (device->devno == devno) {
            return device;
        }
    }
    return NULL;
}

/**
 * device_io_null - 空的操作
 * 
 * 返回成功，表示啥也不做
 */
int device_io_null()
{
    return 0;
}

/**
 * device_io_error - 错误的操作
 * 
 * 返回失败
 */
int device_io_error()
{
    return -1;
}

/**
 * dev_read - 设备的读取操作
 * @devno: 设备id号
 * @buffer: 缓冲区
 * @count: 扇区数
 */
int dev_read(dev_t devno, off_t off, void *buffer, size_t count)
{
    device_t *device;
    int retval = -1;

    /* 检测是否是坏设备 */
    if (is_bad_device(devno))
        return -1;

    device = get_device_by_id(devno);
    
    if (device == NULL)
        return -1;

    /* 如果传入的ID和注册的不一致就直接返回(用于检测没有注册但是使用) */
    if (devno != device->devno)
        return -1;

    /* 没有打开设备就直接返回 */
    if (!atomic_get(&device->references))
        return -1;
    if (device->ops->read != NULL)
        retval = (*device->ops->read)(device, off, buffer, count);

    return retval;
}

/**
 * dev_write - 设备的写入操作
 * @devno: 设备id号
 * @buffer: 缓冲区
 * @count: 字节数
 */
int dev_write(dev_t devno, off_t off, void *buffer, size_t count)
{
    device_t *device;
    int retval = -1;

    /* 检测是否是坏设备 */
    if (is_bad_device(devno))
        return -1;

    device = get_device_by_id(devno);
    
    if (device == NULL)
        return -1;

    /* 如果传入的ID和注册的不一致就直接返回(用于检测没有注册但是使用) */
    if (devno != device->devno)
        return -1;
    /* 没有打开设备就直接返回 */
    if (!atomic_get(&device->references))
        return -1;
    if (device->ops->write != NULL) {
        retval = (*device->ops->write)(device, off, buffer, count);
    }
    return retval;
}

/**
 * dev_ioctl - 设备的控制操作
 * @devno: 设备id号
 * @cmd: 命令
 * @arg: 参数1
 * 
 */
int dev_ioctl(dev_t devno, unsigned int cmd, unsigned long arg)
{
    device_t *device;
    int retval = -1;

    /* 检测是否是坏设备 */
    if (is_bad_device(devno))
        return -1;

    device = get_device_by_id(devno);

    if (device == NULL)
        return -1;

    /* 如果传入的ID和注册的不一致就直接返回(用于检测没有注册但是使用) */
    if (devno != device->devno)
        return -1;
    
    /* 没有打开设备就直接返回 */
    if (!atomic_get(&device->references))
        return -1;
    if (device->ops->ioctl != NULL)
        retval = (*device->ops->ioctl)(device, cmd, arg);

    return retval;
}

/**
 * dev_open - 打开设备
 * @devno: 设备id号
 * @name: 设备名
 * @mode: 模式
 * 
 */
int dev_open(dev_t devno, flags_t flags)
{
    device_t *device;
    int retval = 0;

    /* 检测是否是坏设备 */
    if (is_bad_device(devno)) {
        printk("bad devno\n");
        return -1;
    }
        
    device = get_device_by_id(devno);

    if (device == NULL) {
        printk("get null device by id\n");
        return -1;
    }
        
    /* 如果传入的ID和注册的不一致就直接返回(用于检测没有注册但是使用) */
    if (devno != device->devno) {
        printk("different devno\n");
        return -1;
    }
    
    /* 增加引用 */
    if (atomic_get(&device->references) >= 0)
        atomic_inc(&device->references);
    else {
        printk("%d ref error!\n", atomic_get(&device->references));
        return -1;  /* 引用计数有错误 */
    }

    //printk(">>> open dev %x, ref %d\n", device->devno, atomic_get(&device->references));
    /* 是第一次引用才打开 */
    if (atomic_get(&device->references) == 1) {
        if (device->ops->open != NULL)
            retval = (*device->ops->open)(device, flags);
    }
    
    return retval;
}


/**
 * dev_grow - 设备生长
 * @devno: 设备id号
 * 
 * 增加一个对设备的引用
 * 
 */
int dev_grow(dev_t devno)
{
    device_t *device;
    
    /* 检测是否是坏设备 */
    if (is_bad_device(devno)) {
        printk("bad devno\n");
        return -1;
    }
    
    device = get_device_by_id(devno);

    if (device == NULL) {
        printk("get null device by id\n");
        return -1;
    }
        
    /* 如果传入的ID和注册的不一致就直接返回(用于检测没有注册但是使用) */
    if (devno != device->devno) {
        printk("different devno\n");
        return -1;
    }
    //printk(KERN_DEBUG "dev_grow: dev=%x ref=%d\n", devno, atomic_get(&device->references));
    /* 增加引用 */
    if (atomic_get(&device->references) >= 0)
        atomic_inc(&device->references);
    
    return 0;
}

/**
 * dev_close - 关闭设备
 */
int dev_close(dev_t devno)
{
    device_t *device;
    int retval = 0;

    /* 检测是否是坏设备 */
    if (is_bad_device(devno))
        return -1;

    device = get_device_by_id(devno);

    if (device == NULL)
        return -1;

    //printk(">>> close dev %x\n", device->devno);
    
    /* 如果传入的ID和注册的不一致就直接返回(用于检测没有注册但是使用) */
    if (devno != device->devno)
        return -1;
    /* 增加引用 */
    if (atomic_get(&device->references) > 0)
        atomic_dec(&device->references);
    else 
        return -1;  /* 引用计数有错误 */
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "dev_close: devno=%x ref=%d\n", 
        devno, atomic_get(&device->references));
#endif    
    /* 是最后一次引用才关闭 */
    if (atomic_get(&device->references) == 0) {
        if (device->ops->close != NULL)
            retval = (*device->ops->close)(device);
        
    }
     
    return retval;
}

/**
 * dev_getc - 设备获取一个字符
 * @devno: 设备id号
 */
int dev_getc(dev_t devno, unsigned long *ch)
{
    device_t *device;
    int retval = -1;

    /* 检测是否是坏设备 */
    if (is_bad_device(devno))
        return -1;

    device = get_device_by_id(devno);
    
    if (device == NULL)
        return -1;

    /* 如果传入的ID和注册的不一致就直接返回(用于检测没有注册但是使用) */
    if (devno != device->devno)
        return -1;

    /* 没有打开设备就直接返回 */
    if (!atomic_get(&device->references))
        return -1;
    if (device->ops->getc != NULL)
        retval = (*device->ops->getc)(device, ch);

    return retval;
}

/**
 * dev_putc - 向设备输入一个字符
 * @devno: 设备id号
 * @ch: 字符
 */
int dev_putc(dev_t devno, unsigned long ch)
{
    device_t *device;
    int retval = -1;

    /* 检测是否是坏设备 */
    if (is_bad_device(devno))
        return -1;

    device = get_device_by_id(devno);
    
    if (device == NULL)
        return -1;

    /* 如果传入的ID和注册的不一致就直接返回(用于检测没有注册但是使用) */
    if (devno != device->devno)
        return -1;
    /* 没有打开设备就直接返回 */
    if (!atomic_get(&device->references))
        return -1;
    if (device->ops->putc != NULL)
        retval = (*device->ops->putc)(device, ch);

    return retval;
}

/**
 * register_device - register device to kernel 
 * @dev: device ptr
 * @name: device name
 * @ops: device operations
 * @type: device type
 * @local: local data in driver
 */
int register_device(device_t *dev, char *name, device_type_t type, void *local)
{
    /* check arguments */
    if (dev == NULL || name == NULL || type == DEVTP_UNKNOWN)
        return -1;
    memset(dev->name, 0, DEVICE_NAME_LEN);
    strcpy(dev->name, name);

    dev->type = type;
    dev->local = local;
    
    /* add to device list */
    list_add_tail(&dev->list, &device_list_head);
    
    return 0;
}

/**
 * unregister_device - unregister device from kernel
 * @dev: device ptr
 * 
 * return: 0 SUCESS, -1 FAILED
 */
int unregister_device(device_t *dev)
{
    /* check arguments */
    if (dev == NULL)
        return -1;
    
    /* get dev again to make sure it on the list */
    device_t *_dev = get_device_by_id(dev->devno);
    if (_dev == NULL)
        return -1;

    /* del device from list */
    list_del(&dev->list);
    
    return 0;
}
