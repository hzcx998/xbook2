#include <xbook/driver.h>
#include <math.h>

#include <xbook/debug.h>
#include <assert.h>
#include <xbook/mdl.h>
#include <xbook/clock.h>
#include <string.h>
#include <xbook/vmspace.h>
#include <sys/ioctl.h>
#include <xbook/config.h>
#include <xbook/vmarea.h>
#include <xbook/schedule.h>

// #define DEBUG_DRIVER

/* 驱动链表头 */
LIST_HEAD(driver_list_head);

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
#ifdef DEBUG_DRIVER /* print devices */
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

static void print_drivers_mini()
{
    driver_object_t *drvobj;
    device_object_t *devobj;
    int device_count;
    printk(KERN_INFO "io system info-> drivers\n");
    /* 遍历所有的驱动 */
    list_for_each_owner (drvobj, &driver_list_head, list) {
        list_for_each_owner (devobj, &drvobj->device_list, list) {
            printk("%s ", devobj->name.text);
        }
    }
    printk("\n");
}
#endif
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
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        devobj = device_handle_table[i];
        if (devobj != NULL) {
            if (!strcmp(devobj->name.text, name)) {
                return devobj;
            }
        }
    }
    return NULL;
}


/**
 * device_handle_find_by_object - 通过设备来搜索一个设备句柄
 * @devobj: 设备对象
 * 
 * @return: 成功返回句柄，失败返回-1
 */
handle_t device_handle_find_by_object(device_object_t *devobj)
{
    device_object_t *_devobj;
    int i;
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        _devobj = device_handle_table[i];
        if (_devobj == devobj) {
            return i;
        }
    }
    return -1;
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
    int i;
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        _devobj = &device_handle_table[i];
        if (*_devobj == NULL) {  /* 表项如果空闲，就插入 */
            *_devobj = devobj;
            return i;
        }
    }
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
    int i;
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        _devobj = &device_handle_table[i];
        if (*_devobj) {  /* 表项如果空闲，就插入 */
            if (!strcmp((*_devobj)->name.text, devobj->name.text) &&
                (*_devobj)->type == devobj->type) {
                *_devobj = NULL; /* 设置为NULL，表示删除 */
                return 0;
            }
        }
    }
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
 * driver_object_create - 根据函数地址创建驱动
 * @func: 函数地址
 * 
 * 函数地址是指向一个驱动的一个函数指针，通过调用这个指针来创建一个驱动
 * 
 * @return: 成功返回0，失败返回-1
 */
int driver_object_create(driver_func_t func)
{
    driver_object_t *drvobj;
    iostatus_t status;

    drvobj = kmalloc(sizeof(driver_object_t));
    if (drvobj == NULL)
        return -1;
    /* 初始化驱动对象 */
    driver_object_init(drvobj);
    status = func(drvobj);
    if (status != IO_SUCCESS) {
        kfree(drvobj); /* 释放驱动对象 */
        return -1;
    }
    /* 执行驱动进入部分 */
    if (drvobj->driver_enter)
        status = drvobj->driver_enter(drvobj); 

    if (status != IO_SUCCESS) {
        kfree(drvobj); /* 释放驱动对象 */
        return -1;
    }
    unsigned long flags;        
    spin_lock_irqsave(&driver_lock, flags);
    
    /* 将驱动添加到系统中 */
    ASSERT(!list_find(&drvobj->list, &driver_list_head));
    //printk("%s: <%x %x>\n", __func__,&drvobj->list, &driver_list_head);
    list_add_tail(&drvobj->list, &driver_list_head);
    spin_unlock_irqrestore(&driver_lock, flags);

    return 0;
}

int driver_object_delete(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
#ifdef DEBUG_DRIVER    
    printk(KERN_DEBUG "driver_object_delete: driver %s delete start.\n",
        driver->name.text);
#endif
    
    /* 执行驱动进入部分 */
    if (driver->driver_exit)
        status = driver->driver_exit(driver); 
    
    if (status != IO_SUCCESS) {
        return -1;
    }
    unsigned long flags;        
    
    spin_lock_irqsave(&driver_lock, flags);
    /* 将驱动添加到系统中 */
    ASSERT(list_find(&driver->list, &driver_list_head));
    list_del(&driver->list);
    spin_unlock_irqrestore(&driver_lock, flags);

    /* 释放掉驱动对象 */
    kfree(driver);
#ifdef DEBUG_DRIVER
    printk(KERN_DEBUG "driver_object_delete: driver delete done.\n");
#endif

    return status;
}

/**
 * io_device_queue_init - 设备队列初始化
 * @queue: 队列
 */
void io_device_queue_init(device_queue_t *queue)
{
    /* 初始化设备队列 */
    spinlock_init(&queue->lock);
    INIT_LIST_HEAD(&queue->list_head);
    wait_queue_init(&queue->wait_queue);
    queue->entry_count = 0;
}


/**
 * io_search_device_by_name - 通过名字来搜索一个设备
 * @name: 设备名
 * 
 * @return: 成功返回设备指针，失败返回NULL
 */
device_object_t *io_iterative_search_device_by_type(device_object_t *devptr, device_type_t type)
{
    driver_object_t *drvobj;
    device_object_t *devobj;
    int flags = 0;  /* 标记可迭代对象 */
    spin_lock(&driver_lock);
    /* 遍历所有的驱动 */
    list_for_each_owner (drvobj, &driver_list_head, list) {
        list_for_each_owner (devobj, &drvobj->device_list, list) {
            if (devobj->type == type) { /* 设备类型 */   
                /* 搜索是第一个设备 */
                if (devptr == NULL) {
                    spin_unlock(&driver_lock);
                    return devobj;
                } else {
                    if (flags) {    /* 可以选择下一个设备，就直接返回下一个设备 */
                        spin_unlock(&driver_lock);
                        return devobj;
                    }
                    if (devptr == devobj) { /* 找到和当前设备一样的设备 */
                        flags = 1;  /* 下一次就是后面的迭代设备对象 */
                    }
                }
            }
        }
    }
    spin_unlock(&driver_lock);
    return NULL;
}


/**
 * sys_devscan - 扫描某种类型的设备
 * @de: 输入的设备项
 * @type: 设备类型
 * @out: 输出设备项
 * 
 * @return: 成功返回0，失败返回-1
 */
int sys_devscan(devent_t *de, device_type_t type, devent_t *out)
{
    if (!out)
        return -1;
    driver_object_t *drvobj;   //驱动对象
    device_object_t *devobj;   //驱动对应设备对象
    int flags = 0;  /* 标记可迭代对象 */
    spin_lock(&driver_lock);
    /* 遍历所有的驱动 */
    list_for_each_owner (drvobj, &driver_list_head, list) {
        list_for_each_owner (devobj, &drvobj->device_list, list) {   //遍历驱动下的设备
            if (devobj->type == type) { /* 设备类型 */   
                /* 搜索是第一个设备 */
                if (de == NULL) {
                    memset(out->de_name, 0, DEVICE_NAME_LEN);
                    strcpy(out->de_name, devobj->name.text);
                    out->de_type = type;
                    spin_unlock(&driver_lock);
                    return 0;
                } else {
                    if (flags) {    /* 可以选择下一个设备，就直接返回下一个设备 */
                        memset(out->de_name, 0, DEVICE_NAME_LEN);
                        strcpy(out->de_name, devobj->name.text);
                        out->de_type = type;
                        spin_unlock(&driver_lock);
                        return 0;
                    }
                    if (!strcmp(de->de_name, devobj->name.text)) { /* 找到和当前设备一样的设备 */
                        flags = 1;  /* 下一次就是后面的迭代设备对象 */
                    }
                }
            }
        }
    }
    spin_unlock(&driver_lock);
    return -1;
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
    device_type_t type,
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
    spinlock_init(&devobj->lock.spinlock);    /* 初始化设备锁-自旋锁 */
    mutexlock_init(&devobj->lock.mutexlock);  /* 初始化设备锁-互斥锁 */
    
    spin_lock(&driver->device_lock);
    /* 设备创建好后，添加到驱动链表上 */
    ASSERT(!list_find(&devobj->list, &driver->device_list));
    //printk("%s: <%x %x>\n", __func__,&devobj->list, &driver->device_list);
    list_add_tail(&devobj->list, &driver->device_list);
    spin_unlock(&driver->device_lock);
    
    /* 把设备地址保存到device里面 */
    *device = devobj;
#ifdef DEBUG_DRIVER
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
    spin_lock(&driver_lock);
    /* 查看是否还在使用中 */
    devobj = device_handle_table_search_by_name(device->name.text);
    if (devobj) { /* 在使用中就从句柄表中删除 */
        printk(KERN_DEBUG "io_delete_device: device %s is using!\n", 
            devobj->name.text);
        device_handle_table_remove(devobj);
    }
    spin_unlock(&driver_lock);
    
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

    /* 根据设备类型选择不同的锁 */
    switch (device->type)
    {
    case DEVICE_TYPE_SERIAL_PORT:
    case DEVICE_TYPE_SCREEN:
    case DEVICE_TYPE_KEYBOARD:
    case DEVICE_TYPE_MOUSE:
    case DEVICE_TYPE_VIRTUAL_CHAR:
    case DEVICE_TYPE_BEEP:
        spin_lock(&device->lock.spinlock);
        break;
    case DEVICE_TYPE_DISK:
    case DEVICE_TYPE_NETWORK:
    case DEVICE_TYPE_PHYSIC_NETCARD:
        mutex_lock(&device->lock.mutexlock);
        break;
    default:
        break;
    }
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
    } else if (ioreq->flags & IOREQ_MMAP_OPERATION) {
        func = device->driver->dispatch_function[IOREQ_MMAP];
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
#ifdef DEBUG_DRIVER
            printk(KERN_DEBUG "io_build_sync_request: system buffer.\n");
#endif
        } else if (devobj->flags & DO_DIRECT_IO) {
            ioreq->mdl_address = mdl_alloc(buffer, length, FALSE, ioreq);
            if (ioreq->mdl_address == NULL) {
                kfree(ioreq);
                return NULL;    
            }
            /* 分配内存描述列表 */
#ifdef DEBUG_DRIVER
            printk(KERN_DEBUG "io_build_sync_request: direct buffer.\n");
#endif
        } else {    /* 直接使用用户地址 */
#ifdef DEBUG_DRIVER
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
    case IOREQ_MMAP:
        ioreq->flags |= IOREQ_MMAP_OPERATION;
        ioreq->parame.mmap.flags = offset;
        ioreq->parame.mmap.length = length;
        break;
    default:
        break;
    }
    return ioreq;
}

void io_complete_request(io_request_t *ioreq)
{
    if (ioreq->io_status.status == IO_FAILED)
        ioreq->io_status.infomation = -1;
    
    ioreq->flags |= IOREQ_COMPLETION; /* 添加完成标志 */
    /* 根据设备类型选择不同的锁 */
    switch (ioreq->devobj->type)
    {
    case DEVICE_TYPE_SERIAL_PORT:
    case DEVICE_TYPE_SCREEN:
    case DEVICE_TYPE_KEYBOARD:
    case DEVICE_TYPE_MOUSE:
    case DEVICE_TYPE_VIRTUAL_CHAR:
    case DEVICE_TYPE_BEEP:
        spin_unlock(&ioreq->devobj->lock.spinlock);
        break;
    case DEVICE_TYPE_DISK:
    case DEVICE_TYPE_NETWORK:
    case DEVICE_TYPE_PHYSIC_NETCARD:
        mutex_unlock(&ioreq->devobj->lock.mutexlock);
        break;
    default:
        break;
    }
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

void io_device_queue_cleanup(device_queue_t *queue)
{
    device_queue_entry_t *entry, *next;
    unsigned long irqflags;
    spin_lock_irqsave(&queue->lock, irqflags);
    /* 由于要删除队列成员，所以需要用safe版本 */
    list_for_each_owner_safe (entry, next, &queue->list_head, list) {
        list_del(&entry->list); /* 从链表删除 */
        kfree(entry);           /* 释放空间 */
    }
    spin_unlock_irqrestore(&queue->lock, irqflags);
}

iostatus_t io_device_queue_append(device_queue_t *queue, unsigned char *buf, int len)
{
    unsigned long irqflags;
    spin_lock_irqsave(&queue->lock, irqflags);
    if (queue->entry_count > DEVICE_QUEUE_ENTRY_NR) { /* 超过队列项数，就先丢弃数据包 */
#if DEBUG_LOCLA == 1
        printk(KERN_NOTICE "io_device_queue_append: device queue full!\n");   
#endif
        spin_unlock_irqrestore(&queue->lock, irqflags);
        return IO_FAILED;
    }
    device_queue_entry_t *entry = kmalloc(sizeof(device_queue_entry_t) + len);
    if (entry == NULL) {
        spin_unlock_irqrestore(&queue->lock, irqflags);
        return IO_FAILED;
    }

    list_add_tail(&entry->list, &queue->list_head);
    queue->entry_count++;
    entry->buf = (unsigned char *) (entry + 1);
    entry->length = len;
    memcpy(entry->buf, buf, len);
    spin_unlock_irqrestore(&queue->lock, irqflags);
    wait_queue_wakeup(&queue->wait_queue);
#if DEBUG_LOCLA == 1
    printk(KERN_DEBUG "io_device_queue_put: pid=%d len=%d.\n", 
        queue->wait_queue.task->pid, len);
#endif
    return IO_SUCCESS;
}

int io_device_queue_pickup(device_queue_t *queue, unsigned char *buf, int buflen, int flags)
{
    unsigned long irqflags;

    spin_lock_irqsave(&queue->lock, irqflags);
    if (!queue->entry_count) {  /* 没有数据包 */
        if (flags & IO_NOWAIT) {    /* 不进行等待 */
            spin_unlock_irqrestore(&queue->lock, irqflags);
            return -1;    
        }
        printk("!!!block wait\n");
        wait_queue_add(&queue->wait_queue, current_task);
        spin_unlock_irqrestore(&queue->lock, irqflags);
        task_block(TASK_BLOCKED);
        spin_lock_irqsave(&queue->lock, irqflags);   
    }
    
    device_queue_entry_t *entry;
    entry = list_first_owner(&queue->list_head, device_queue_entry_t, list);
    list_del(&entry->list);
    queue->entry_count--;
    int len = MIN(entry->length, buflen);
    memcpy(buf, entry->buf, len);
    kfree(entry);
    spin_unlock_irqrestore(&queue->lock, irqflags);
#if DEBUG_LOCLA == 1
    printk(KERN_DEBUG "io_device_queue_get: pid=%d len=%d.\n",
        queue->wait_queue.task->pid, len);
#endif            
    return len;
}

iostatus_t io_device_increase_reference(device_object_t *devobj)
{
    /* 增加引用 */
    if (atomic_get(&devobj->reference) >= 0) {
        atomic_inc(&devobj->reference);
    } else {
        printk(KERN_ERR "device_open: reference %d error!\n", atomic_get(&devobj->reference));
        return IO_FAILED;
    }
    return IO_SUCCESS;
}

iostatus_t io_device_decrease_reference(device_object_t *devobj)
{
    /* 减少引用 */
    if (atomic_get(&devobj->reference) >= 0) {
        atomic_dec(&devobj->reference);
    } else {
        printk(KERN_ERR "device_close: reference %d error!\n", atomic_get(&devobj->reference));
        return IO_FAILED;    
    }
    return IO_SUCCESS;
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
    device_object_t *devobj;
    /* 搜索设备 */
    devobj = io_search_device_by_name(devname);
    if (devobj == NULL) {
        printk(KERN_ERR "device_open: device %s not found!\n", devname);
        return -1;
    }
    iostatus_t status = IO_SUCCESS;
    
    status = io_device_increase_reference(devobj);
    if (status == IO_FAILED) {
        printk(KERN_ERR "device_open: increase reference failed!\n");
        return -1;
    }

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
        
        status = io_call_dirver(devobj, ioreq);
        if (!io_complete_check(ioreq, status)) {
            io_request_free((ioreq));
            /* 真正打开的时候才创建句柄 */
            spin_lock(&driver_lock);
            handle_t handle = device_handle_table_insert(devobj);
            if (handle == -1) {
                printk(KERN_ERR "device_open: insert device handle tabel failed!\n");
                spin_unlock(&driver_lock);
                return -1;  /* 插入句柄表失败 */    
            }
            spin_unlock(&driver_lock);

            return handle;
        }
        io_request_free(ioreq);
        /* 失败则跳转到回滚 */
        goto rollback_ref;
    } else {
        handle_t handle = device_handle_find_by_object(devobj);
        return handle;
    }
    
rollback_ref:
    printk(KERN_ERR "device_open: do dispatch failed!\n");
    io_device_decrease_reference(devobj);

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
    
    iostatus_t status = IO_SUCCESS;
    status = io_device_decrease_reference(devobj);
    if (status == IO_FAILED) {
        return -1;
    }
    io_request_t *ioreq = NULL;

    if (!atomic_get(&devobj->reference)) { /* 最后一次关闭才关闭 */    
        ioreq = io_build_sync_request(IOREQ_CLOSE, devobj, NULL, 0, 0, NULL);
        if (ioreq == NULL) {
            printk(KERN_ERR "device_close: alloc io request packet failed!\n", atomic_get(&devobj->reference));
            goto rollback_ref;
        }
        
        status = io_call_dirver(devobj, ioreq);
        if (!io_complete_check(ioreq, status)) {
            /* 真正关闭的时候，才删除句柄 */
            spin_lock(&driver_lock);
            if (device_handle_table_remove(devobj)) {
                printk(KERN_ERR "device_close: device=%s remove from device handle table failed!\n",
                    devobj->name.text);
                spin_unlock(&driver_lock);
                return -1;
            }
            spin_unlock(&driver_lock);

            io_request_free((ioreq));
            return 0;
        }
        io_request_free(ioreq);
        
        /* 失败则跳转到回滚 */
        goto rollback_ref;
    } else {
        /* 引用计数>=1，直接返回0 */
        return 0;
    }
rollback_ref:
#ifdef DEBUG_DRIVER
    printk(KERN_DEBUG "device_close: do dispatch failed!\n");
#endif
    io_device_increase_reference(devobj);
    return -1;
}


/**
 * device_mmap - 映射设备操作
 * @handle: 设备句柄
 * @length: 设备资源长度
 * @flags: 标志
 * 
 * @return: 成功返回映射后的地址，失败返回NULL
 */
void *device_mmap(handle_t handle, size_t length, int flags)
{
    if (IS_BAD_DEVICE_HANDLE(handle))
        return NULL;

    device_object_t *devobj;
    
    devobj = GET_DEVICE_BY_HANDLE(handle);

    /* 获取设备 */
    if (devobj == NULL) {
        printk(KERN_ERR "%s: device object error by handle=%d!\n", __func__, handle);
        /* 应该激活一个触发器，让调用者停止运行 */
        return NULL;
    }
    
    iostatus_t status = IO_SUCCESS;
    
    io_request_t *ioreq = NULL;

    ioreq = io_build_sync_request(IOREQ_MMAP, devobj, NULL, length, flags, NULL);
    if (ioreq == NULL) {
        printk(KERN_ERR "%s: alloc io request packet failed!\n", __func__);
        return NULL;
    }
    status = io_call_dirver(devobj, ioreq);

    if (!io_complete_check(ioreq, status)) {
        void *mapaddr = NULL;
        /* 对获取的物理地址进行映射 */
#ifdef DEBUG_DRIVER        
        printk(KERN_DEBUG "%s: get device phy addr:%x\n", __func__, ioreq->io_status.infomation);
#endif
        if (ioreq->io_status.infomation) {  /* 有物理地址，说明获取成功，再做进一步设置 */
            /* 进行内存映射 */
            if (flags & IO_KERNEL)
                mapaddr = ioremap(ioreq->io_status.infomation, length);
            else
                mapaddr = vmspace_mmap(0, ioreq->io_status.infomation, length, 
                    PROT_USER | PROT_WRITE, VMS_MAP_SHARED | VMS_MAP_REMAP);

        }
        io_request_free((ioreq));
        return mapaddr;
    }
#ifdef DEBUG_DRIVER
    printk(KERN_DEBUG "%s: do dispatch failed!\n", __func__);
#endif
    io_request_free((ioreq));
    return NULL;
}

/**
 * device_grow - 增加设备引用计数
 * @handle: 设备句柄
 * 
 * @return: 成功返回0，失败返回-1
 */
int device_grow(handle_t handle)
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
    if (io_device_increase_reference(devobj) == IO_SUCCESS)
        return 0;
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
    if (IS_BAD_DEVICE_HANDLE(handle))
        return -1;
    
    device_object_t *devobj;
    
    devobj = GET_DEVICE_BY_HANDLE(handle);
    /* 获取设备 */
    if (devobj == NULL) {
        printk(KERN_ERR "device_read: device object error by handle=%d!\n", handle);
        /* 应该激活一个触发器，让调用者停止运行 */
        return -1;
    }
    int len;
    iostatus_t status = IO_SUCCESS;
    io_request_t *ioreq = NULL;
    ioreq = io_build_sync_request(IOREQ_READ, devobj, buffer, length, offset, NULL);
    if (ioreq == NULL) {
        printk(KERN_ERR "device_read: alloc io request packet failed!\n");
        return -1;
    }
    status = io_call_dirver(devobj, ioreq);   //调用设备驱动的功能函数

    if (!io_complete_check(ioreq, status)) {
        //printk("io complete.\n");
        len = ioreq->io_status.infomation;
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
        /* 读取后，分发到已经其它在设备上的进程中去 */

        io_request_free((ioreq));
        //printk("io ret.\n");
        return len;
    }
#ifdef DEBUG_DRIVER
    printk(KERN_ERR "device_read: do dispatch failed!\n");
#endif
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
    if (IS_BAD_DEVICE_HANDLE(handle))
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
#ifdef DEBUG_DRIVER
    printk(KERN_ERR "device_write: do dispatch failed!\n");
#endif
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
 * @return: 成功返回信息数，失败返回-1
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
#ifdef DEBUG_DRIVER
    printk(KERN_ERR "device_devctl: do dispatch failed!\n");
#endif
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

int input_even_init(input_even_buf_t *evbuf)
{
    spinlock_init(&evbuf->lock);
    evbuf->head = evbuf->tail = 0;
    memset(evbuf->evbuf, 0, sizeof(input_event_t) * EVBUF_SIZE);
    return 0;
}


int input_even_put(input_even_buf_t *evbuf, input_event_t *even)
{
    unsigned long flags;
    spin_lock_irqsave(&evbuf->lock, flags);
    evbuf->evbuf[evbuf->head++] = *even;
    evbuf->head &= EVBUF_SIZE - 1;
    spin_unlock_irqrestore(&evbuf->lock, flags);
    return 0;
}

int input_even_get(input_even_buf_t *evbuf, input_event_t *even)
{
    unsigned long flags;
    spin_lock_irqsave(&evbuf->lock, flags);
    if (evbuf->head == evbuf->tail) {   /* 没有数据后就返回 */
        spin_unlock_irqrestore(&evbuf->lock, flags);
        return -1;    
    }
    *even = evbuf->evbuf[evbuf->tail++];
    evbuf->tail &= EVBUF_SIZE - 1;
    spin_unlock_irqrestore(&evbuf->lock, flags);
    return 0;
}

/* 初始化驱动架构 */
void init_driver_arch()
{
    int i;
    /* 初始化设备句柄表，要在初始化驱动前，因为驱动中可能有虚拟驱动调用物理设备 */
    for (i = 0; i < DEVICE_HANDLE_NR; i++) {
        device_handle_table[i] = NULL;
    }

#ifdef DEBUG_DRIVER
    //print_drivers_mini();
    /* 输出所有驱动以及设备 */
    print_drivers();
#endif

#if 0
    handle_t beep = device_open("buzzer", 0);
    if (beep < 0)
        panic(KERN_DEBUG "open buzzer failed!\n");
    
    device_devctl(beep, SNDIO_PLAY, 0);
    //int i;
    for (i = 20; i < 20000; i++) {
        device_devctl(beep, SNDIO_SETFREQ, i);
        udelay(5000);
    }
    device_devctl(beep, SNDIO_STOP, 0);

    device_close(beep);
    while (1)
    {
        /* code */
    }
#endif


#if 0
    int sda = device_open("sata0", 0);
    if (sda < 0)
        panic(KERN_DEBUG "open sda0 failed!\n");
    /*
    char *buffer;
    for (i = 200; i <= 220; i++) {
        char *buffer = kmalloc(PAGE_SIZE * i);
        memset(buffer, 0, PAGE_SIZE);
        printk(KERN_DEBUG "read on %d len=%d\n", i, device_read(sda, buffer, SECTOR_SIZE * i, i));
        printk(KERN_DEBUG "%x %x\n", buffer[0], buffer[511]);
        memset(buffer, i, PAGE_SIZE);
        printk(KERN_DEBUG "write to %d len=%d\n", i, device_write(sda, buffer, SECTOR_SIZE * i, i));
        kfree(buffer);
    }*/
    char *buffer = kmalloc(PAGE_SIZE);
    memset(buffer, 0, PAGE_SIZE);
    printk(KERN_DEBUG "read on %d len=%d\n", 0, device_read(sda, buffer, SECTOR_SIZE, 0));
    uint32_t *p = (uint32_t *) buffer;
    for (i = 0; i < SECTOR_SIZE / sizeof(uint32_t); i++) {
        printk("%x  ", p[i]);
    }

    spin("ahci test.");
#endif    


#if 0    
    handle_t null = device_open("null", 0);
    if (null < 0)
        panic(KERN_DEBUG "open null failed!\n");
    
    char *buffer = kmalloc(PAGE_SIZE);
    memset(buffer, 0xff, PAGE_SIZE);
    printk(KERN_DEBUG "write len=%d\n", device_write(null, buffer, PAGE_SIZE, 0));
    memset(buffer, 0, PAGE_SIZE);
    printk(KERN_DEBUG "read len=%d\n", device_read(null, buffer, PAGE_SIZE, 0));
    printk(KERN_DEBUG "%x %x: %x %x\n", buffer[0], buffer[511], buffer[512], buffer[1023]);

#endif
#if 0    
    handle_t ramd = device_open("vfloppy", 0);
    if (ramd < 0)
        panic(KERN_DEBUG "open ramdisk failed!\n");

    char *buffer = kmalloc(PAGE_SIZE);
    memset(buffer, 0xff, PAGE_SIZE);
    printk(KERN_DEBUG "write len=%d\n", device_write(ramd, buffer, PAGE_SIZE, 1));
    memset(buffer, 0, PAGE_SIZE);
    printk(KERN_DEBUG "read len=%d\n", device_read(ramd, buffer, PAGE_SIZE, 0));
    printk(KERN_DEBUG "%x %x: %x %x\n", buffer[0], buffer[511], buffer[512], buffer[1023]);

    device_devctl(ramd, DISKIO_CLEAR, 0);
    printk(KERN_DEBUG "disk sectors=%d\n", device_devctl(ramd, DISKIO_GETSIZE, 0));
#endif
#if 0
    handle_t net0 = device_open("rtl8139", 0);
    if (net0 < 0)
        printk("open rtl 8139 failed!\n");

    loop_delay(100);
    
    char *net_buf = kmalloc(2048);
    if (net_buf == NULL) {
        panic("alloc for net buf failed!\n");
    }
    while (1) {
        loop_delay(10);
        //device_write(net0, net_buf, 128, 0);
        int len = device_read(net0, net_buf, 2048, 0);
        if (len > 0)
            dump_buffer(net_buf, len, 1);
    }
    

    device_close(net0);

    loop_delay(100);

    net0 = device_open("rtl8139", 0);
    if (net0 < 0)
        printk("open rtl 8139 failed!\n");


    while (1)
    {
        
    }
#endif
#if 0
    handle_t handle = device_open("com0", 123);
    printk(KERN_DEBUG "init_driver_arch: open device handle=%d\n", handle);
    if (handle >= 0)
        dump_device_object(GET_DEVICE_BY_HANDLE(handle));
    handle_t handle1 = device_open("com1", 123);
    printk(KERN_DEBUG "init_driver_arch: open device handle=%d\n", handle);
    if (handle1 >= 0)
        dump_device_object(GET_DEVICE_BY_HANDLE(handle1));
    handle_t con0 = device_open("con0", 0);
    printk(KERN_DEBUG "init_driver_arch: open device handle=%d\n", con0);
    if (handle >= 0)
        dump_device_object(GET_DEVICE_BY_HANDLE(con0));
        
    device_write(handle, "hello, com0\n", 12, 0);
    device_write(handle1, "hello, com1\n", 12, 0);
    device_devctl(con0, CODE_CON_SETPOS, 10 * 80 + 0);
    device_devctl(con0, CODE_CON_SETCOLOR, 0x5);
    
    device_write(con0, "hello, con0!\nsecond line.\nabc\n", 32, 0);
    device_devctl(con0, CODE_CON_SETPOS, 10 * 80 + 0);

    char buf[32] = {0, };
    device_read(con0, buf, 32, 0);
    printk(KERN_DEBUG "read buffer:%s\n", buf);
    
    printk(KERN_DEBUG "get pos:%d color:%d\n", device_devctl(con0, CODE_CON_GETPOS, 0), device_devctl(con0, CODE_CON_GETCOLOR, 0));
    
    iostatus_t status;
    status = device_close(handle);
    printk(KERN_DEBUG "init_driver_arch: close devce ihandle=%d status=%d\n", handle, status);

    handle_t ide0 = device_open("ide0", 0);
    printk(KERN_DEBUG "init_driver_arch: open device handle=%d\n", ide0);
    if (handle >= 0)
        dump_device_object(GET_DEVICE_BY_HANDLE(ide0));
    
    char *disk_buf = kmalloc(256 * 1024);
    if (disk_buf == NULL)
        panic("kmalloc for disk buf failed!\n");

    printk(KERN_DEBUG "read disk:%d\n", device_read(ide0, disk_buf, 256 * 1024, 0));
    printk(KERN_DEBUG "write disk:%d\n", device_write(ide0, disk_buf, 256 * 1024, 0));
    
    dump_buffer(disk_buf, 32, 1);    
    /*
    io_uninstall_driver("uart-serial");
    io_uninstall_driver("vga-console");
    io_uninstall_driver("ide-disk");
    */
#endif
    //spin("test");
}
