#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>
#include <stddef.h>

#include <xbook/driver.h>
#include <xbook/schedule.h>
#include <xbook/exception.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <xbook/fifoio.h>
#include <xbook/safety.h>

#define DRV_NAME "virtual-tty"
#define DRV_VERSION "0.1"

#define DEV_NAME "tty"

/* 键盘设备名 */
//#define KBD_DEVICE_NAME "kbd"
#define KBD_DEVICE_NAME "con0"
/* 控制台设备名 */
#define CON_DEVICE_NAME "con"

/* 一个8个tty设备数 */
#define TTY_DEVICE_NR       1

#define TTY_DEVICE_RAW      7

#define DEV_FIFO_BUF_LEN     64

// #define DEBUG_DRV

struct _device_extension;
/* 设备共有的资源 */
typedef struct _device_public {
    handle_t current_device;        /* 可访问者设备id */
    int detach_kbd;             /* 分离键盘 */
    struct _device_extension *extensions[TTY_DEVICE_NR];
    int counts; // 打开的tty数量
} device_public_t;

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    pid_t pgrp;             /* 进程组ID */
    int device_id;          /* 设备id */
    handle_t con;           /* 对于的控制台设备 */
    handle_t kbd;           /* 对于的键盘设备 */
    device_public_t *public;    /* 共有资源 */
    uint32_t modify_ctl;          /* 组合按键 */
    uint32_t modify_alt;           /* 组合按键 */
    
    fifo_io_t fifoio;
    uint32_t flags;
} device_extension_t;

static int tty_set_current(device_extension_t *extension, int visitor)
{
    if (visitor < 0 || visitor >= TTY_DEVICE_NR)
        return -1;
    /* 设置拜访者设备id */
    extension->public->current_device = visitor;
    return 0;
}

/* 将小键盘的按键转换成主键盘上面的按键 */
static unsigned char __keycode_map_table[] = {
    KEY_KP_PERIOD,      KEY_PERIOD,          /* . */
    KEY_KP_MULTIPLY,    KEY_ASTERISK,        /* * */
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
static unsigned char __key_code_switch(int code)
{
    unsigned char key_value = '?';
    unsigned int i = 0;
    for ( i = 0;  i < sizeof(__keycode_map_table);  i += 2 ) {
        if (__keycode_map_table[i] == code) {
            key_value = __keycode_map_table[i + 1]; // 返回转换后的键值
            return key_value;
        }
    }
    return 0; // not switch
}

static int tty_filter_keycode_down(device_extension_t *extension, int keycode, int retstate)
{
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_RCTRL:    /* right ctl */
    case KEY_LCTRL:    /* left ctl */
        extension->modify_ctl = 1;
    case KEY_LSHIFT:    /* left shift */
    case KEY_RSHIFT:    /* right shift */
    case KEY_NUMLOCK:     /* numlock */
    case KEY_CAPSLOCK:    /* capslock */
    case KEY_SCROLLOCK:  /* scrollock */
    case KEY_UP:           /* up arrow */
    case KEY_DOWN:         /* down arrow */
    case KEY_RIGHT:        /* right arrow */
    case KEY_LEFT:         /* left arrow */
        return retstate;
    case KEY_LALT:    /* left alt */
    case KEY_RALT:    /* right alt */
        extension->modify_alt = 1;
        return retstate;
    default:
        break;
    }
    return 0;
}

static int tty_filter_keycode_up(device_extension_t *extension, int keycode, int retstate)
{
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_RCTRL:    /* right ctl */
    case KEY_LCTRL:    /* left ctl */
        extension->modify_ctl = 0;
    case KEY_LSHIFT:    /* left shift */
    case KEY_RSHIFT:    /* right shift */
    case KEY_NUMLOCK:     /* numlock */
    case KEY_CAPSLOCK:    /* capslock */
    case KEY_SCROLLOCK:  /* scrollock */
    case KEY_UP:           /* up arrow */
    case KEY_DOWN:         /* down arrow */
    case KEY_RIGHT:        /* right arrow */
    case KEY_LEFT:         /* left arrow */
        return retstate;
    case KEY_LALT:    /* left alt */
    case KEY_RALT:    /* right alt */
        extension->modify_alt = 0;
        return retstate;
    default:
        break;
    }
    return 0;
}

iostatus_t tty_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    
    iostatus_t status = IO_FAILED;
    // if (extension->public->current_device == extension->device_id) {  /* 可拜访者设备id */
        // get key code from keyboard fifo buf
        uint8_t *buf = ioreq->user_buffer;
        size_t len = ioreq->parame.read.length;
        size_t read_count = 0;
        // 判断是否有数据
        if (extension->flags & TTYFLG_NOWAIT) {
            int iolen = fifo_io_len(&extension->fifoio);
            if (iolen <= 0) {
                status = IO_FAILED;
                goto end_read;
            }
        }
        // 等待是当前设备
        while (extension->public->current_device != extension->device_id) {
            task_yield();
        }
        if (extension->public->current_device == TTY_DEVICE_RAW) {
            while (len > 0) {
                uint8_t code = fifo_io_get(&extension->fifoio);
                if (code > 0) {
                    *buf = code;
                    buf++;
                    len--;
                    read_count++;
                } else {
                    break;
                }
            }
            ioreq->io_status.infomation = read_count;
            status = IO_SUCCESS;
        } else {
            /* 前台任务 */
            if (extension->pgrp == task_current->pgid) {
                
                while (len > 0) {
                    uint8_t code = fifo_io_get(&extension->fifoio);
                    if (code > 0) {
                        *buf = code;
                        buf++;
                        len--;
                        read_count++;
                    } else {
                        break;
                    }
                }
                ioreq->io_status.infomation = read_count;
                status = IO_SUCCESS;
            
            } else {    /* 不是前台任务就触发任务的硬件触发器 */
                noteprint("pid %d not process group %d ! send TTIN exception.\n", task_current->pid, extension->pgrp);
                exception_force_self(EXP_CODE_TTIN);
            }
        }
        
    // }
end_read:
    ioreq->io_status.status = status;
    /* 调用完成请求 */
    io_complete_request(ioreq);

    return status;
}

static int __tty_write(device_extension_t *extension, char *buf, int len)
{
    return device_write(extension->con, buf, len, 0);
}

iostatus_t tty_write(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    iostatus_t status = IO_SUCCESS;
    int len = __tty_write(extension, ioreq->user_buffer, ioreq->parame.write.length);
    if (len < 0)
        status = IO_FAILED;
    
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = len;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return status;
}

iostatus_t tty_devctl(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *extension = device->device_extension;
    unsigned long arg = ioreq->parame.devctl.arg;
    iostatus_t status = IO_SUCCESS;
    ssize_t retval = 0;
    switch (ioreq->parame.devctl.code)
    {    
    case TTYIO_SELECT:
        tty_set_current(extension, *(unsigned long *)arg);
        break;
    case TIOCSFLGS:
        extension->flags = *(unsigned long *)arg;
        break;        
    case TIOCGFLGS:
        *(unsigned long *)arg = extension->flags;
        break;    
    case TIOCSPGRP:
        extension->pgrp = *(unsigned long *)arg;
        // noteprint("task %s pid=%d set pgrp:%d\n", task_current->name, task_current->pid, extension->pgrp);
        break;
    case TIOCGPGRP:
        *(unsigned long *)arg = extension->pgrp;
        // noteprint("task %s pid=%d get pgrp:%d\n", task_current->name, task_current->pid, extension->pgrp);
        break;
    case TIOCISTTY:
        *(unsigned long *)arg = 1;
        break;        
    case TIOCNAME:
        {
            char *buf = (char *)arg;
            strncpy(buf, device->name.text, strlen(device->name.text));
        }
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

/**
 * 处理切换按键，成功处理返回0，没有处理掉返回-1
*/
static int tty_process_switch(device_extension_t *extension, int code)
{
    /* ctl + alt + key */
    if (extension->modify_ctl && extension->modify_alt) {
        /* 切换tty */
        switch (code) {
        case KEY_F1:  
        case KEY_F2:
        case KEY_F3:
        case KEY_F4:
        case KEY_F5:  
        case KEY_F6:
        case KEY_F7:
        case KEY_F8:
            {
                int device_id = code - KEY_F1;
                tty_set_current(extension, device_id);
            }
            return 0;
        default:
            return -1;
        }
        return 0;
    }
    return -1;
}

/**
 * 成功处理返回0，没有处理掉返回-1
*/
static int tty_process(device_extension_t *extension, int code)
{
    /* ctl + alt + key */
    if (!tty_process_switch(extension, code))
        return 0;
    if (extension->modify_ctl) {
        switch (code) {
        case 'c':
        case 'C':
            // ctl + c
            /* 往进程组发送INT消息 */
            exception_send_group(extension->pgrp, EXP_CODE_INT);
            break;
        default:
            return -1;
        }
        return 0;
    }
    return -1;
}

void tty_thread(void *arg)
{
    device_public_t *public = (device_public_t *)arg;
    while (1) {
        device_extension_t *extension = public->extensions[public->current_device];
        /* read from input even */
        struct input_event event;
        int ret = 0;
        memset(&event, 0, sizeof(event));
        ret = device_read(extension->kbd, &event, sizeof(event), 0);
        if ( ret >= 1 ) {
            switch (event.type) {                
            case EV_KEY:
                // raw设备就发送原始数据
                if (public->current_device == TTY_DEVICE_RAW) {
                    /* 按下的按键 */
                    if ((event.value) > 0) {
                        if (!tty_filter_keycode_down(extension, event.code, 0)) {
                            /* 只处理切换的按键 */
                            if (tty_process_switch(extension, event.code) < 0) {
                                /* put into fifo buf */
                                fifo_io_put(&extension->fifoio, event.code);
                                fifo_io_put(&extension->fifoio, 0); // key down
                            }
                        }
                    } else {
                        /* 弹起 */
                        if (!tty_filter_keycode_up(extension, event.code, 0)) {
                            /* put into fifo buf */
                            fifo_io_put(&extension->fifoio, event.code);
                            fifo_io_put(&extension->fifoio, 1); // key up
                        }
                    }
                } else {
                    /* 按下的按键 */
                    if ((event.value) > 0) {
                        if (!tty_filter_keycode_down(extension, event.code, 1)) {
                            uint8_t ch = __key_code_switch(event.code);
                            if (ch > 0)
                                event.code = ch;

                            /* 处理内部按键 */
                            if (tty_process(extension, event.code) < 0) {
                                /* put into fifo buf */
                                fifo_io_put(&extension->fifoio, event.code);
                                if (extension->flags & TTYFLG_ECHO) {
                                    device_write(extension->con,
                                        &event.code, 1, 0);
                                }
                            }
                        }
                    } else {
                        /* 弹起 */
                        tty_filter_keycode_up(extension, event.code, 1);
                    }
                }
                break;
            default:
                break;
            }
        } else {
            task_yield();
        }
    }
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
        if (extension->pgrp == -1) {    /* 首次打开，组就是当前任务的组 */
            extension->pgrp = task_current->pgid;   /* 绑定组 */
#ifdef DEBUG_DRV
            keprint(PRINT_DEBUG "tty_open: open tty=%d.\n", extension->device_id);
#endif        
        }
    }
    if (extension->public->counts == 0) {
        kern_thread_start("tty", TASK_PRIO_LEVEL_NORMAL, tty_thread, extension->public);
        extension->public->counts++;
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
        extension->pgrp = -1;
#ifdef DEBUG_DRV
        keprint(PRINT_DEBUG "tty_close: close tty=%d.\n", extension->device_id);
#endif
    } else {
        status = IO_FAILED;
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
    public->current_device = -1;
    public->counts = 0;
    /* 打开键盘设备 */
    handle_t kbd = device_open(KBD_DEVICE_NAME, 0);
    if (kbd < 0) {
        keprint(PRINT_DEBUG "tty_enter: open keyboard device failed!\n");
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
            keprint(PRINT_ERR "tty_enter: create device failed!\n");
            device_close(kbd);
            mem_free(public);
            return status;
        }
        /* neither io mode */
        devobj->flags = 0;
        extension = (device_extension_t *)devobj->device_extension;
        extension->device_object = devobj;
        extension->device_id = i;   
        extension->pgrp = -1;
        extension->kbd = kbd;
        extension->con = -1;    /* 没有控制台 */
        extension->modify_ctl = 0;
        extension->modify_alt = 0;
        extension->flags = TTYFLG_ECHO;
        extension->public = public;
        unsigned char *buf = mem_alloc(DEV_FIFO_BUF_LEN);
        if (buf == NULL) {
            status = IO_FAILED;
            keprint(PRINT_DEBUG "keyboard_enter: alloc buf failed!\n");
            device_close(kbd);
            mem_free(public);
            io_delete_device(devobj);
            return status;
        }
        fifo_io_init(&extension->fifoio, buf, DEV_FIFO_BUF_LEN);

        /* 默认第一个tty */
        if (public->current_device == -1)
            public->current_device = i;
        public->extensions[i] = extension;
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
    keprint(PRINT_DEBUG "tty_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void tty_driver_entry(void)
{
    if (driver_object_create(tty_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

filter_initcall(tty_driver_entry);
