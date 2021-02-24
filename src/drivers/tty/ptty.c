#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/schedule.h>
#include <xbook/pipe.h>
#include <xbook/exception.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DRV_NAME "pseudo-terminal"
#define DRV_VERSION "0.1"

#define DEV_NAME_MASTER "ptm"
#define DEV_NAME_SLAVE "pts"

// #define PTTY_DEBUG

enum {
    PTTY_MASTER,    /* 主端 */
    PTTY_SLAVER,     /* 从端 */
};

enum {
    PTTY_RDNOBLK = 0x01,    
    PTTY_WRNOBLK = 0x02,
};

/* master 数量 */
#define NR_PTM      8

typedef struct _device_extension {
    int locked;         /* 上锁，表示是否允许打开 */
    int opened;         /* 处于打开状态 */
    device_object_t *other_devobj; // other devobj
    int type;      /* 设备类型 */
    int device_id;  /* 设备id */
    pid_t hold_pid; /* 持有控制权的进程 */
    pipe_t *pipe_in;
    pipe_t *pipe_out;
    int flags;      
} device_extension_t;

iostatus_t ptty_open(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_FAILED;
    device_extension_t *extension = device->device_extension;
    pipe_t *pipe_in;
    pipe_t *pipe_out;

    device_object_t *devobj;
    device_extension_t *devext;
    /* 打开时创建other_devobj端 */
    if (extension->type == PTTY_MASTER) {
        if (extension->other_devobj == NULL && extension->pipe_in == NULL && extension->pipe_out == NULL) { // 没有从端就创建
            /* 创建一对管道 */
            pipe_in = create_pipe();
            if (pipe_in == NULL) {
                keprint(PRINT_ERR "ptty_open: create in pipe failed!\n");
                goto err_pipe_in;
            }
            pipe_out = create_pipe();
            if (pipe_out == NULL) {
                keprint(PRINT_ERR "ptty_open: create out pipe failed!\n");
                goto err_pipe_out;
            }

            char devname[DEVICE_NAME_LEN] = {0, };
            memset(devname, 0, DEVICE_NAME_LEN);
            sprintf(devname, "%s%d", DEV_NAME_SLAVE, extension->device_id); // 和主端一样的id
            /* 创建一个other_devobj终端设备 */
            status = io_create_device(device->driver, sizeof(device_extension_t), devname, DEVICE_TYPE_VIRTUAL_CHAR, &devobj);
            if (status != IO_SUCCESS) {
                keprint(PRINT_ERR "ptty_open: create master device failed!\n");
                goto err_create_dev;
            }
            /* neither io mode */
            devobj->flags = 0;
            devext = (device_extension_t *)devobj->device_extension;
            devext->type = PTTY_SLAVER;   /* 从终端 */
            devext->device_id = extension->device_id;
            
            extension->pipe_in = pipe_in;
            extension->pipe_out = pipe_out;
            extension->other_devobj = devobj;

            /* 对于从设备来说，读写端互换 */
            devext->pipe_in = pipe_out;
            devext->pipe_out = pipe_in;
            devext->other_devobj = device;
            devext->locked = 1; // locked
            devext->flags = 0;
            devext->opened = 0;
            extension->hold_pid = -1;
        }
    } else {
        /* 如果设备上锁了，就不能打开 */
        if (extension->locked) {
            goto err_no;
        }
        
    }
    extension->opened = 1; // 打开
    extension->hold_pid = task_current->pid;   
    
    status = IO_SUCCESS;
    #ifdef PTTY_DEBUG
    keprint(PRINT_INFO "ptty_open: device %s ref %d success!\n", device->name.text, atomic_get(&device->reference));
    
    keprint(PRINT_INFO "ptty_open: other device %s ref %d success!\n", extension->other_devobj->name.text,
        atomic_get(&extension->other_devobj->reference));
    #endif
    goto err_no;

err_create_dev:
    destroy_pipe(pipe_out);
err_pipe_out:
    destroy_pipe(pipe_in);
err_pipe_in:
err_no:
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

iostatus_t ptty_close(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_FAILED;
    device_extension_t *extension = device->device_extension;
    device_extension_t *devext;
    /* 关闭时销毁other_devobj端 */
    if (extension->type == PTTY_MASTER) {
        extension->locked = 0;
        
        extension->opened = 0;
        if (extension->other_devobj) {
            devext = extension->other_devobj->device_extension;
            if (!devext->opened) { // closed
                #ifdef PTTY_DEBUG
                keprint(PRINT_NOTICE "ptty_close: master clear pipe.\n");
                #endif
                pipe_clear(devext->pipe_in);
                pipe_clear(devext->pipe_out);
            }
        }
    } else if (extension->type == PTTY_SLAVER) {
        /* 如果设备上锁了，就不能关闭 */
        if (extension->locked) {
            goto err_not_found;
        }
        extension->locked = 1;
        extension->opened = 0;
        if (extension->other_devobj) {
            devext = extension->other_devobj->device_extension;
            if (!devext->opened) { // closed
                #ifdef PTTY_DEBUG
                keprint(PRINT_NOTICE "ptty_close: slaver clear pipe.\n");
                #endif
                pipe_clear(devext->pipe_in);
                pipe_clear(devext->pipe_out);
            }
        }
    }
    extension->flags = 0;
    extension->hold_pid = -1;
    #ifdef PTTY_DEBUG
    keprint(PRINT_INFO "ptty_close: device %s success!\n", device->name.text);
    #endif

    status = IO_SUCCESS;
err_not_found:
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

iostatus_t ptty_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    
    iostatus_t status = IO_FAILED;
    uint8_t *buf = (uint8_t *) ioreq->user_buffer;
    int len = ioreq->parame.read.length;

    /* 前台任务 */
    if (extension->hold_pid == task_current->pid) {
        
        #ifdef PTTY_DEBUG
        keprint(PRINT_INFO "ptty_read: buf %x len %d.\n", buf, len);
        #endif
        /* 从读端读取 */
        if ((len = pipe_read(extension->pipe_in->id, buf, len)) < 0)
            goto err_rd;

    } else {
        keprint(PRINT_ERR "[ptty]: pid %d read but not holder, abort!\n", task_current->pid);
        /* 不是前台任务就触发任务的硬件触发器 */
        exception_force_self(EXP_CODE_TTIN);
        goto err_rd;
    }
    status = IO_SUCCESS;
err_rd:
#ifdef PTTY_DEBUG
    keprint(PRINT_INFO "ptty_read: read %d bytes.\n", len);
#endif    
    ioreq->io_status.infomation = len;
    ioreq->io_status.status = status;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return status;
}

iostatus_t ptty_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;

    uint8_t *buf = (uint8_t *) ioreq->user_buffer;
    int len = ioreq->parame.write.length;
    #ifdef PTTY_DEBUG
    keprint(PRINT_INFO "ptty_write: buf %x len %d.\n", buf, len);
    #endif
    // if (extension->hold_pid == task_current->pid) {
    /* 从写端写入 */
    if ((len = pipe_write(extension->pipe_out->id, buf, len)) < 0)
        goto err_wr;
    
    #if 0
    } else {
        keprint(PRINT_ERR "[ptty]: pid %d write but not holder, abort!\n", task_current->pid);
        exception_force_self(EXP_CODE_TTOU);
        goto err_wr;
    }*/
    #endif
    status = IO_SUCCESS;
err_wr:
#ifdef PTTY_DEBUG
    keprint(PRINT_INFO "ptty_write: write %d bytes.\n", len);
#endif
    ioreq->io_status.infomation = len;
    ioreq->io_status.status = status;
    /* 调用完成请求 */
    io_complete_request(ioreq);

    return status;
}

iostatus_t ptty_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    int flags;
    switch (ioreq->parame.devctl.code)
    {    
    case TIOCGPTN:
        if (extension->other_devobj && extension->type == PTTY_MASTER) {
            extension = extension->other_devobj->device_extension;
            *(unsigned long *)ioreq->parame.devctl.arg = extension->device_id;
        } else {
            status = IO_FAILED;
        }
        break;
    case TIOCSPTLCK:
        if (extension->other_devobj && extension->type == PTTY_MASTER) {
            extension = extension->other_devobj->device_extension;
            extension->locked = *(unsigned long *) ioreq->parame.devctl.arg;
        } else {
            status = IO_FAILED;
        }
        break;
    case TIOCSFLGS:
        extension->flags = *(unsigned long *) ioreq->parame.devctl.arg;
        if (extension->flags & PTTY_RDNOBLK) {
            flags = O_NONBLOCK;
            if (pipe_ioctl(extension->pipe_in->id, F_SETFL, (unsigned long) &flags, 0) < 0)
                status = IO_FAILED;
        }
        if (extension->flags & PTTY_WRNOBLK) {
            flags = O_NONBLOCK;
            if (pipe_ioctl(extension->pipe_out->id, F_SETFL, (unsigned long) &flags, 1) < 0)
                status = IO_FAILED;
        }
        break;
    case TIOCGFLGS:
        *(unsigned long *) ioreq->parame.devctl.arg = extension->flags;
        break;
    case TTYIO_HOLDER:
        extension->hold_pid = *(unsigned long *) ioreq->parame.devctl.arg;
        break;
    case TIOCGFG: /* get front group tasks */
        if (extension->other_devobj && extension->type == PTTY_MASTER) {
            extension = extension->other_devobj->device_extension;
            if (extension->hold_pid > 0)
                *(unsigned long *)ioreq->parame.devctl.arg = extension->hold_pid;
            else
                status = IO_FAILED;    
        } else {
            status = IO_FAILED;
        }
        break;
    case TIOCISTTY:
        *(unsigned long *) ioreq->parame.devctl.arg = 1;
        break;
    case TIOCNAME:
        {
            char *buf = (char *)ioreq->parame.devctl.arg;
            strncpy(buf, device->name.text, strlen(device->name.text));
        }
        break;
    default:
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t ptty_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *extension;
    
    int i;
    char devname[DEVICE_NAME_LEN] = {0, };
    
    for (i = 0; i < NR_PTM; i++) {
        memset(devname, 0, DEVICE_NAME_LEN);
        sprintf(devname, "%s%d", DEV_NAME_MASTER, i);
        /* 初始化一些其它内容 */
        status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_VIRTUAL_CHAR, &devobj);
        if (status != IO_SUCCESS) {
            keprint(PRINT_ERR "ptty_enter: create device failed!\n");
            return status;
        }
        /* neither io mode */
        devobj->flags = 0;
        extension = (device_extension_t *)devobj->device_extension;
        extension->type = PTTY_MASTER;
        extension->device_id = i;   
        extension->pipe_in = NULL;
        extension->pipe_out = NULL;
        extension->other_devobj = NULL;
        extension->locked = 0;
        extension->flags = 0;
        extension->hold_pid = -1;
        extension->opened = 0;
    }
    status = IO_SUCCESS;
    return status;
}

static iostatus_t ptty_exit(driver_object_t *driver)
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

iostatus_t ptty_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = ptty_enter;
    driver->driver_exit = ptty_exit;

    driver->dispatch_function[IOREQ_OPEN] = ptty_open;
    driver->dispatch_function[IOREQ_CLOSE] = ptty_close;
    driver->dispatch_function[IOREQ_READ] = ptty_read;
    driver->dispatch_function[IOREQ_WRITE] = ptty_write;
    driver->dispatch_function[IOREQ_DEVCTL] = ptty_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef PTTY_DEBUG
    keprint(PRINT_DEBUG "ptty_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void ptty_driver_entry(void)
{
    if (driver_object_create(ptty_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(ptty_driver_entry);
