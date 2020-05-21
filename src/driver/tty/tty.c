#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <xbook/vsprintf.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <xbook/task.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>

#define DRV_NAME "virtual-tty"
#define DRV_VERSION "0.1"

#define DEV_NAME "tty"

/* 键盘设备名 */
#define KBD_DEVICE_NAME "kbd"
/* 控制台设备名 */
#define CON_DEVICE_NAME "con"

/* 一个8个tty设备数 */
#define TTY_DEVICE_NR       8

#define DEBUG_LOCAL 0

/* 设备共有的资源 */
typedef struct _device_public {
    handle_t visitor_id;        /* 可访问者设备id */
    int detach_kbd;             /* 分离键盘 */
} device_public_t;

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    pid_t hold_pid;         /* 持有控制权的进程 */
    int device_id;          /* 设备id */
    handle_t con;           /* 对于的控制台设备 */
    handle_t kbd;           /* 对于的键盘设备 */
    device_public_t *public;    /* 共有资源 */
} device_extension_t;

static int tty_set_visitor(device_object_t *device, int visitor)
{
    if (visitor < 0 || visitor >= TTY_DEVICE_NR)
        return -1;
    /* 设置拜访者设备id */
    device_extension_t *extension = device->device_extension;
    extension->public->visitor_id = visitor;
    return 0;
}

iostatus_t tty_open(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    device_extension_t *extension = device->device_extension;
    
    char devname[DEVICE_NAME_LEN] = {0, };
    memset(devname, 0, DEVICE_NAME_LEN);
    sprintf(devname, "%s%d", CON_DEVICE_NAME, extension->device_id);
    extension->con = device_open(devname, 0);
    if (extension->con < 0) {   /* 打开失败就释放资源 */
        status = IO_FAILED;
    } else { /* 打开成功 */
        if (extension->hold_pid == -1) {    /* 首次打开，持有者就是打开者 */
            extension->hold_pid = current_task->pid;
#if DEBUG_LOCAL == 1
            printk(KERN_DEBUG "tty_open: open tty=%d.\n", extension->device_id);
#endif        
        }
    }

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

iostatus_t tty_close(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    device_extension_t *extension = device->device_extension;
    
    if (extension->con >= 0) {
        if (device_close(extension->con))
            status = IO_FAILED;
        extension->con = -1;
        extension->hold_pid = -1;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "tty_close: close tty=%d.\n", extension->device_id);
#endif
    } else {
        status = IO_FAILED;
    }

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}


iostatus_t tty_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    
    iostatus_t status = IO_SUCCESS;
    if (extension->public->visitor_id == extension->device_id) {  /* 可拜访者设备id */
        /* 前台任务 */
        if (extension->hold_pid == current_task->pid) {
            if (extension->public->detach_kbd) {    /* 键盘分离了就不能读取键盘 */
                status = IO_FAILED;
            } else {
                /* read from input even */
                struct input_event event;
                int ret = 0;
                
                memset(&event, 0, sizeof(event));
                ret = device_read(extension->kbd, &event, sizeof(event), 0);
                if ( ret < 1 ) {
                    status = IO_FAILED;
                } else {
                    switch (event.type)
                    {                
                        case EV_KEY:         
                            /* 按下的按键 */
                            if ((event.value) > 0) {
                                //printk("tty: key down %x %c\n", event.code, event.code);
                                ioreq->io_status.infomation = sizeof(event);
                                *(unsigned int *) ioreq->user_buffer = event.code;
                            }
                            break;
                        default:
                            status = IO_FAILED;
                            break;
                    }
                }
                #if 0
                ioreq->io_status.infomation = device_read(extension->kbd,
                    ioreq->user_buffer, 4, ioreq->parame.read.offset); /* read all */
                if (ioreq->io_status.infomation == -1) {
                    status = IO_FAILED;
                } else {
                    /* 过滤弹起码 */
                    unsigned int key = *(unsigned int *) ioreq->user_buffer;
                    //printk("code:%x\n", key);
                    if (key & 0x40000) {    /* 弹起码标志 */
                        status = IO_FAILED; /* 不捕捉 */
                    }
                }
                #endif
            }
        } else {    /* 不是前台任务就触发任务的硬件触发器 */
            trigger_force(TRIGHW, current_task->pid);
            status = IO_FAILED;
        }
    } else {
        status = IO_FAILED;
    }
    ioreq->io_status.status = status;
    /* 调用完成请求 */
    io_complete_request(ioreq);

    return status;
}

iostatus_t tty_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;

    ioreq->io_status.infomation = device_write(extension->con,
        ioreq->user_buffer, ioreq->parame.write.length, ioreq->parame.write.offset);
    
    ioreq->io_status.status = status;
    /* 调用完成请求 */
    io_complete_request(ioreq);

    return status;
}

iostatus_t tty_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    ssize_t retval = 0;
    switch (ioreq->parame.devctl.code)
    {    
    case TTYIO_VISITOR:   /* 设置可以访问键盘的tty */
        tty_set_visitor(device, ioreq->parame.devctl.arg);
    case TTYIO_HOLDER:
        extension->hold_pid = current_task->pid;
        break;
    case TTYIO_DETACH:
        extension->public->detach_kbd = 1;
        break;
    case TTYIO_COMBINE:
        extension->public->detach_kbd = 0;
        break;
            
    default:
        retval = device_devctl(extension->con, ioreq->parame.devctl.code,
            ioreq->parame.devctl.arg);
        if (retval == -1)
            status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t tty_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *extension;
    device_public_t *public;    /* 公有数据 */
    public = kmalloc(sizeof(device_public_t));
    if (public == NULL) {
        return IO_FAILED;
    }
    public->visitor_id = -1;
    /* 打开键盘设备 */
    handle_t kbd = device_open(KBD_DEVICE_NAME, 0);
    if (kbd < 0) {
        printk(KERN_DEBUG "tty_enter: open keyboard device failed!\n");
        kfree(public);
        return IO_FAILED;
    }

    int i;
    char devname[DEVICE_NAME_LEN] = {0, };
    
    for (i = 0; i < TTY_DEVICE_NR; i++) {
        memset(devname, 0, DEVICE_NAME_LEN);
        sprintf(devname, "%s%d", DEV_NAME, i);
        /* 初始化一些其它内容 */
        status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_VIRTUAL_CHAR, &devobj);

        if (status != IO_SUCCESS) {
            printk(KERN_ERR "tty_enter: create device failed!\n");
            device_close(kbd);
            kfree(public);
            return status;
        }
        /* neither io mode */
        devobj->flags = 0;
        extension = (device_extension_t *)devobj->device_extension;
        extension->device_object = devobj;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "tty_enter: device extension: device name=%s object=%x\n",
            devext->device_name.text, devext->device_object);
#endif
        extension->device_id = i;   
        extension->hold_pid = -1;   /* 没有进程持有 */
        extension->kbd = kbd;
        extension->con = -1;    /* 没有控制台 */
        extension->public = public;
        /* 默认第一个tty */
        if (public->visitor_id == -1)
            public->visitor_id = i;
        public->detach_kbd = 0; /* 键盘尚未分离 */
    }
    return status;
}

static iostatus_t tty_exit(driver_object_t *driver)
{
    /* 遍历所有对象 */
    device_object_t *devobj, *next;
    /* 由于涉及到要释放devobj，所以需要使用safe版本 */
    list_for_each_owner_safe (devobj, next, &driver->device_list, list) {
        io_delete_device(devobj);   /* 删除每一个设备 */
    }

    string_del(&driver->name); /* 删除驱动名 */
    return IO_SUCCESS;
}

iostatus_t tty_driver_vine(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = tty_enter;
    driver->driver_exit = tty_exit;

    driver->dispatch_function[IOREQ_OPEN] = tty_open;
    driver->dispatch_function[IOREQ_CLOSE] = tty_close;
    driver->dispatch_function[IOREQ_READ] = tty_read;
    driver->dispatch_function[IOREQ_WRITE] = tty_write;
    driver->dispatch_function[IOREQ_DEVCTL] = tty_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "tty_driver_vine: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}
