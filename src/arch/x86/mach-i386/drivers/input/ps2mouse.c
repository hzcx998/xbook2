#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>

#include <xbook/driver.h>
#include <xbook/fifoio.h>
#include <xbook/task.h>
#include <xbook/spinlock.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>

#define DRV_NAME "input-mouse"
#define DRV_VERSION "0.1"

#define DEV_NAME "mouse"

// #define DEBUG_DRV

#define DEV_FIFO_BUF_LEN     64


#define IRQ_MOUSE 12
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

/* 鼠标数据包 */
struct mouse_data {
	/* 第一字节数据内容
	7: y overflow, 6: x overflow, 5: y sign bit, 4: x sign bit,
	3: alawys 1  , 2: middle btn, 1: right btn , 0: left btn  .
	*/
	uint8_t byte0;
	int byte1;		// x 移动值
	int byte2;		// y 移动值
	int byte3;		// z 移动值
	/*
	7: y always 0, 6: always 0, 5: 鼠标第5键, 4: 鼠标第4键,
	3: 滚轮水平左滚动, 2: 滚轮水平右滚动, 1: 滚轮垂直下滚动, 0: 滚轮垂直上滚动.
	*/
	uint8_t byte4;
};

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    char irq;           /* irq号 */
    
    struct mouse_data mouse_data;
    char phase;		                                        /* 解析步骤 */
    uint8_t raw_data;                                       /* 原始数据 */
    input_even_buf_t evbuf;     /* 事件缓冲区 */

    /* 按键记录:
     bit 0 置1表示左键已经按下。
     bit 1 置1表示右键已经按下。
     bit 2 置1表示中键已经按下。
    */
    uint8_t button_record;      
    int seq;
} device_extension_t;


static device_extension_t *mouse_ext;
static bool m_device_present = false;
static bool m_has_wheel = false;

u8 __mouse_read();
void mouse_write(u8 data);
void prepare_for_output();
void prepare_for_input();
void expect_ack();
void initialize_device();
void check_device_presence();
void initialize();
u8 wait_then_read(u8 port);
void wait_then_write(u8 port, u8 data);

u8 m_data_state = 0;
u8 m_data[4];


static void parse_data_packet()
{
    int x = m_data[1];
    int y = m_data[2];
    int z = 0;
    if (m_has_wheel)
        z = (char)m_data[3];
    bool x_overflow = m_data[0] & 0x40;
    bool y_overflow = m_data[0] & 0x80;
    bool x_sign = m_data[0] & 0x10;
    bool y_sign = m_data[0] & 0x20;
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
        input_even_put(&mouse_ext->evbuf, &e);
    }
    /* 垂直方向 */
    if (y) {  /* x轴有变化 */
        e.type = EV_REL;
        e.code = REL_Y;
        e.value = y;
        /* 推送事件 */
        input_even_put(&mouse_ext->evbuf, &e);
    }
    /* 纵深方向 */
    if (z) {  /* z轴有变化 */
        e.type = EV_REL;
        e.code = REL_WHEEL;
        e.value = z;
        
        /* 推送事件 */
        input_even_put(&mouse_ext->evbuf, &e);
    }
    /* 同步鼠标移动 */
    e.type = EV_SYN;
    e.code = 0;
    e.value = 0;
    /* 推送事件 */
    input_even_put(&mouse_ext->evbuf, &e);

    /* 按钮 */
    m_data[0] &= 0x07; /* 只取低3位 */
    if (m_data[0] & 0x01) {   /* left button */
        if (!(mouse_ext->button_record & 0x01)) {   /* left button not pressed */
            mouse_ext->button_record |= 0x01; /* record */

            /* 鼠标左键按下 */
            e.type = EV_KEY;
            e.code = BTN_LEFT;
            e.value = 1;    /* 1表示按下 */
            /* 推送事件 */
            input_even_put(&mouse_ext->evbuf, &e);
        }
    } else {    /* no left button */
        /* 如果上次是按下，这次又没有按下，就是弹起 */
        if (mouse_ext->button_record & 0x01) {
            mouse_ext->button_record &= ~0x01; /* clear record */
            
            /* 鼠标左键弹起 */
            e.type = EV_KEY;
            e.code = BTN_LEFT;
            e.value = 0;    /* 0表示弹起 */
            /* 推送事件 */
            input_even_put(&mouse_ext->evbuf, &e);
        }
    } 
    if (m_data[0] & 0x02) {   /* right button */
        if (!(mouse_ext->button_record & 0x02)) {   /* right button not pressed */
            mouse_ext->button_record |= 0x02; /* record */

            /* 鼠标右键按下 */
            e.type = EV_KEY;
            e.code = BTN_RIGHT;
            e.value = 1;    /* 1表示按下 */
            /* 推送事件 */
            input_even_put(&mouse_ext->evbuf, &e);
        }
    } else {    /* no right button */
        /* 如果上次是按下，这次又没有按下，就是弹起 */
        if (mouse_ext->button_record & 0x02) {
            mouse_ext->button_record &= ~0x02; /* clear record */
            
            /* 鼠标左键弹起 */
            e.type = EV_KEY;
            e.code = BTN_RIGHT;
            e.value = 0;    /* 0表示弹起 */
            /* 推送事件 */
            input_even_put(&mouse_ext->evbuf, &e);
        }
    } 
    if (m_data[0] & 0x04) {   /* middle button */
        if (!(mouse_ext->button_record & 0x04)) {   /* middle button not pressed */
            mouse_ext->button_record |= 0x04; /* record */

            /* 鼠标中键按下 */
            e.type = EV_KEY;
            e.code = BTN_MIDDLE;
            e.value = 1;    /* 1表示按下 */
            /* 推送事件 */
            input_even_put(&mouse_ext->evbuf, &e);
        }
    } else {    /* no middle button */
        /* 如果上次是按下，这次又没有按下，就是弹起 */
        if (mouse_ext->button_record & 0x04) {
            mouse_ext->button_record &= ~0x04; /* clear record */
            
            /* 鼠标左键弹起 */
            e.type = EV_KEY;
            e.code = BTN_MIDDLE;
            e.value = 0;    /* 0表示弹起 */
            /* 推送事件 */
            input_even_put(&mouse_ext->evbuf, &e);
        }
    }

#if 1
    printk("PS2 Relative Mouse: Buttons %x\n", m_data[0] & 0x07);
    printk("Mouse: X %d, Y %d, Z %d\n", x, y, z);
#endif
    // m_queue.enqueue(packet);
}

static void commit_packet() {
    m_data_state = 0;
#if 1
    printk("PS2Mouse: %d, %d %s %s\n",
        m_data[1],
        m_data[2],
        (m_data[0] & 1) ? "Left" : "",
        (m_data[0] & 2) ? "Right" : "");
#endif
    parse_data_packet();
};

/**
 * mouse_handler - 鼠标中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int mouse_handler(unsigned long irq, unsigned long data)
{
    device_extension_t *extension = (device_extension_t *) data;
	/* 先从硬件获取按键数据 */
    while (1)
    {
        u8 status = in8(I8042_STATUS);
        if (!(((status & I8042_WHICH_BUFFER) == I8042_MOUSE_BUFFER) && (status & I8042_BUFFER_FULL)))
            return;

        u8 data = in8(I8042_BUFFER);
        m_data[m_data_state] = data;

        printk("data %x\n", data);

        switch (m_data_state) {
        case 0:
            if (!(data & 0x08)) {
                printk("PS2Mouse: Stream out of sync.\n");
                break;
            }
            ++m_data_state;
            break;
        case 1:
            ++m_data_state;
            break;
        case 2:
            if (m_has_wheel) {
                ++m_data_state;
                break;
            }
            commit_packet();
            break;
        case 3:
            ASSERT(m_has_wheel);
            commit_packet();
            break;
        }
    }
    
	uint8_t scan_code = in8(0x60);
    printk("scan code %x\n", scan_code);
    extension->raw_data = scan_code;

    /* 直接解析数据 */
    //mouse_parse(extension);

    return 0;
}

void wait_then_write(u8 port, u8 data)
{
    prepare_for_output();
    out8(port, data);
}


u8 wait_then_read(u8 port)
{
    prepare_for_input();
    return in8(port);
}


void initialize()
{
    // Enable PS aux port
    wait_then_write(I8042_STATUS, 0xa8);

    check_device_presence();

    if (m_device_present)
        initialize_device();
}


void check_device_presence()
{
    mouse_write(PS2MOUSE_REQUEST_SINGLE_PACKET);
    u8 maybe_ack = __mouse_read();
    if (maybe_ack == I8042_ACK) {
        m_device_present = true;
        printk("PS2MouseDevice: Device detected\n");

        // the mouse will send a packet of data, since that's what we asked
        // for. we don't care about the content.
        __mouse_read();
        __mouse_read();
        __mouse_read();
    } else {
        m_device_present = false;
        printk("PS2MouseDevice: Device not detected\n");
    }
}


void initialize_device()
{
    if (!m_device_present)
        return;
    printk("Enable interrupts\n");

    // Enable interrupts
    wait_then_write(I8042_STATUS, 0x20);

    printk("Enable the PS/2 mouse IRQ (12)\n");

    // Enable the PS/2 mouse IRQ (12).
    // NOTE: The keyboard uses IRQ 1 (and is enabled by bit 0 in this register).
    u8 status = wait_then_read(I8042_BUFFER) | 2;
    wait_then_write(I8042_STATUS, 0x60);
    wait_then_write(I8042_BUFFER, status);
    printk("Set default settings.\n");

    // Set default settings.
    mouse_write(PS2MOUSE_SET_DEFAULTS);
    expect_ack();

    printk("Enable.\n");

    // Enable.
    mouse_write(PS2MOUSE_ENABLE_PACKET_STREAMING);
    expect_ack();
    printk("Get device id.\n");
    mouse_write(PS2MOUSE_GET_DEVICE_ID);
    expect_ack();
    u8 device_id = __mouse_read();
    printk("ID=%x.\n", device_id);

    if (device_id != PS2MOUSE_INTELLIMOUSE_ID) {
        // Send magical wheel initiation sequence.
        mouse_write(PS2MOUSE_SET_SAMPLE_RATE);
        expect_ack();
        mouse_write(200);
        expect_ack();
        mouse_write(PS2MOUSE_SET_SAMPLE_RATE);
        expect_ack();
        mouse_write(100);
        expect_ack();
        mouse_write(PS2MOUSE_SET_SAMPLE_RATE);
        expect_ack();
        mouse_write(80);
        expect_ack();

        mouse_write(PS2MOUSE_GET_DEVICE_ID);
        expect_ack();
        device_id = __mouse_read();
    }

    if (device_id == PS2MOUSE_INTELLIMOUSE_ID) {
        m_has_wheel = true;
        printk("PS2MouseDevice: Mouse wheel enabled!\n");
    } else {
        printk("PS2MouseDevice: No mouse wheel detected!\n");
    }

    /* 注册时钟中断并打开中断，因为设定硬件过程中可能产生中断，所以要提前打开 */	
	register_irq(12, mouse_handler, IRQF_DISABLED, "IRQ12_MOUSE", DRV_NAME, (unsigned long )0);
    
    //enable_irq();
}


void expect_ack()
{
    u8 data = __mouse_read();
    ASSERT(data == I8042_ACK);
}



void prepare_for_input()
{
    int timeout = 10000;
    for (;timeout > 0; --timeout) {
        if (in8(I8042_STATUS) & 1)
            return;
    }
}


void prepare_for_output()
{
    int timeout = 10000;
    for (;timeout > 0; --timeout) {
        if (!(in8(I8042_STATUS) & 2))
            return;
    }
}

void mouse_write(u8 data)
{
    prepare_for_output();
    out8(I8042_STATUS, 0xd4);
    prepare_for_output();
    out8(I8042_BUFFER, data);
}

u8 __mouse_read()
{
    prepare_for_input();
    return in8(I8042_BUFFER);
}



iostatus_t mouse_read(device_object_t *device, io_request_t *ioreq)
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
#ifdef DEBUG_DRV
    #ifdef DEBUG_INFO
            printk(KERN_DEBUG "mouse even get: type=%d code=%x value=%d\n", even->type, even->code, even->value);
            printk(KERN_DEBUG "mouse even buf: head=%d tail=%d\n", ext->evbuf.head, ext->evbuf.tail);
    #endif
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

iostatus_t mouse_devctl(device_object_t *device, io_request_t *ioreq)
{
    //device_extension_t *extension = device->device_extension;

    unsigned int ctlcode = ioreq->parame.devctl.code;
    iostatus_t status;

    switch (ctlcode)
    {
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
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

	memset(&devext->mouse_data, 0, sizeof(struct mouse_data));

	devext->raw_data = 0;
    devext->phase = 0;  /* 步骤0 */
    devext->seq = 0;
    input_even_init(&devext->evbuf);

    devext->button_record = 0;

    mouse_ext = devext;
    /* 注册时钟中断并打开中断，因为设定硬件过程中可能产生中断，所以要提前打开 */	
	//register_irq(devext->irq, mouse_handler, IRQF_DISABLED, "IRQ12_MOUSE", DRV_NAME, (unsigned long )devext);
    
    initialize();

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

iostatus_t mouse_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = mouse_enter;
    driver->driver_exit = mouse_exit;

    driver->dispatch_function[IOREQ_READ] = mouse_read;
    driver->dispatch_function[IOREQ_DEVCTL] = mouse_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    printk(KERN_DEBUG "mouse_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void ps2mouse_driver_entry(void)
{
    if (driver_object_create(mouse_driver_func) < 0) {
        printk(KERN_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(ps2mouse_driver_entry);
