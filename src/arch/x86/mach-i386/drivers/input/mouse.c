#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/fifoio.h>
#include <xbook/task.h>
#include <xbook/spinlock.h>
#include <math.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>

#define DRV_NAME "input-mouse"
#define DRV_VERSION "0.1"

#define DEV_NAME "mouse"

// #define DEBUG_DRV

#define DEV_FIFO_BUF_LEN     64

#define USE_THREAD  1

/* 键盘控制器端口 */
enum kbd_controller_port {
    KBC_READ_DATA   = 0x60,     /* 读取数据端口(R) */
    KBC_WRITE_DATA  = 0x60,     /* 写入数据端口(W) */
    KBC_STATUS      = 0x64,     /* 获取控制器状态(R) */
    KBC_CMD         = 0x64,     /* 向控制器发送命令(W) */
};

/* 键盘控制器的命令 */
enum kbd_controller_cmds {
    KBC_CMD_READ_CONFIG     = 0x20,     /* 读取配置命令 */
    KBC_CMD_WRITE_CONFIG    = 0x60,     /* 写入配置命令 */
    KBC_CMD_DISABLE_MOUSE   = 0xA7,     /* 禁止鼠标端口 */
    KBC_CMD_ENABLE_MOUSE    = 0xA8,     /* 开启鼠标端口 */
    KBC_CMD_DISABLE_KEY     = 0xAD,     /* 禁止键盘通信，自动复位1控制器状态的第4位 */
    KBC_CMD_ENABLE_KEY      = 0xAE,     /* 开启键盘通信，自动置位0控制器状态的第4位 */
    KBC_CMD_SEND_TO_MOUSE   = 0xD4,     /* 向鼠标发送数据 */
    KBC_CMD_REBOOT_SYSTEM   = 0xFE,     /* 系统重启 */    
};

/* 键盘配置位 */
enum kbd_controller_config {
    KBC_CFG_ENABLE_KEY_INTR     = (1 << 0), /* bit 0=1: 使能键盘中断IRQ1(IBE) */
    KBC_CFG_ENABLE_MOUSE_INTR   = (1 << 1), /* bit 1=1: 使能鼠标中断IRQ12(MIBE) */
    KBC_CFG_INIT_DONE           = (1 << 2), /* bit 2=1: 设置状态寄存器的位2 */
    KBC_CFG_IGNORE_STATUS_BIT4  = (1 << 3), /* bit 3=1: 忽略状态寄存器中的位4 */
    KBC_CFG_DISABLE_KEY         = (1 << 4), /* bit 4=1: 禁止键盘 */
    KBC_CFG_DISABLE_MOUSE       = (1 << 5), /* bit 5=1: 禁止鼠标 */
    KBC_CFG_SCAN_CODE_TRANS     = (1 << 6), /* bit 6=1: 将第二套扫描码翻译为第一套 */
    /* bit 7 保留为0 */
};

/* 键盘控制器状态位 */
enum kbd_controller_status {
    KBC_STATUS_OUT_BUF_FULL     = (1 << 0), /* OUT_BUF_FULL: 输出缓冲器满置1，CPU读取后置0 */
    KBC_STATUS_INPUT_BUF_FULL   = (1 << 1), /* INPUT_BUF_FULL: 输入缓冲器满置1，i8042 取走后置0 */
    KBC_STATUS_SYS_FLAG         = (1 << 2), /* SYS_FLAG: 系统标志，加电启动置0，自检通过后置1 */
    KBC_STATUS_CMD_DATA         = (1 << 3), /* CMD_DATA: 为1，输入缓冲器中的内容为命令，为0，输入缓冲器中的内容为数据。 */
    KBC_STATUS_KYBD_INH         = (1 << 4), /* KYBD_INH: 为1，键盘没有被禁止。为0，键盘被禁止。 */
    KBC_STATUS_TRANS_TMOUT      = (1 << 5), /* TRANS_TMOUT: 发送超时，置1 */
    KBC_STATUS_RCV_TMOUT        = (1 << 6), /* RCV-TMOUT: 接收超时，置1 */
    KBC_STATUS_PARITY_EVEN      = (1 << 7), /* PARITY-EVEN: 从键盘获得的数据奇偶校验错误 */
};

/* 键盘控制器发送命令后 */
enum kbd_controller_return_code {
    /* 当击键或释放键时检测到错误时，则在Output Bufer后放入此字节，
    如果Output Buffer已满，则会将Output Buffer的最后一个字节替代为此字节。
    使用Scan code set 1时使用00h，Scan code 2和Scan Code 3使用FFh。 */
    KBC_RET_KEY_ERROR_00    = 0x00,
    KBC_RET_KEY_ERROR_FF    = 0xFF,
    
    /* AAH, BAT完成代码。如果键盘检测成功，则会将此字节发送到8042 Output Register中。 */
    KBC_RET_BAT_OK          = 0xAA,

    /* EEH, Echo响应。mouse使用EEh响应从60h发来的Echo请求。 */
    KBC_RET_ECHO            = 0xEE,

    /* F0H, 在Scan code set 2和Scan code set 3中，被用作Break Code的前缀。*/
    KBC_RET_BREAK           = 0xF0,
    /* FAH, ACK。当mouse任何时候收到一个来自于60h端口的合法命令或合法数据之后，
    都回复一个FAh。 */
    KBC_RET_ACK             = 0xFA,
    
    /* FCH, BAT失败代码。如果键盘检测失败，则会将此字节发送到8042 Output Register中。 */
    KBC_RET_BAT_BAD         = 0xFC,

    /* FEH, 当mouse任何时候收到一个来自于60h端口的非法命令或非法数据之后，
    或者数据的奇偶交验错误，都回复一个FEh，要求系统重新发送相关命令或数据。 */
    KBC_RET_RESEND          = 0xFE,
};

/* 单独发送给鼠标的命令，有别于键盘控制器命令
首先得在命令端口0x64发送一个命令，把数据发送到鼠标的命令，0xD4，
这样0x60中的数据才是发送到鼠标，不然就是发送到键盘的。
这些命令是发送到数据端口0x60，而不是命令0x64，
如果有参数，就在发送一次到0x60即可。
 */
enum mouse_cmds {
    /*
    大部分鼠标设备的ID号为0，部分鼠标根据设备ID号会有不同的数据包，
    ID为3时是3B数据包，ID为4时是4B两张数据包。
    */
    MOUSE_CMD_GET_DEVICE_ID =  0xF2,   /* 获取键盘设备的ID号 */                
    MOUSE_CMD_ENABLE_SEND   =  0xF4,   /* 允许鼠标设备发送数据包 */
    MOUSE_CMD_DISABLE_SEND  =  0xF5,   /* 禁止鼠标设备发送数据包 */
    MOUSE_CMD_RESTART       =  0xFF,   /* 重启鼠标 */
};


/* 键盘控制器配置 */
#define KBC_CONFIG	(KBC_CFG_ENABLE_KEY_INTR | KBC_CFG_ENABLE_MOUSE_INTR | \
                    KBC_CFG_INIT_DONE | KBC_CFG_SCAN_CODE_TRANS)

/* 等待键盘控制器可写入，当输入缓冲区为空后才可以写入 */
#define WAIT_KBC_WRITE()    while (in8(KBC_STATUS) & KBC_STATUS_INPUT_BUF_FULL)
/* 等待键盘控制器可读取，当输出缓冲区为空后才可以读取 */
#define WAIT_KBC_READ()    while (in8(KBC_STATUS) & KBC_STATUS_OUT_BUF_FULL)


/* 鼠标数据包 */
struct mouse_data {
	/* 第一字节数据内容
	7: y overflow, 6: x overflow, 5: y sign bit, 4: x sign bit,
	3: alawys 1  , 2: middle btn, 1: right btn , 0: left btn  .
	*/
	unsigned char byte0;
	unsigned char byte1;		// x 移动值
	unsigned char byte2;		// y 移动值
	unsigned char byte3;		// z 移动值
	/*
	7: y always 0, 6: always 0, 5: 鼠标第5键, 4: 鼠标第4键,
	3: 滚轮水平左滚动, 2: 滚轮水平右滚动, 1: 滚轮垂直下滚动, 0: 滚轮垂直上滚动.
	*/
	unsigned char byte4;
};

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    char irq;           /* irq号 */
    
    struct mouse_data mouse_data;
    char phase;		                                        /* 解析步骤 */
    uint8_t raw_data;                                       /* 原始数据 */
    input_even_buf_t evbuf;     /* 事件缓冲区 */
#if USE_THREAD == 1
    fifo_io_t fifoio;
#endif

    /* 按键记录:
     bit 0 置1表示左键已经按下。
     bit 1 置1表示右键已经按下。
     bit 2 置1表示中键已经按下。
    */
    uint8_t button_record;      
    int seq;
} device_extension_t;

#if USE_THREAD == 1
/**
 * get_bytes_from_buf - 从键盘缓冲区中读取下一个字节
 */
static unsigned char get_bytes_from_buf(device_extension_t *ext)       
{
    unsigned char scan_code;
    /* 从队列中获取一个数据 */
    scan_code = fifo_io_get(&ext->fifoio);
    return scan_code;
}
#endif
/**
 * mouse_parse - 解析鼠标数据
 * 
 * 返回-1，表明解析出错。
 * 返回0，表明解析成功，但还没有获取完整的数据包
 * 返回1，表明解析成功，获取了完整的数据包
*/
static int mouse_parse(device_extension_t *extension)
{
    if (extension->phase == 0) {
        /* 打开中断后产生的第一个数据是ACK码，然后才开始数据传输 */
		if (extension->raw_data == KBC_RET_ACK) {
			extension->phase++;
        }
		return 0;
	}
	if (extension->phase == 1) {
        extension->mouse_data.byte0 = extension->raw_data;
        extension->phase++;
		return 0;
	}
	if (extension->phase == 2) {
		extension->mouse_data.byte1 = extension->raw_data;
		extension->phase++;
		return 0;
	}
	if (extension->phase == 3) {
		extension->mouse_data.byte2 = extension->raw_data;
		extension->phase = 1;

#ifdef DEBUG_DRV		
        printk(KERN_DEBUG "[%d](B:%x, X:%d, Y:%d)\n", 
            ++extension->seq, extension->mouse_data.byte0, (char)extension->mouse_data.byte1,
            (char)extension->mouse_data.byte2);
#endif
        input_event_t e;
        /* 水平方向 */
        if (extension->mouse_data.byte1) {  /* x轴有变化 */
            e.type = EV_REL;
            e.code = REL_X;
            e.value = extension->mouse_data.byte1;
            /* 如果x有符号，那么就添加上符号 */
            if ((extension->mouse_data.byte0 & 0x10) != 0)
                e.value |= 0xffffff00;
            
            /* 推送事件 */
            input_even_put(&extension->evbuf, &e);
        }
        /* 垂直方向 */
        if (extension->mouse_data.byte2) {  /* x轴有变化 */
            e.type = EV_REL;
            e.code = REL_Y;
            e.value = extension->mouse_data.byte2;
            /* 如果y有符号，那么就添加上符号 */
            if ((extension->mouse_data.byte0 & 0x20) != 0)
                e.value |= 0xffffff00;
            
            /* y增长是反向的，所以要取反 */
            e.value = -e.value;
            
            /* 推送事件 */
            input_even_put(&extension->evbuf, &e);
        }
        /* 纵深方向 */
        if (extension->mouse_data.byte3) {  /* z轴有变化 */
            e.type = EV_REL;
            e.code = REL_WHEEL;
            e.value = extension->mouse_data.byte3;
            
            /* 推送事件 */
            input_even_put(&extension->evbuf, &e);
        }
        /* 同步鼠标移动 */
        e.type = EV_SYN;
        e.code = 0;
        e.value = 0;
        /* 推送事件 */
        input_even_put(&extension->evbuf, &e);
#if 1		
        /* 按钮 */
        extension->mouse_data.byte0 &= 0x07; /* 只取低3位 */
        if (extension->mouse_data.byte0 & 0x01) {   /* left button */
            if (!(extension->button_record & 0x01)) {   /* left button not pressed */
                extension->button_record |= 0x01; /* record */

                /* 鼠标左键按下 */
                e.type = EV_KEY;
                e.code = BTN_LEFT;
                e.value = 1;    /* 1表示按下 */
                /* 推送事件 */
                input_even_put(&extension->evbuf, &e);
            }
        } else {    /* no left button */
            /* 如果上次是按下，这次又没有按下，就是弹起 */
            if (extension->button_record & 0x01) {
                extension->button_record &= ~0x01; /* clear record */
                
                /* 鼠标左键弹起 */
                e.type = EV_KEY;
                e.code = BTN_LEFT;
                e.value = 0;    /* 0表示弹起 */
                /* 推送事件 */
                input_even_put(&extension->evbuf, &e);
            }
        } 
        if (extension->mouse_data.byte0 & 0x02) {   /* right button */
            if (!(extension->button_record & 0x02)) {   /* right button not pressed */
                extension->button_record |= 0x02; /* record */

                /* 鼠标右键按下 */
                e.type = EV_KEY;
                e.code = BTN_RIGHT;
                e.value = 1;    /* 1表示按下 */
                /* 推送事件 */
                input_even_put(&extension->evbuf, &e);
            }
        } else {    /* no right button */
            /* 如果上次是按下，这次又没有按下，就是弹起 */
            if (extension->button_record & 0x02) {
                extension->button_record &= ~0x02; /* clear record */
                
                /* 鼠标左键弹起 */
                e.type = EV_KEY;
                e.code = BTN_RIGHT;
                e.value = 0;    /* 0表示弹起 */
                /* 推送事件 */
                input_even_put(&extension->evbuf, &e);
            }
        } 
        if (extension->mouse_data.byte0 & 0x04) {   /* middle button */
            if (!(extension->button_record & 0x04)) {   /* middle button not pressed */
                extension->button_record |= 0x04; /* record */

                /* 鼠标中键按下 */
                e.type = EV_KEY;
                e.code = BTN_MIDDLE;
                e.value = 1;    /* 1表示按下 */
                /* 推送事件 */
                input_even_put(&extension->evbuf, &e);
            }
        } else {    /* no middle button */
            /* 如果上次是按下，这次又没有按下，就是弹起 */
            if (extension->button_record & 0x04) {
                extension->button_record &= ~0x04; /* clear record */
                
                /* 鼠标左键弹起 */
                e.type = EV_KEY;
                e.code = BTN_MIDDLE;
                e.value = 0;    /* 0表示弹起 */
                /* 推送事件 */
                input_even_put(&extension->evbuf, &e);
            }
        }
#endif
        return 1;
	}
	return -1; 
}

/**
 * mouse_handler - 鼠标中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int mouse_handler(unsigned long irq, unsigned long data)
{
    device_extension_t *extension = (device_extension_t *) data;
	/* 先从硬件获取按键数据 */
	uint8_t scan_code = in8(KBC_READ_DATA);

    extension->raw_data = scan_code;

#if USE_THREAD == 1
    /* 把数据放到io队列 */
    fifo_io_put(&extension->fifoio, scan_code);
#else
    /* 直接解析数据 */
    mouse_parse(extension);
#endif
    return 0;
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
#if USE_THREAD == 1
/* 用内核线程来处理到达的数据 */
void mouse_thread(void *arg) {
    device_extension_t *ext = (device_extension_t *) arg;
    unsigned int key;
    while (1) {
        ext->raw_data = get_bytes_from_buf(ext);
        mouse_parse(ext);
    }
}
#endif


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
#if USE_THREAD == 1
    unsigned char *buf = kmalloc(DEV_FIFO_BUF_LEN);
    if (buf == NULL) {
        status = IO_FAILED;
        printk(KERN_DEBUG "%s: alloc buf failed!\n", __func__);
        return status;
    }
    fifo_io_init(&devext->fifoio, buf, DEV_FIFO_BUF_LEN);
#endif
    
    /* 注册时钟中断并打开中断，因为设定硬件过程中可能产生中断，所以要提前打开 */	
	register_irq(devext->irq, mouse_handler, IRQF_DISABLED, "IRQ12_MOUSE", DRV_NAME, (unsigned long )devext);
    
    /* 开启鼠标端口 */
    WAIT_KBC_WRITE();
	out8(KBC_CMD, KBC_CMD_ENABLE_MOUSE);
	
	/* 设置发送数据给鼠标 */
	WAIT_KBC_WRITE();
	out8(KBC_CMD, KBC_CMD_SEND_TO_MOUSE);
	
	/* 传递打开鼠标的数据传输 */
	WAIT_KBC_WRITE();
	out8(KBC_WRITE_DATA, MOUSE_CMD_ENABLE_SEND);
	
    /* 设置鼠标与键盘使能以及IRQ中断开启 */
    WAIT_KBC_WRITE();
	out8(KBC_CMD, KBC_CMD_WRITE_CONFIG);
	
    /* 配置键盘控制器的值 */
    WAIT_KBC_WRITE();
	out8(KBC_WRITE_DATA, KBC_CONFIG);

#if USE_THREAD == 1    
    /* 启动一个内核线程来处理数据 */
    kthread_start("mouse", TASK_PRIO_RT, mouse_thread, devext);
#endif
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
