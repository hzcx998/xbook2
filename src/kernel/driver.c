#include <xbook/driver.h>
#include <xbook/math.h>
#include <xbook/vine.h>
#include <xbook/debug.h>
#include <xbook/assert.h>
#include <xbook/mdl.h>

#define DEBUG_LOCAL 0

/* 驱动链表头 */
LIST_HEAD(driver_list_head);

/* 驱动藤蔓表 */
driver_func_t driver_vine_table[] = {
    serial_driver_vine,
};

/* 打开的设备表 */
device_object_t *device_handle_table[DEVICE_HANDLE_NR];

/* 维护驱动的锁 */
DEFINE_SPIN_LOCK_UNLOCKED(driver_lock);

iostatus_t default_device_dispatch(device_object_t *device, io_request_t *ioreq)
{
    ioreq->io_status.infomation = 0;
    ioreq->io_status.status = IO_SUCCESS;
    io_complete_request(ioreq);
    return IO_SUCCESS;  /* 默认是表示执行成功 */
}

static void print_drivers()
{
    driver_object_t *drvobj;
    device_object_t *devobj;
    int device_count;
    printk(KERN_INFO "io system info-> drivers\n");
    /* 遍历所有的驱动 */
    list_for_each_owner (drvobj, &driver_list_head, list) {
        printk(KERN_INFO "driver: name=%s\n", drvobj->name.text);
        device_count = 0;
        list_for_each_owner (devobj, &drvobj->device_list, list) {
            printk(KERN_INFO "        device: name=%s\n", devobj->name.text);
            device_count++;
        }
        printk(KERN_INFO "        device: count=%d\n", device_count);
    }
}


static driver_object_t *io_search_driver_by_name(char *drvname)
{
    driver_object_t *drvobj;
    spin_lock(&driver_lock);
    /* 遍历所有的驱动 */
    list_for_each_owner (drvobj, &driver_list_head, list) {
        if (!strcmp(drvobj->name.text, drvname)) {
            spin_unlock(&driver_lock);
            return drvobj;
        }
    }
    spin_unlock(&driver_lock);
    return NULL;
}

/**
 * device_handle_table_search_by_name - 通过名字来搜索一个设备句柄
 * @name: 设备名
 * 
 * @return: 成功返回设备指针，失败返回NULL
 */
device_object_t *device_handle_table_search_by_name(char *name)
{
    device_object_t *devobj;
    int i;
    spin_lock(&driver_lock);
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        devobj = device_handle_table[i];
        if (devobj != NULL) {
            if (!strcmp(devobj->name.text, name)) {
                spin_unlock(&driver_lock);
                return devobj;
            }
        }
    }
    spin_unlock(&driver_lock);
    return NULL;
}

/**
 * device_handle_table_insert - 把一个设备插入到设备句柄表
 * @devobj: 设备
 * 
 * @return: 成功返回句柄，失败返回-1
 */
int device_handle_table_insert(device_object_t *devobj)
{
    device_object_t **_devobj;
    spin_lock(&devobj->device_lock);
    int i;
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        _devobj = &device_handle_table[i];
        if (*_devobj == NULL) {  /* 表项如果空闲，就插入 */
            *_devobj = devobj;
            spin_unlock(&devobj->device_lock);
            return i;
        }
    }
    
    spin_unlock(&devobj->device_lock);
    return -1;
}

/**
 * device_handle_table_remove - 把一个设备从设备句柄表中删除
 * @devobj: 设备
 * 
 * @return: 成功返回0，失败返回-1
 */
int device_handle_table_remove(device_object_t *devobj)
{
    device_object_t **_devobj;
    spin_lock(&devobj->device_lock);
    int i;
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        _devobj = &device_handle_table[i];
        if (*_devobj) {  /* 表项如果空闲，就插入 */
            if (!strcmp((*_devobj)->name.text, devobj->name.text) &&
                (*_devobj)->type == devobj->type) {
                *_devobj = NULL; /* 设置为NULL，表示删除 */
                
                spin_unlock(&devobj->device_lock);
                return 0;
            }
        }
    }

    spin_unlock(&devobj->device_lock);
    return -1;
}

/**
 * io_search_device_by_name - 通过名字来搜索一个设备
 * @name: 设备名
 * 
 * @return: 成功返回设备指针，失败返回NULL
 */
device_object_t *io_search_device_by_name(char *name)
{
    driver_object_t *drvobj;
    device_object_t *devobj;
          
    spin_lock(&driver_lock);

    /* 先到表里面查找，没有找到就去驱动链表查找 */
    devobj = device_handle_table_search_by_name(name);
    if (devobj) {
        spin_unlock(&driver_lock);
        return devobj;
    }
        
    /* 遍历所有的驱动 */
    list_for_each_owner (drvobj, &driver_list_head, list) {
        list_for_each_owner (devobj, &drvobj->device_list, list) {
            if (!strcmp(devobj->name.text, name)) {
                spin_unlock(&driver_lock);
                return devobj;
            }
        }
    }
    spin_unlock(&driver_lock);
    return NULL;
}

void driver_object_init(driver_object_t *driver)
{
    INIT_LIST_HEAD(&driver->device_list);
    INIT_LIST_HEAD(&driver->list);
    driver->drver_extension = NULL;
    driver->driver_enter = NULL;
    driver->driver_exit = NULL;
    
    int i;
    for (i = 0; i < MAX_IOREQ_FUNCTION_NR; i++) {
        driver->dispatch_function[i] = default_device_dispatch;
    }
    string_init(&driver->name);

    spinlock_init(&driver->device_lock);
}

/**
 * driver_object_create - 根据藤蔓创建驱动
 * @vine: 藤蔓
 * 
 * 藤蔓是指向一个驱动的一个函数指针，通过调用这个指针来创建一个驱动
 * 
 * @return: 成功返回0，失败返回-1
 */
int driver_object_create(driver_func_t vine)
{
    driver_object_t *drvobj;
    iostatus_t status;

    drvobj = kmalloc(sizeof(driver_object_t));
    if (drvobj == NULL)
        return -1;
    /* 初始化驱动对象 */
    driver_object_init(drvobj);
    status = vine(drvobj);
    if (status != IO_SUCCESS) {
        kfree(drvobj); /* 释放驱动对象 */
        return -1;
    }
    unsigned long flags;        
    spin_lock_irqsave(&driver_lock, flags);
    /* 执行驱动进入部分 */
    if (drvobj->driver_enter)
        status = drvobj->driver_enter(drvobj); 

    if (status != IO_SUCCESS) {
        kfree(drvobj); /* 释放驱动对象 */
        spin_unlock_irqrestore(&driver_lock, flags);
        return -1;
    }
    
    /* 将驱动添加到系统中 */
    ASSERT(!list_find(&drvobj->list, &driver_list_head));
    list_add_tail(&drvobj->list, &driver_list_head);
    spin_unlock_irqrestore(&driver_lock, flags);

    return 0;
}

int driver_object_delete(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
#if DEBUG_LOCAL == 1    
    printk(KERN_DEBUG "driver_object_delete: driver %s delete start.\n",
        driver->name.text);
#endif
    unsigned long flags;        
    spin_lock_irqsave(&driver_lock, flags);
    /* 执行驱动进入部分 */
    if (driver->driver_exit)
        status = driver->driver_exit(driver); 
    
    if (status != IO_SUCCESS) {
        spin_unlock_irqrestore(&driver_lock, flags);
        return -1;
    }
    
    /* 将驱动添加到系统中 */
    ASSERT(list_find(&driver->list, &driver_list_head));
    list_del(&driver->list);
    spin_unlock_irqrestore(&driver_lock, flags);

    /* 释放掉驱动对象 */
    kfree(driver);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "driver_object_delete: driver delete done.\n");
#endif

    return status;
}

/**
 * io_create_device - 创建一个设备
 * 
 * 在驱动里面创建一个设备。
 * 注意：device参数是一个需要传回的设备指针
 * 
 * @return: 成功返回IO_SUCCESS，失败返回IO_FAILED
 */
iostatus_t io_create_device(
    driver_object_t *driver,
    unsigned long device_extension_size,
    char *device_name,
    _device_type_t type,
    device_object_t **device
) {
    device_object_t *devobj;

    devobj = kmalloc(sizeof(device_object_t) + device_extension_size);
    if (devobj == NULL)
        return IO_FAILED;
    
    /* 填写设备信息 */
    INIT_LIST_HEAD(&devobj->list);
    devobj->type = type;
    if (device_extension_size > 0)
        devobj->device_extension = (void *) (devobj + 1); /* 设备扩展的空间位于设备末尾 */
    else /* 没有扩展就指向NULL */
        devobj->device_extension = NULL;
    devobj->flags = 0;
    atomic_set(&devobj->reference, 0);
    devobj->cur_ioreq = NULL;
    devobj->reserved = 0;
    /* 如果创建字符串失败，就返回 */
    if (string_new(&devobj->name, device_name, DEVICE_NAME_LEN)) {
        kfree(devobj);
        return IO_FAILED;
    }
    devobj->driver = driver; /* 绑定驱动 */
    spinlock_init(&devobj->device_lock);    /* 初始化设备锁 */
    /* 初始化设备队列 */
    devobj->device_queue.busy = false;
    spinlock_init(&devobj->device_queue.lock);
    INIT_LIST_HEAD(&devobj->device_queue.list_head);

    spin_lock(&driver->device_lock);
    /* 设备创建好后，添加到驱动链表上 */
    ASSERT(!list_find(&devobj->list, &driver->device_list));
    list_add_tail(&devobj->list, &driver->device_list);
    spin_unlock(&driver->device_lock);
    
    /* 把设备地址保存到device里面 */
    *device = devobj;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "io_create_device: create device done.\n");
#endif
    return IO_SUCCESS;
}

/**
 * io_delete_device - 删除一个设备
 */
void io_delete_device(
    device_object_t *device
) {
    if (device == NULL)
        return;
    device_object_t *devobj;
    /* 查看是否还在使用中 */
    devobj = device_handle_table_search_by_name(device->name.text);
    if (devobj) { /* 在使用中就从句柄表中删除 */
        printk(KERN_DEBUG "io_delete_device: device %s is using!\n", 
            devobj->name.text);
        device_handle_table_remove(devobj);
    }
    devobj = device;

    driver_object_t *driver = device->driver;

    spin_lock(&driver->device_lock);
    /* 先从驱动链表中删除 */
    ASSERT(list_find(&devobj->list, &driver->device_list));
    list_del(&devobj->list);
    spin_unlock(&driver->device_lock);
    
    string_del(&devobj->name); /* 释放名字 */
    /* 释放设备对象 */
    kfree(devobj);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "io_delete_device: delete done!\n");
#endif    
}

io_request_t *io_request_alloc()
{
    io_request_t *ioreq = kmalloc(sizeof(io_request_t));
    if (ioreq)
        memset(ioreq, 0, sizeof(io_request_t));
    return ioreq;
}

void io_request_free(io_request_t *ioreq)
{
    kfree(ioreq);    
}

iostatus_t io_call_dirver(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status= IO_SUCCESS;

    driver_dispatch_t func = NULL;
    spin_lock(&device->device_lock);

    /* 选择操作函数 */
    if (ioreq->flags & IOREQ_OPEN_OPERATION) {
        func = device->driver->dispatch_function[IOREQ_OPEN];
    } else if (ioreq->flags & IOREQ_CLOSE_OPERATION) {
        func = device->driver->dispatch_function[IOREQ_CLOSE];
    } else if (ioreq->flags & IOREQ_READ_OPERATION) {
        func = device->driver->dispatch_function[IOREQ_READ];
    } else if (ioreq->flags & IOREQ_WRITE_OPERATION) {
        func = device->driver->dispatch_function[IOREQ_WRITE];
    } else if (ioreq->flags & IOREQ_DEVCTL_OPERATION) {
        func = device->driver->dispatch_function[IOREQ_DEVCTL];
    }
    if (func)
        status = func(device, ioreq); /* 执行操作 */
    
    return status;
}

io_request_t *io_build_sync_request(
    unsigned long function,
    device_object_t *devobj,
    void *buffer,
    unsigned long length,
    unsigned long offset,
    io_status_block_t *io_status_block
){
    io_request_t *ioreq = io_request_alloc();
    if (ioreq == NULL) {
        io_status_block->status = IO_FAILED;
        return NULL;
    }
    
    ioreq->devobj = devobj;

    /* 设置请求的状态 */
    if (io_status_block) {
        ioreq->io_status = *io_status_block;
    } else {
        ioreq->io_status.infomation = 0;
        ioreq->io_status.status = IO_FAILED;
    }
    INIT_LIST_HEAD(&ioreq->list);
    ioreq->system_buffer = NULL;
    ioreq->user_buffer = buffer; /* 指向用户地址 */
    ioreq->mdl_address = NULL;
    if (buffer) {    
        if (devobj->flags & DO_BUFFERED_IO) {
            /* 分配一个内存缓冲区 */
            if (length >= MAX_MEM_CACHE_SIZE) {
                length = MAX_MEM_CACHE_SIZE; /* 调整大小 */
                printk(KERN_WARING "io_build_sync_request: length too big!\n");
            }
            ioreq->system_buffer = kmalloc(length);
            if (ioreq->system_buffer == NULL) {
                kfree(ioreq);
                return NULL;
            }
            ioreq->flags |= IOREQ_BUFFERED_IO;
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "io_build_sync_request: system buffer.\n");
#endif
        } else if (devobj->flags & DO_DIRECT_IO) {
            ioreq->mdl_address = mdl_alloc(buffer, length, FALSE, ioreq);
            if (ioreq->mdl_address == NULL) {
                kfree(ioreq);
                return NULL;    
            }
            /* 分配内存描述列表 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "io_build_sync_request: direct buffer.\n");
#endif
        } else {    /* 直接使用用户地址 */
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "io_build_sync_request: user buffer.\n");
#endif
        }
    }
    unsigned long flags;

    switch (function)
    {
    case IOREQ_OPEN:
        ioreq->flags |= IOREQ_OPEN_OPERATION;
        ioreq->parame.open.devname = NULL;
        ioreq->parame.open.flags = 0;
        break;
    case IOREQ_CLOSE:
        ioreq->flags |= IOREQ_CLOSE_OPERATION;
        break;
    case IOREQ_READ:
        ioreq->flags |= IOREQ_READ_OPERATION;
        ioreq->parame.read.length = length;
        ioreq->parame.read.offset = offset;
        break;
    case IOREQ_WRITE:
        ioreq->flags |= IOREQ_WRITE_OPERATION;
        ioreq->parame.write.length = length;
        ioreq->parame.write.offset = offset;
        /* 把数据复制到内核缓冲区，不允许产生中断 */
        if (devobj->flags & DO_BUFFERED_IO) {
            save_intr(flags);
            memcpy(ioreq->system_buffer, buffer, length);
            restore_intr(flags);
        }
        break;
    case IOREQ_DEVCTL:
        ioreq->flags |= IOREQ_DEVCTL_OPERATION;
        ioreq->parame.devctl.code = 0;
        ioreq->parame.devctl.arg = 0;
        break;
    default:
        break;
    }
    return ioreq;
}

void io_complete_request(io_request_t *ioreq)
{
    ioreq->flags |= IOREQ_COMPLETION; /* 添加完成标志 */
    spin_unlock(&ioreq->devobj->device_lock);    
}

static int io_complete_check(io_request_t *ioreq, iostatus_t status)
{
    if (status == IO_SUCCESS) { /* 执行成功 */
        if (ioreq->io_status.status == IO_SUCCESS && 
            ioreq->flags & IOREQ_COMPLETION) {    /* 请求完成 */
            return 0;
        }
    }
    return -1;
}

/**
 * device_open - 打开设备操作
 * @devname: 设备名
 * @flags: 设备的标志
 * 
 * @return: 成功返回设备的handle，失败返回-1
 */
handle_t device_open(char *devname, unsigned int flags)
{
    if (devname == NULL)
        return -1;
    
    device_object_t *devobj;
    
    /* 搜索设备 */
    devobj = io_search_device_by_name(devname);
    if (devobj == NULL) {
        printk(KERN_ERR "device_open: device %s not found!\n", devname);
        return -1;
    }
    handle_t handle = device_handle_table_insert(devobj);
    if (handle == -1) {
        printk(KERN_ERR "device_open: insert device handle tabel failed!\n");
        return -1;  /* 插入句柄表失败 */    
    }
    spin_lock(&devobj->device_lock);

    /* 增加引用 */
    if (atomic_get(&devobj->reference) >= 0)
        atomic_inc(&devobj->reference);
    else {
        printk(KERN_ERR "device_open: reference %d error!\n", atomic_get(&devobj->reference));
        goto rollback_handle;  /* 引用计数有错误 */
    }
    
    iostatus_t status = IO_SUCCESS;
    io_request_t *ioreq = NULL;
    
    /* 引用计数为1，才真正打开 */
    if (atomic_get(&devobj->reference) == 1) {
        ioreq = io_build_sync_request(IOREQ_OPEN, devobj, NULL, 0, 0, NULL);
        if (ioreq == NULL) {
            printk(KERN_ERR "device_open: alloc io request packet failed!\n", atomic_get(&devobj->reference));
            goto rollback_ref;
        }
        ioreq->parame.open.devname = devname;
        ioreq->parame.open.flags = flags;
        spin_unlock(&devobj->device_lock);

        status = io_call_dirver(devobj, ioreq);

        spin_lock(&devobj->device_lock);
    }

    if (!io_complete_check(ioreq, status)) {
        io_request_free((ioreq));
        spin_unlock(&devobj->device_lock);
        return handle;
    }
    
    printk(KERN_ERR "device_open: do dispatch failed!\n");
/* rollback_ioreq */
    io_request_free(ioreq);
rollback_ref:
    atomic_dec(&devobj->reference);
rollback_handle:
    device_handle_table_remove(devobj);
    spin_unlock(&devobj->device_lock);
    
    return -1;
}

/**
 * device_close - 关闭设备操作
 * @handle: 设备句柄
 * 
 * @return: 成功返回0，失败返回-1
 */
int device_close(handle_t handle)
{
    if (IS_BAD_DEVICE_HANDLE(handle))
        return -1;
    
    device_object_t *devobj;
    
    devobj = GET_DEVICE_BY_HANDLE(handle);

    /* 获取设备 */
    if (devobj == NULL) {
        printk(KERN_ERR "device_close: device object error by handle=%d!\n", handle);
        /* 应该激活一个触发器，让调用者停止运行 */
        return -1;
    }
    
    if (device_handle_table_remove(devobj)) {
        printk(KERN_ERR "device_close: device=%s remove from device handle table failed!\n",
            devobj->name.text);
        return -1;
    }
    spin_lock(&devobj->device_lock);
    
    /* 减少引用 */
    if (atomic_get(&devobj->reference) >= 0)
        atomic_dec(&devobj->reference);
    else {
        printk(KERN_ERR "device_close: reference %d error!\n", atomic_get(&devobj->reference));
        goto scrollback_handle;    
    }
    
    iostatus_t status = IO_SUCCESS;
    io_request_t *ioreq = NULL;

    if (!atomic_get(&devobj->reference)) { /* 最后一次关闭才关闭 */    
        ioreq = io_build_sync_request(IOREQ_CLOSE, devobj, NULL, 0, 0, NULL);
        if (ioreq == NULL) {
            printk(KERN_ERR "device_close: alloc io request packet failed!\n", atomic_get(&devobj->reference));
            goto rollback_ref;
        }
        spin_unlock(&devobj->device_lock);
        status = io_call_dirver(devobj, ioreq);
        spin_lock(&devobj->device_lock);
    }

    if (!io_complete_check(ioreq, status)) {
        io_request_free((ioreq));
        spin_unlock(&devobj->device_lock);
        return 0;
    }
    printk(KERN_ERR "device_close: do dispatch failed!\n");
/* rollback_ioreq */
    io_request_free(ioreq);
rollback_ref:
    atomic_inc(&devobj->reference);
scrollback_handle:
    device_handle_table_insert(devobj);
    spin_unlock(&devobj->device_lock);
    return -1;
}

/**
 * device_read - 从设备读取数据
 * @handle: 设备句柄
 * @buffer: 缓冲区
 * @size: 数据长度
 * @offset: 偏移位置
 * 
 * @return: 成功返回读取的数据量，失败返回-1
 */
ssize_t device_read(handle_t handle, void *buffer, size_t length, off_t offset)
{
    if (IS_BAD_DEVICE_HANDLE(handle) || buffer == NULL || !length)
        return -1;
    
    device_object_t *devobj;
    
    devobj = GET_DEVICE_BY_HANDLE(handle);
    /* 获取设备 */
    if (devobj == NULL) {
        printk(KERN_ERR "device_read: device object error by handle=%d!\n", handle);
        /* 应该激活一个触发器，让调用者停止运行 */
        return -1;
    }
    
    iostatus_t status = IO_SUCCESS;
    io_request_t *ioreq = NULL;
    ioreq = io_build_sync_request(IOREQ_READ, devobj, buffer, length, offset, NULL);
    if (ioreq == NULL) {
        printk(KERN_ERR "device_read: alloc io request packet failed!\n");
        return -1;
    }
    status = io_call_dirver(devobj, ioreq);

    if (!io_complete_check(ioreq, status)) {
        unsigned int len = ioreq->io_status.infomation;
        /* 如果是内存缓冲区，就需要从内核缓冲区复制到用户空间 */
        if (devobj->flags & DO_BUFFERED_IO) { 
            /* 复制数据到用户空间 */
            unsigned long flags;
            save_intr(flags);
            memcpy(ioreq->user_buffer, ioreq->system_buffer, len);
            restore_intr(flags);
            kfree(ioreq->system_buffer);
        } else if (devobj->flags & DO_DIRECT_IO) { 
            /* 删除映射 */
            printk(KERN_DEBUG "device_read: read done. free mdl.\n");
            mdl_free(ioreq->mdl_address);
            ioreq->mdl_address = NULL;
        }
        io_request_free((ioreq));
        return len;
    }

    printk(KERN_ERR "device_read: do dispatch failed!\n");
/* rollback_ioreq */
    io_request_free(ioreq);
    return -1;
}


/**
 * device_write - 往设备写入数据
 * @handle: 设备句柄
 * @buffer: 缓冲区
 * @size: 数据长度
 * @offset: 偏移位置
 * 
 * @return: 成功写入读取的数据量，失败返回-1
 */
ssize_t device_write(handle_t handle, void *buffer, size_t length, off_t offset)
{
    if (IS_BAD_DEVICE_HANDLE(handle) || buffer == NULL || !length)
        return -1;
    
    device_object_t *devobj;
    
    devobj = GET_DEVICE_BY_HANDLE(handle);
    /* 获取设备 */
    if (devobj == NULL) {
        printk(KERN_ERR "device_write: device object error by handle=%d!\n", handle);
        /* 应该激活一个触发器，让调用者停止运行 */
        return -1;
    }

    iostatus_t status = IO_SUCCESS;
    io_request_t *ioreq = NULL;
    ioreq = io_build_sync_request(IOREQ_WRITE, devobj, buffer, length, offset, NULL);
    if (ioreq == NULL) {
        printk(KERN_ERR "device_write: alloc io request packet failed!\n");
        return -1;
    }
    status = io_call_dirver(devobj, ioreq);

    if (!io_complete_check(ioreq, status)) {
        if (devobj->flags & DO_DIRECT_IO) { 
            /* 删除映射 */
            printk(KERN_DEBUG "device_write: write done. free mdl.\n");
            mdl_free(ioreq->mdl_address);
            ioreq->mdl_address = NULL;
        }
        unsigned int len = ioreq->io_status.infomation;
        io_request_free((ioreq));
        return len;
    }

    printk(KERN_ERR "device_write: do dispatch failed!\n");
/* rollback_ioreq */
    io_request_free(ioreq);
    return -1;
}


/**
 * device_devctl - 往设备写入数据
 * @handle: 设备句柄
 * @buffer: 缓冲区
 * @size: 数据长度
 * @offset: 偏移位置
 * 
 * @return: 成功写入读取的数据量，失败返回-1
 */
ssize_t device_devctl(handle_t handle, unsigned int code, unsigned long arg)
{
    if (IS_BAD_DEVICE_HANDLE(handle))
        return -1;
    
    device_object_t *devobj;
    
    devobj = GET_DEVICE_BY_HANDLE(handle);
    /* 获取设备 */
    if (devobj == NULL) {
        printk(KERN_ERR "device_devctl: device object error by handle=%d!\n", handle);
        /* 应该激活一个触发器，让调用者停止运行 */
        return -1;
    }

    iostatus_t status = IO_SUCCESS;
    io_request_t *ioreq = NULL;
    ioreq = io_build_sync_request(IOREQ_DEVCTL, devobj, NULL, 0, 0, NULL);
    if (ioreq == NULL) {
        printk(KERN_ERR "device_devctl: alloc io request packet failed!\n");
        return -1;
    }
    ioreq->parame.devctl.code = code;
    ioreq->parame.devctl.arg = arg;
    
    status = io_call_dirver(devobj, ioreq);
    if (!io_complete_check(ioreq, status)) {
        unsigned int infomation = ioreq->io_status.infomation;
        io_request_free((ioreq));
        return infomation;
    }

    printk(KERN_ERR "device_devctl: do dispatch failed!\n");
/* rollback_ioreq */
    io_request_free(ioreq);
    return -1;
}

int io_uninstall_driver(char *drvname)
{
    driver_object_t *drvobj;
    drvobj = io_search_driver_by_name(drvname);
    if (!drvobj)
        return -1;
    if (driver_object_delete(drvobj)) {
        printk(KERN_ERR "io_uninstall_driver: delete driver %s failed!\n", drvname);
    }
    return 0;
}

void dump_device_object(device_object_t *device)
{
    printk(KERN_DEBUG "dump_device_object: type=%d driver=%x extension=%x flags=%x reference=%x name=%s\n",
        device->type, device->driver, device->device_extension, device->flags,
        atomic_get(&device->reference), device->name.text);
}

/* 初始化驱动架构 */
void init_driver_arch()
{
    driver_func_t vine;
    
    int i;
    for (i = 0; i < ARRAY_SIZE(driver_vine_table); i++) {
        vine = driver_vine_table[i];
        if (driver_object_create(vine))
            printk(KERN_ERR "init_driver_arch: create one driver failed!\n");
    }

    /* 输出所有驱动以及设备 */
    print_drivers();
    
    /* 初始化设备句柄表 */
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        device_handle_table[i] = NULL;
    }
    
    handle_t handle = device_open("com0", 123);
    printk(KERN_DEBUG "init_driver_arch: open device handle=%d\n", handle);
    if (handle >= 0)
        dump_device_object(GET_DEVICE_BY_HANDLE(handle));
    handle_t handle1 = device_open("com1", 123);
    printk(KERN_DEBUG "init_driver_arch: open device handle=%d\n", handle);
    if (handle1 >= 0)
        dump_device_object(GET_DEVICE_BY_HANDLE(handle1));
    
    #if 0
    char *big_buf = kmalloc(0x20000);
    if (big_buf == NULL)
        panic("init_driver_arch: kmalloc for big buf failed!\n");    
    
    //char buf[12];
    ssize_t read = device_read(handle, big_buf, 1, 0);
    printk(KERN_DEBUG "init_driver_arch: read %d bytes.\n", read);
    if (read > 0)
        dump_buffer(big_buf, 32, 1);
    
    memset(big_buf, 0x5a, 0x20000);
    ssize_t write = device_write(handle, "hello, com0", 10, 0);
    printk(KERN_DEBUG "init_driver_arch: write %d bytes.\n", write);
    
    ssize_t devctl = device_devctl(handle, DEVCTL_CODE_TEST, 0x1234abcd);
    
    #endif
    while (1)
    {
        device_write(handle, "hello, com0\n", 12, 0);
        device_write(handle1, "hello, com1\n", 12, 0);
        
    }
    
    
    iostatus_t status;
    status = device_close(handle);
    printk(KERN_DEBUG "init_driver_arch: close devce ihandle=%d status=%d\n", handle, status);
    io_uninstall_driver("uart-serial");
    
    print_drivers();
}
