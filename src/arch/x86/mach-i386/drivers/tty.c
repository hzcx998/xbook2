#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/schedule.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define DRV_NAME "virtual-tty"
#define DRV_VERSION "0.1"

#define DEV_NAME "tty"

/* 键盘设备名 */
#define KBD_DEVICE_NAME "kbd"
/* 控制台设备名 */
#define CON_DEVICE_NAME "con"

/* 一个8个tty设备数 */
#define TTY_DEVICE_NR       8

// #define DEBUG_DRV

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
            extension->hold_pid = task_current->pid;
#ifdef DEBUG_DRV
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
#ifdef DEBUG_DRV
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


/* 将小键盘的按键转换成主键盘上面的按键 */
static unsigned char _g_keycode_map_table[] = {
    KEY_KP_PERIOD,      KEY_PERIOD,
    KEY_KP_MULTIPLY,    KEY_ASTERISK,     /* * */
    KEY_KP_PLUS,        KEY_PLUS,            /* + */
    KEY_KP_MINUS,       KEY_MINUS,           /* - */
    KEY_KP_DIVIDE,      KEY_SLASH,           /* / */
    KEY_KP0,            KEY_0,               /* 0 */
    KEY_KP1,            KEY_1,               /* 1 */
    KEY_KP2,            KEY_2,               /* 2 */
    KEY_KP3,            KEY_3,               /* 3 */
    KEY_KP4,            KEY_4,               /* 4 */
    KEY_KP5,            KEY_5,               /* 5 */
    KEY_KP6,            KEY_6,               /* 6 */
    KEY_KP7,            KEY_7,               /* 7 */
    KEY_KP8,            KEY_8,               /* 8 */
    KEY_KP9,            KEY_9,               /* 9 */
    0xff,               KEY_UNKNOWN,
};
/**
 * 键值转换
 */
static unsigned char _g_key_code_switch(int code)
{
    unsigned char key_value = '?';
    unsigned int i = 0;
    for ( i = 0;  i < sizeof(_g_keycode_map_table);  i += 2 ) {
        if (_g_keycode_map_table[i] == code) {
            key_value = _g_keycode_map_table[i + 1]; // 返回转换后的键值
            return key_value;
        }
    }
    return 0; // not switch
}


static int tty_filter_keycode(int keycode)
{
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_LSHIFT:    /* left shift */
    case KEY_RSHIFT:    /* right shift */
    case KEY_LALT:    /* left alt */
    case KEY_RALT:    /* right alt */
    case KEY_LCTRL:    /* left ctl */
    case KEY_RCTRL:    /* right ctl */
    case KEY_NUMLOCK:     /* numlock */
    case KEY_CAPSLOCK:    /* capslock */
    case KEY_SCROLLOCK:  /* scrollock */
    case KEY_UP:           /* up arrow */
    case KEY_DOWN:         /* down arrow */
    case KEY_RIGHT:        /* right arrow */
    case KEY_LEFT:         /* left arrow */
        return 1;
    default:
        break;
    }
    return 0;
}



iostatus_t tty_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    
    iostatus_t status = IO_FAILED;
    if (extension->public->visitor_id == extension->device_id) {  /* 可拜访者设备id */
        /* 前台任务 */
        if (extension->hold_pid == task_current->pid) {
            if (!extension->public->detach_kbd) {    /* 键盘分离了就不能读取键盘 */
                /* read from input even */
                struct input_event event;
                int ret = 0;
                
                memset(&event, 0, sizeof(event));
                while (1)
                {
                    ret = device_read(extension->kbd, &event, sizeof(event), 0);
                    if ( ret >= 1 ) {
                        switch (event.type)
                        {                
                            case EV_KEY:
                                /* 按下的按键 */
                                if ((event.value) > 0 && !tty_filter_keycode(event.code)) {
                                    ioreq->io_status.infomation = sizeof(event);
                                    uint8_t ch = _g_key_code_switch(event.code);
                                    if (ch > 0)
                                        event.code = ch;
                                    *(unsigned int *) ioreq->user_buffer = event.code;
                                    status = IO_SUCCESS;
                                    #ifdef DEBUG_DRV                                
                                    printk(KERN_DEBUG "tty: read keycode %x\n", event.code);
                                    #endif
                                    device_write(extension->con,
                                        &event.code, 1, 0);

                                    goto end_of_read;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                  
                }
                
            }
        } else {    /* 不是前台任务就触发任务的硬件触发器 */
            trigger_force(TRIGSYS, task_current->pid);
        }
    }
end_of_read:
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
        extension->hold_pid = task_current->pid;
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
    public = mem_alloc(sizeof(device_public_t));
    if (public == NULL) {
        return IO_FAILED;
    }
    public->visitor_id = -1;
    /* 打开键盘设备 */
    handle_t kbd = device_open(KBD_DEVICE_NAME, 0);
    if (kbd < 0) {
        printk(KERN_DEBUG "tty_enter: open keyboard device failed!\n");
        mem_free(public);
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
            mem_free(public);
            return status;
        }
        /* neither io mode */
        devobj->flags = 0;
        extension = (device_extension_t *)devobj->device_extension;
        extension->device_object = devobj;
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

iostatus_t tty_driver_func(driver_object_t *driver)
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
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "tty_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void tty_driver_entry(void)
{
    if (driver_object_create(tty_driver_func) < 0) {
        printk(KERN_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

filter_initcall(tty_driver_entry);
