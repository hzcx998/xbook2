#include <xbook/debug.h>
#include <xbook/driver.h>
#include <xbook/kernel.h>
#include <xbook/safety.h>
#include <xbook/hardirq.h>
#include <xbook/fifoio.h>
#include <xbook/exception.h>
#include <arch/config.h>
#include <arch/memory.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <arch/sbi.h>

#define DRV_NAME "uart-console"
#define DRV_VERSION "0.1"

#define DEV_NAME "con"

#ifdef QEMU     // QEMU 
#define UART_IRQ    10 
#else           // k210 
#define UART_IRQ    33
#endif 

#define BACKSPACE 0x100
#define C(x)  ((x)-'@')  // Control-x

// #define DEBUG_DRV

/* 1个控制台 */
#define MAX_CONSOLE_NR	    1

#define DEV_FIFO_BUF_LEN     64

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    // input
#define INPUT_BUF 128
    char buf[INPUT_BUF];
    uint32_t r;  // Read index
    uint32_t w;  // Write index
    uint32_t e;  // Edit index
    int flags;
} device_extension_t;

static void console_putc(int c) {
    if(c == BACKSPACE){
        // if the user typed backspace, overwrite with a space.
        sbi_console_putchar('\b');
        sbi_console_putchar(' ');
        sbi_console_putchar('\b');
    } else {
        sbi_console_putchar(c);
    }
}

//
// user read()s from the console go here.
// copy (up to) a whole input line to dst.
// user_dist indicates whether dst is a user
// or kernel address.
//
static int console_do_read(device_extension_t *extension, char *dst, int n)
{
    uint64_t target;
    int c;

    task_t *cur =task_current;
    target = n;
    while(n > 0){
        // wait until interrupt handler has put some
        // input into extension->buffer.
        mb();
        while(extension->r == extension->w){
            /* 等待有可用数据 */
            if (exception_cause_exit_when_wait(&cur->exception_manager)) {
                return -EINTR;
            }
            task_yield();
        }
        /* 读取一个数据 */
        c = extension->buf[extension->r++ % INPUT_BUF];

        if(c == C('D')){  // end-of-file
            if(n < target){
                // Save ^D for next time, to make sure
                // caller gets a 0-byte result.
                extension->r--;
            }
            break;
        }

        // copy the input byte to the user-space buffer.
        *dst = c;
        dst++;
        --n;
        if(c == '\n'){
            // a whole line has arrived, return to
            // the user-level read().
            break;
        }
    }
    return target - n;
}

iostatus_t console_read(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len = ioreq->parame.read.length;
    device_extension_t *ext = device->device_extension;
    char *buf = (char *)ioreq->user_buffer; 
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "console_read: %s\n", buf);
#endif
    iostatus_t status = IO_SUCCESS;
    int rd = console_do_read(ext, buf, len);
    if (rd < 0) {
        status = IO_FAILED;
    }
#ifdef DEBUG_DRV
    dbgprintln("console: read line: %s\n", buf);
#endif    
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = rd;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return status;
}

iostatus_t console_write(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len = ioreq->parame.write.length;
    uint8_t *buf = (uint8_t *)ioreq->user_buffer; 
    //panic("[console] write: buf=%p, len=%d\n", buf, len);
    int i = len;
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "console_write: %s\n", buf);
#endif
    while (i > 0) {
        sbi_console_putchar(*buf);
        i--;
        buf++;
    }
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

iostatus_t console_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;
    //unsigned long arg = ioreq->parame.devctl.arg;
    iostatus_t status = IO_SUCCESS;
    int infomation = 0;
    switch (ctlcode) {
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = infomation;
    io_complete_request(ioreq);
    return status;
}

/**
 * 当终端按下回车键后，才会将按键发送过来，并且是一个按键一个发送，
 * 在最后还会发送回车键值
 */
static int console_intr(irqno_t irqno, void *data)
{
    device_extension_t *extension = (device_extension_t *) data; 
    //keprintln("console intr!");
    int c = sbi_console_getchar();
    if (-1 != c) {
        switch(c){
        case C('P'):  // Print process list.
            tasks_print();
            break;
        case C('U'):  // Kill line.
            while(extension->e != extension->w && 
                extension->buf[(extension->e-1) % INPUT_BUF] != '\n') {
                extension->e--;
                console_putc(BACKSPACE);
            }
            break;
        case C('H'): // Backspace
        case '\x7f':
            if(extension->e != extension->w){
                extension->e--;
                console_putc(BACKSPACE);
            }
            break;
        default:
            if(c != 0 && extension->e-extension->r < INPUT_BUF){
                #ifndef QEMU
                if (c == '\r') break;     // on k210, "enter" will input \n and \r
                #else
                c = (c == '\r') ? '\n' : c;
                #endif
                // echo back to the user.
                if (extension->flags & TTYFLG_ECHO)
                    console_putc(c);
                // store for consumption by console_do_read().
                extension->buf[extension->e++ % INPUT_BUF] = c;
                if(c == '\n' || c == C('D') || extension->e == extension->r+INPUT_BUF){
                    // wake up console_do_read() if a whole line (or end-of-file)
                    // has arrived.
                    mb();
                    extension->w = extension->e;
                }
            }
            break;
        }

    }
    return IRQ_HANDLED;   
}

static iostatus_t console_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *devext;
    int id = 0;
    char devname[DEVICE_NAME_LEN] = {0};

    sprintf(devname, "%s%d", DEV_NAME, id);
    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_SCREEN, &devobj);

    if (status != IO_SUCCESS) {
        keprint(PRINT_ERR "console_enter: create device failed!\n");
        return status;
    }
    /* neither io mode */
    devobj->flags = 0;

    devext = (device_extension_t *)devobj->device_extension;
    devext->device_object = devobj;
    devext->e = devext->w = devext->r = 0;
    devext->flags = 0;  // no TTYFLG_ECHO
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "console_enter: device extension: device name=%s object=%x\n",
        devext->device_name.text, devext->device_object);
#endif
    /* 注册中断 */
    if (irq_register(UART_IRQ, console_intr, 0, "uart irq", DEV_NAME, devext) < 0) {
        keprint(PRINT_ERR "console_enter: register irq %d failed!\n", UART_IRQ);
        io_delete_device(devobj);
        return IO_FAILED;
    }
    dbgprintln("[console] init done");
    return IO_SUCCESS;
}

static iostatus_t console_exit(driver_object_t *driver)
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

iostatus_t console_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = console_enter;
    driver->driver_exit = console_exit;

    driver->dispatch_function[IOREQ_READ] = console_read;
    driver->dispatch_function[IOREQ_WRITE] = console_write;
    driver->dispatch_function[IOREQ_DEVCTL] = console_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "console_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    return status;
}

static __init void console_driver_entry(void)
{
    if (driver_object_create(console_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(console_driver_entry);
