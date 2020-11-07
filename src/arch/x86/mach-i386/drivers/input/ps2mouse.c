#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include <xbook/debug.h>
#include <xbook/driver.h>
#include <arch/io.h>
#include <arch/interrupt.h>

#define DRV_NAME "input-mouse"
#define DRV_VERSION "0.1"

#define DEV_NAME "mouse"

// #define DEBUG_PS2MOUSE
// #define DEBUG_PS2MOUSE_EVBUF

#define I8042_BUFFER 0x60
#define I8042_STATUS 0x64
#define I8042_ACK 0xFA
#define I8042_BUFFER_FULL 0x01
#define I8042_WHICH_BUFFER 0x20
#define I8042_MOUSE_BUFFER 0x20
#define I8042_KEYBOARD_BUFFER 0x00

#define PS2MOUSE_SET_RESOLUTION 0xE8
#define PS2MOUSE_STATUS_REQUEST 0xE9
#define PS2MOUSE_REQUEST_SINGLE_PACKET 0xEB
#define PS2MOUSE_GET_DEVICE_ID 0xF2
#define PS2MOUSE_SET_SAMPLE_RATE 0xF3
#define PS2MOUSE_ENABLE_PACKET_STREAMING 0xF4
#define PS2MOUSE_DISABLE_PACKET_STREAMING 0xF5
#define PS2MOUSE_SET_DEFAULTS 0xF6
#define PS2MOUSE_RESEND 0xFE
#define PS2MOUSE_RESET 0xFF

#define PS2MOUSE_LEFT_CLICK 0x01
#define PS2MOUSE_RIGHT_CLICK 0x02
#define PS2MOUSE_MIDDLE_CLICK 0x04

#define PS2MOUSE_INTELLIMOUSE_ID 0x03

/*
mouse data packet: 
data0:  7: y overflow, 6: x overflow, 5: y sign bit, 4: x sign bit,
	    3: alawys 1  , 2: middle btn, 1: right btn , 0: left btn  .
data1:  x 移动值
data2:  y 移动值
data3:  z 移动值 (如果设备支持)
data4:  7: y always 0, 6: always 0, 5: 鼠标第5键, 4: 鼠标第4键,
	    3: 滚轮水平左滚动, 2: 滚轮水平右滚动, 1: 滚轮垂直下滚动, 0: 滚轮垂直上滚动.
        (如果设备支持)
*/

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    char irq;           /* irq号 */

    input_even_buf_t evbuf;     /* 事件缓冲区 */

    /* 按键记录:
     bit 0 置1表示左键已经按下。
     bit 1 置1表示右键已经按下。
     bit 2 置1表示中键已经按下。
    */
    uint8_t button_record;      
    bool has_wheel;             /* device has wheel ? */
    bool device_present;        /* does device exist ? */
    u8 data_state;              /* mouse data state, index in data table */
    u8 data[4];                 /* mouse data get from hardware */
} device_extension_t;

static u8 ps2mouse_read_data();
static void ps2mouse_write_data(u8 data);
static void ps2mouse_prepare_for_output();
static void ps2mouse_prepare_for_input();
static void ps2mouse_expect_ack();
static void ps2mouse_initialize_device(device_extension_t *devext);
static void ps2mouse_check_device_presence(device_extension_t *devext);
static void ps2mouse_initialize(device_extension_t *devext);
static u8 ps2mouse_wait_then_read(u8 port);
static void ps2mouse_wait_then_write(u8 port, u8 data);

static void parse_data_packet(device_extension_t *devext)
{
    int x = devext->data[1];
    int y = devext->data[2];
    int z = 0;
    if (devext->has_wheel)
        z = (char)devext->data[3];
    bool x_overflow = devext->data[0] & 0x40;
    bool y_overflow = devext->data[0] & 0x80;
    bool x_sign = devext->data[0] & 0x10;
    bool y_sign = devext->data[0] & 0x20;
    if (x && x_sign)
        x -= 0x100;
    if (y && y_sign)
        y -= 0x100;
    if (x_overflow || y_overflow) {
        x = 0;
        y = 0;
    }
    
    input_event_t e;
    /* 水平方向 */
    if (x) {  /* x轴有变化 */
        e.type = EV_REL;
        e.code = REL_X;
        e.value = x;
        
        /* 推送事件 */
        input_even_put(&devext->evbuf, &e);
    }
    /* 垂直方向 */
    if (y) {  /* x轴有变化 */
        e.type = EV_REL;
        e.code = REL_Y;
        y = -y; /* y是倒立的 */
        e.value = y; 
        /* 推送事件 */
        input_even_put(&devext->evbuf, &e);
    }
    /* 纵深方向 */
    if (z) {  /* z轴有变化 */
        e.type = EV_REL;
        e.code = REL_WHEEL;
        e.value = z;
        
        /* 推送事件 */
        input_even_put(&devext->evbuf, &e);
    }
    /* 同步鼠标移动 */
    e.type = EV_SYN;
    e.code = 0;
    e.value = 0;
    /* 推送事件 */
    input_even_put(&devext->evbuf, &e);

    /* 按钮 */
    devext->data[0] &= 0x07; /* 只取低3位 */
    if (devext->data[0] & PS2MOUSE_LEFT_CLICK) {   /* left button */
        if (!(devext->button_record & PS2MOUSE_LEFT_CLICK)) {   /* left button not pressed */
            devext->button_record |= 0x01; /* record */

            /* 鼠标左键按下 */
            e.type = EV_KEY;
            e.code = BTN_LEFT;
            e.value = 1;    /* 1表示按下 */
            /* 推送事件 */
            input_even_put(&devext->evbuf, &e);
        }
    } else {    /* no left button */
        /* 如果上次是按下，这次又没有按下，就是弹起 */
        if (devext->button_record & PS2MOUSE_LEFT_CLICK) {
            devext->button_record &= ~PS2MOUSE_LEFT_CLICK; /* clear record */
            
            /* 鼠标左键弹起 */
            e.type = EV_KEY;
            e.code = BTN_LEFT;
            e.value = 0;    /* 0表示弹起 */
            /* 推送事件 */
            input_even_put(&devext->evbuf, &e);
        }
    } 
    if (devext->data[0] & PS2MOUSE_RIGHT_CLICK) {   /* right button */
        if (!(devext->button_record & PS2MOUSE_RIGHT_CLICK)) {   /* right button not pressed */
            devext->button_record |= PS2MOUSE_RIGHT_CLICK; /* record */

            /* 鼠标右键按下 */
            e.type = EV_KEY;
            e.code = BTN_RIGHT;
            e.value = 1;    /* 1表示按下 */
            /* 推送事件 */
            input_even_put(&devext->evbuf, &e);
        }
    } else {    /* no right button */
        /* 如果上次是按下，这次又没有按下，就是弹起 */
        if (devext->button_record & PS2MOUSE_RIGHT_CLICK) {
            devext->button_record &= ~PS2MOUSE_RIGHT_CLICK; /* clear record */
            
            /* 鼠标左键弹起 */
            e.type = EV_KEY;
            e.code = BTN_RIGHT;
            e.value = 0;    /* 0表示弹起 */
            /* 推送事件 */
            input_even_put(&devext->evbuf, &e);
        }
    } 
    if (devext->data[0] & PS2MOUSE_MIDDLE_CLICK) {   /* middle button */
        if (!(devext->button_record & PS2MOUSE_MIDDLE_CLICK)) {   /* middle button not pressed */
            devext->button_record |= PS2MOUSE_MIDDLE_CLICK; /* record */

            /* 鼠标中键按下 */
            e.type = EV_KEY;
            e.code = BTN_MIDDLE;
            e.value = 1;    /* 1表示按下 */
            /* 推送事件 */
            input_even_put(&devext->evbuf, &e);
        }
    } else {    /* no middle button */
        /* 如果上次是按下，这次又没有按下，就是弹起 */
        if (devext->button_record & PS2MOUSE_MIDDLE_CLICK) {
            devext->button_record &= ~PS2MOUSE_MIDDLE_CLICK; /* clear record */
            
            /* 鼠标左键弹起 */
            e.type = EV_KEY;
            e.code = BTN_MIDDLE;
            e.value = 0;    /* 0表示弹起 */
            /* 推送事件 */
            input_even_put(&devext->evbuf, &e);
        }
    }

#ifdef DEBUG_PS2MOUSE
    printk(KERN_DEBUG "PS2 Mouse: Buttons %x\n", devext->data[0] & 0x07);
    printk(KERN_DEBUG "Mouse: X %d, Y %d, Z %d\n", x, y, z);
#endif /* DEBUG_PS2MOUSE */
}

static void ps2mouse_commit_packet(device_extension_t *devext) {
    devext->data_state = 0;
#ifdef DEBUG_PS2MOUSE
    printk(KERN_DEBUG "PS2Mouse: raw data: %d, %d %s %s\n",
        devext->data[1],
        devext->data[2],
        (devext->data[0] & 1) ? "Left" : "",
        (devext->data[0] & 2) ? "Right" : "");
#endif /* DEBUG_PS2MOUSE */
    parse_data_packet(devext);
};

/**
 * mouse_handler - 鼠标中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int mouse_handler(unsigned long irq, unsigned long data)
{
    device_extension_t *devext = (device_extension_t *) data;
	/* 先从硬件获取按键数据 */
    while (1)
    {
        u8 status = in8(I8042_STATUS);
        if (!(((status & I8042_WHICH_BUFFER) == I8042_MOUSE_BUFFER) && (status & I8042_BUFFER_FULL)))
            return -1;

        u8 data = in8(I8042_BUFFER);
        devext->data[devext->data_state] = data;

        switch (devext->data_state) {
        case 0:
            if (!(data & 0x08)) {
                printk(KERN_ERR "PS2Mouse: Stream out of sync.\n");
                break;
            }
            ++devext->data_state;
            break;
        case 1:
            ++devext->data_state;
            break;
        case 2:
            if (devext->has_wheel) {
                ++devext->data_state;
                break;
            }
            ps2mouse_commit_packet(devext);
            break;
        case 3:
            ASSERT(devext->has_wheel);
            ps2mouse_commit_packet(devext);
            break;
        }
    }
    return 0;
}

static void ps2mouse_wait_then_write(u8 port, u8 data)
{
    ps2mouse_prepare_for_output();
    out8(port, data);
}

static u8 ps2mouse_wait_then_read(u8 port)
{
    ps2mouse_prepare_for_input();
    return in8(port);
}

static void ps2mouse_initialize(device_extension_t *devext)
{
    // Enable PS aux port
    ps2mouse_wait_then_write(I8042_STATUS, 0xa8);

    ps2mouse_check_device_presence(devext);

    if (devext->device_present)
        ps2mouse_initialize_device(devext);
}

static void ps2mouse_check_device_presence(device_extension_t *devext)
{
    ps2mouse_write_data(PS2MOUSE_REQUEST_SINGLE_PACKET);
    u8 maybe_ack = ps2mouse_read_data();
    if (maybe_ack == I8042_ACK) {
        devext->device_present = true;
        printk(KERN_INFO "ps2mouse: Device detected\n");

        // the mouse will send a packet of data, since that's what we asked
        // for. we don't care about the content.
        ps2mouse_read_data();
        ps2mouse_read_data();
        ps2mouse_read_data();
    } else {
        devext->device_present = false;
        printk(KERN_INFO "ps2mouse: Device not detected\n");
    }
}

static void ps2mouse_initialize_device(device_extension_t *devext)
{
    if (!devext->device_present)
        return;

    // Enable interrupts
    ps2mouse_wait_then_write(I8042_STATUS, 0x20);

    // Enable the PS/2 mouse IRQ (12).
    // NOTE: The keyboard uses IRQ 1 (and is enabled by bit 0 in this register).
    u8 status = ps2mouse_wait_then_read(I8042_BUFFER) | 2;
    ps2mouse_wait_then_write(I8042_STATUS, 0x60);
    ps2mouse_wait_then_write(I8042_BUFFER, status);

    // Set default settings.
    ps2mouse_write_data(PS2MOUSE_SET_DEFAULTS);
    ps2mouse_expect_ack();

    // Enable.
    ps2mouse_write_data(PS2MOUSE_ENABLE_PACKET_STREAMING);
    ps2mouse_expect_ack();
    ps2mouse_write_data(PS2MOUSE_GET_DEVICE_ID);
    ps2mouse_expect_ack();
    u8 device_id = ps2mouse_read_data();

    if (device_id != PS2MOUSE_INTELLIMOUSE_ID) {
        // Send magical wheel initiation sequence.
        ps2mouse_write_data(PS2MOUSE_SET_SAMPLE_RATE);
        ps2mouse_expect_ack();
        ps2mouse_write_data(200);
        ps2mouse_expect_ack();
        ps2mouse_write_data(PS2MOUSE_SET_SAMPLE_RATE);
        ps2mouse_expect_ack();
        ps2mouse_write_data(100);
        ps2mouse_expect_ack();
        ps2mouse_write_data(PS2MOUSE_SET_SAMPLE_RATE);
        ps2mouse_expect_ack();
        ps2mouse_write_data(80);
        ps2mouse_expect_ack();

        ps2mouse_write_data(PS2MOUSE_GET_DEVICE_ID);
        ps2mouse_expect_ack();
        device_id = ps2mouse_read_data();
    }

    if (device_id == PS2MOUSE_INTELLIMOUSE_ID) {
        devext->has_wheel = true;
        printk(KERN_INFO "ps2mouse: Mouse wheel enabled!\n");
    } else {
        printk(KERN_INFO "ps2mouse: No mouse wheel detected!\n");
    }

	register_irq(devext->irq, mouse_handler, IRQF_DISABLED, "IRQ12_MOUSE", DRV_NAME, (unsigned long )devext);
}

static void ps2mouse_expect_ack()
{
    u8 data = ps2mouse_read_data();
    ASSERT(data == I8042_ACK);
}

static void ps2mouse_prepare_for_input()
{
    int timeout = 10000;
    for (;timeout > 0; --timeout) {
        if (in8(I8042_STATUS) & 1)
            return;
    }
}

static void ps2mouse_prepare_for_output()
{
    int timeout = 10000;
    for (;timeout > 0; --timeout) {
        if (!(in8(I8042_STATUS) & 2))
            return;
    }
}

static void ps2mouse_write_data(u8 data)
{
    ps2mouse_prepare_for_output();
    out8(I8042_STATUS, 0xd4);
    ps2mouse_prepare_for_output();
    out8(I8042_BUFFER, data);
}

static u8 ps2mouse_read_data()
{
    ps2mouse_prepare_for_input();
    return in8(I8042_BUFFER);
}

static iostatus_t mouse_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *ext = device->device_extension;

    iostatus_t status = IO_SUCCESS;
    /* 直接返回读取的数据 */
    ioreq->io_status.infomation = ioreq->parame.read.length;
    /* 参数正确 */
    if (ioreq->user_buffer && ioreq->parame.read.length == sizeof(input_event_t)) {
        input_event_t *even = (input_event_t *) ioreq->user_buffer;
        
        if (input_even_get(&ext->evbuf, even)) {
            status = IO_FAILED;
        } else {
#ifdef DEBUG_PS2MOUSE_EVBUF
        printk(KERN_DEBUG "mouse even get: type=%d code=%x value=%d\n", even->type, even->code, even->value);
        printk(KERN_DEBUG "mouse even buf: head=%d tail=%d\n", ext->evbuf.head, ext->evbuf.tail);
#endif        
        }
    } else {
        status = IO_FAILED;
    }
    ioreq->io_status.status = status;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return status;
}

static iostatus_t mouse_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *devext;

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_MOUSE, &devobj);

    if (status != IO_SUCCESS) {
        printk(KERN_ERR "mouse_enter: create device failed!\n");
        return status;
    }
    /* neither io mode */
    devobj->flags = 0;
    devext = (device_extension_t *)devobj->device_extension;
    devext->device_object = devobj;

    devext->irq = IRQ12_MOUSE;
    input_even_init(&devext->evbuf);

    devext->button_record = 0;

    ps2mouse_initialize(devext);

    if (!devext->device_present) {  /* device not exist! */
        io_delete_device(devobj);
        status = IO_FAILED;
    }
    return status;
}

static iostatus_t mouse_exit(driver_object_t *driver)
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

static iostatus_t mouse_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = mouse_enter;
    driver->driver_exit = mouse_exit;
    
    driver->dispatch_function[IOREQ_READ] = mouse_read;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
    return status;
}

static __init void ps2mouse_driver_entry(void)
{
    if (driver_object_create(mouse_driver_func) < 0) {
        printk(KERN_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(ps2mouse_driver_entry);
