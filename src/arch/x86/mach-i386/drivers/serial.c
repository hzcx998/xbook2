#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/mdl.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <stdio.h>

#define DRV_NAME "uart-serial"
#define DRV_VERSION "0.1"

#define DEV_NAME "com"

/* 传输方法：0->user, 1->buffered, 2->direct  */
#define TRANS_METHOD 1

// #define DEBUG_DRV

/* 串口的地址是IO地址 */
#define COM1_BASE   0X3F8
#define COM2_BASE   0X2F8
#define COM3_BASE   0X3E8
#define COM4_BASE   0X2E8

/* 最大波特率值 */
#define MAX_BAUD_VALUE  11520

/* 波特率值表：
Baud Rate   | Divisor 
50          | 2304  
110         | 1047  
220         | 524  
300         | 384  
600         | 192  
1200        | 96  
2400        | 48  
4800        | 24  
9600        | 12  
19200       | 6  
38400       | 3  
57600       | 2  
115200      | 1  

Divisor计算方法：
Divisor = 115200 / BaudRate 
*/

/* 默认波特率值 */
#define DEFAULT_BAUD_VALUE  11520
#define DEFAULT_DIVISOR_VALUE (MAX_BAUD_VALUE / DEFAULT_BAUD_VALUE)

#define SERIAL_IRQ_4    IRQ4
#define SERIAL_IRQ_3    IRQ3

enum FifoControlRegisterBits {
    FIFO_ENABLE = 1,                             /* Enable FIFOs */
    FIFO_CLEAR_RECEIVE   = (1 << 1),             /* Clear Receive FIFO */
    FIFO_CLEAR_TRANSMIT  = (1 << 2),             /* Clear Transmit FIFO */
    FIFO_DMA_MODE_SELECT = (1 << 3),             /* DMA Mode Select */
    FIFO_RESERVED        = (1 << 4),             /* Reserved */
    FIFO_ENABLE_64       = (1 << 5),             /* Enable 64 Byte FIFO(16750) */
    /* Interrupt Trigger Level/Trigger Level  */
    FIFO_TRIGGER_1       = (0 << 6),             /* 1 Byte */
    FIFO_TRIGGER_4       = (1 << 6),             /* 4 Byte */
    FIFO_TRIGGER_8       = (1 << 7),             /* 8 Byte */
    FIFO_TRIGGER_14      = (1 << 6) | (1 << 7),  /* 14 Byte */
};

enum LineControlRegisterBits {
    /* Word Length */
    LINE_WORD_LENGTH_5   = 0,                    /* 5 Bits */
    LINE_WORD_LENGTH_6   = 1,                    /* 6 Bits */
    LINE_WORD_LENGTH_7   = (1 << 1),             /* 7 Bits */
    LINE_WORD_LENGTH_8   = ((1 << 1) | 1),       /* 8 Bits */
    LINE_STOP_BIT_1      = (0 << 2),             /* One Stop Bit */
    LINE_STOP_BIT_2      = (1 << 2),             /* 1.5 Stop Bits or 2 Stop Bits */
        /* Parity Select */
    LINE_PARITY_NO       = (0 << 3),             /* No Parity */
    LINE_PARITY_ODD      = (1 << 3),             /* Odd Parity */
    LINE_PARITY_EVEN     = (1 << 3) | (1 << 4),  /* Even Parity */
    LINE_PARITY_MARK     = (1 << 3) | (1 << 5),  /* Mark */
    LINE_PARITY_SPACE    = (1 << 3) | (1 << 4) | (1 << 5), /* Space */
    LINE_BREAK_ENABLE    = (1 << 6),             /* Set Break Enable */
    LINE_DLAB            = (1 << 7),             /* Divisor Latch Access Bit */
};
enum InterruptEnableRegisterBits {
    INTR_RECV_DATA_AVALIABLE = 1,        /* Enable Received Data Available Interrupt */
    INTR_TRANSMIT_HOLDING    = (1 << 1), /* Enable Transmitter Holding Register Empty Interrupt */
    INTR_RECV_LINE_STATUS    = (1 << 2), /* Enable Receiver Line Status Interrupt */
    INTR_MODEM_STATUS        = (1 << 3), /* Enable Modem Status Interrupt */
    INTR_SLEEP_MODE          = (1 << 4), /* Enable Sleep Mode(16750) */
    INTR_LOW_POWER_MODE      = (1 << 5), /* Enable Low Power Mode(16750) */
    INTR_RESERVED1           = (1 << 6), /* Reserved */
    INTR_RESERVED2           = (1 << 7), /* Reserved */
};

enum LineStatusRegisterBits {
    LINE_STATUS_DATA_READY                  = 1,        /* Data Ready */
    LINE_STATUS_OVERRUN_ERROR               = (1 << 1), /* Overrun Error */
    LINE_STATUS_PARITY_ERROR                = (1 << 2), /* Parity Error */
    LINE_STATUS_FRAMING_ERROR               = (1 << 3), /* Framing Error */
    LINE_STATUS_BREAK_INTERRUPT             = (1 << 4), /* Break Interrupt */
    LINE_STATUS_EMPTY_TRANSMITTER_HOLDING   = (1 << 5), /* Empty Transmitter Holding Register */
    LINE_STATUS_EMPTY_DATA_HOLDING          = (1 << 6), /* Empty Data Holding Registers */
    LINE_STATUS_ERROR_RECEIVE_FIFO          = (1 << 7), /* Error in Received FIFO */
};

enum intr_indenty_regBits {
    INTR_STATUS_PENDING_FLAG        = 1,        /* Interrupt Pending Flag */
    /* 产生的什么中断 */                
    INTR_STATUS_MODEM               = (0 << 1), /* Transmitter Holding Register Empty Interrupt	 */
    INTR_STATUS_TRANSMITTER_HOLDING = (1 << 1), /* Received Data Available Interrupt */
    INTR_STATUS_RECEIVE_DATA        = (1 << 2), /* Received Data Available Interrupt */
    INTR_STATUS_RECEIVE_LINE        = (1 << 1) | (1 << 2),  /* Receiver Line Status Interrupt */
    INTR_STATUS_TIME_OUT_PENDING    = (1 << 2) | (1 << 3),  /* Time-out Interrupt Pending (16550 & later) */
    INTR_STATUS_64BYTE_FIFO         = (1 << 5), /* 64 Byte FIFO Enabled (16750 only) */
    INTR_STATUS_NO_FIFO             = (0 << 6), /* No FIFO on chip */
    INTR_STATUS_RESERVED_CONDITION  = (1 << 6), /* Reserved condition */
    INTR_STATUS_FIFO_NOT_FUNC       = (1 << 7), /* FIFO enabled, but not functioning */
    INTR_STATUS_FIFO                = (1 << 6) | (1 << 7),  /* FIFO enabled */
};

/* 最多有4个串口 */
#define MAX_COM_NR  4 

typedef struct _device_extension {
    string_t device_name;           /* 设备名字 */
    device_object_t *device_object; /* 设备对象 */
    char irq;           /* irq号 */

    /* ----串口的寄存器---- */
    uint16_t iobase;                    /* IO基地址 */
    uint16_t data_reg;              /* 数据寄存器，读数据/写数据 */
    uint16_t divisor_low_reg;        /* 除数寄存器低8位 */
    uint16_t intr_enable_reg;   /* 中断使能寄存器 */
    uint16_t divisor_high_reg;       /* 除数寄存器高8位 */
    uint16_t intr_indenty_reg;       /* 中断标识寄存器 */
    uint16_t fifo_reg;       /* fifo寄存器 */
    uint16_t line_ctrl_reg;       /* 行控制寄存器 */
    uint16_t modem_ctrl_reg;       /* 调制解调器控制寄存器 */
    uint16_t line_status_reg;       /* 行状态寄存器 */
    uint16_t modem_status_reg;       /* 调制解调器状态寄存器 */
    uint16_t scratch_reg;       /* 刮伤寄存器 */
    uint8_t id;           /* 串口id，对应着哪个串口 */
} device_extension_t;

iostatus_t serial_open(device_object_t *device, io_request_t *ioreq)
{
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

iostatus_t serial_close(device_object_t *device, io_request_t *ioreq)
{
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

/**
 * uart_recv - 从串口接收数据
 * @devext: 私有结构体指针
 * 
 * @return: 返回收到的数据
 */
static uint8_t uart_recv(device_extension_t *devext)
{
    /* 如果接收的时候数据没有准备好，就不能接收 */
    while (!(in8(devext->line_status_reg) & 
            LINE_STATUS_DATA_READY));

    /* 从数据端口读取数据 */
    return in8(devext->data_reg);
}

/**
 * uart_send - 串口发送数据
 * @devext: 私有结构体指针
 * @data: 传输的数据
 */
static int uart_send(device_extension_t *devext, uint8_t data)
{
    int timeout = 100000;
    /* 如果发送的时候不持有传输状态，就不能发送 */
    while (!(in8(devext->line_status_reg) & 
        LINE_STATUS_EMPTY_TRANSMITTER_HOLDING) && timeout--);

    /* 往数据端口写入数据 */
    out8(devext->data_reg, data);
    return 0;
}

iostatus_t serial_read(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len = ioreq->parame.read.length;
    
    uint8_t *buf = (uint8_t *)ioreq->system_buffer; 
    int i = len;
    
    while (i > 0) {
        *buf = uart_recv(device->device_extension);
        i--;
        buf++;
    }
#ifdef DEBUG_DRV    
    buf = (uint8_t *)ioreq->system_buffer; 
    kprint(PRINT_DEBUG "serial_write: %s\n", buf);
#endif
    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;
    
    /* 调用完成请求 */
    io_complete_request(ioreq);

    return IO_SUCCESS;
}

iostatus_t serial_write(device_object_t *device, io_request_t *ioreq)
{
    unsigned long len = ioreq->parame.write.length;
    
    uint8_t *buf = (uint8_t *)ioreq->system_buffer; 
    int i = len;
#ifdef DEBUG_DRV    
    kprint(PRINT_DEBUG "serial_write: %s\n", buf);
#endif
    while (i > 0) {
        uart_send(device->device_extension, *buf);
        i--;
        buf++;
    }

    ioreq->io_status.status = IO_SUCCESS;
    ioreq->io_status.infomation = len;
    /* 调用完成请求 */
    io_complete_request(ioreq);
    return IO_SUCCESS;
}

iostatus_t serial_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;

    iostatus_t status;

    switch (ctlcode)
    {
    case DEVCTL_CODE_TEST:
#ifdef DEBUG_DRV
        kprint(PRINT_DEBUG "serial_devctl: code=%x arg=%x\n", ctlcode, ioreq->parame.devctl.arg);
#endif
        status = IO_SUCCESS;
        break;
    default:
        status = IO_FAILED;
        break;
    }
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;
    io_complete_request(ioreq);
    return status;
}

static iostatus_t serial_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *devext;
    char irq;
    uint16_t iobase;
    
    int id;
    char devname[DEVICE_NAME_LEN] = {0};

    for (id = 0; id < MAX_COM_NR; id++) {
        sprintf(devname, "%s%d", DEV_NAME, id);
        /* 初始化一些其它内容 */
        status = io_create_device(driver, sizeof(device_extension_t), devname, DEVICE_TYPE_SERIAL_PORT, &devobj);

        if (status != IO_SUCCESS) {
            kprint(PRINT_ERR "serial_enter: create device failed!\n");
            return status;
        }
        /* buffered io mode */
        devobj->flags = DO_BUFFERED_IO;

        devext = (device_extension_t *)devobj->device_extension;
        string_new(&devext->device_name, devname, DEVICE_NAME_LEN);
        devext->device_object = devobj;
#ifdef DEBUG_DRV
        kprint(PRINT_DEBUG "serial_enter: device extension: device name=%s object=%x\n",
            devext->device_name.text, devext->device_object);
#endif        
        /* 根据ID设置iobase和irq */
        switch (id)
        {
        case 0:
            iobase = COM1_BASE;
            irq = SERIAL_IRQ_4;
            break;
        case 1:
            iobase = COM2_BASE;
            irq = SERIAL_IRQ_3;
            break;
        case 2:
            iobase = COM3_BASE;
            irq = SERIAL_IRQ_4;
            break;
        case 3:
            iobase = COM4_BASE;
            irq = SERIAL_IRQ_3;
            break;
        default:
            break;
        }

        /* 串口的寄存器参数设置(对齐后很好看！！！) */
        devext->iobase           = iobase;
        devext->data_reg         = iobase + 0;
        devext->divisor_low_reg  = iobase + 0;
        devext->intr_enable_reg  = iobase + 1;
        devext->divisor_high_reg = iobase + 1;
        devext->intr_indenty_reg = iobase + 2;
        devext->line_ctrl_reg    = iobase + 3;
        devext->modem_ctrl_reg   = iobase + 4;
        devext->line_status_reg  = iobase + 5;
        devext->modem_status_reg = iobase + 6;
        devext->scratch_reg      = iobase + 7;

        /* 设置串口的id */
        devext->id = id;

        /* irq号 */
        devext->irq = irq;
#ifdef DEBUG_DRV
        kprint(PRINT_DEBUG "serial_enter: com%d, base:%x irq:%d\n", id, iobase, irq);
#endif  /* DEBUG_SERIAL */
        
        /* ----执行设备的初始化---- */

        /* 设置可以更改波特率Baud */
        out8(devext->line_ctrl_reg, LINE_DLAB);

        /* Set Baud rate to 115200，设置除数寄存器为新的波特率值 */
        out8(devext->divisor_low_reg, low8(DEFAULT_DIVISOR_VALUE));
        out8(devext->divisor_high_reg, high8(DEFAULT_DIVISOR_VALUE));
        
        /* 设置 DLAB to 0, 设置字符宽度为 8, 停字为 to 1, 没有奇偶校验, 
        Break signal Disabled */
        out8(devext->line_ctrl_reg, LINE_WORD_LENGTH_8 | 
                LINE_STOP_BIT_1 | LINE_PARITY_NO);
        
        /* 关闭所有串口中断，所以无需注册中断号 */
        out8(devext->intr_enable_reg, INTR_RECV_DATA_AVALIABLE | 
            INTR_RECV_LINE_STATUS | INTR_LOW_POWER_MODE); 

        /* 设置FIFO，打开FIFO, 清除接收 FIFO, 清除传输 FIFO
        打开 64Byte FIFO, 中断触发等级为 14Byte
        */
        out8(devext->fifo_reg, FIFO_ENABLE | FIFO_CLEAR_TRANSMIT |
                    FIFO_CLEAR_RECEIVE | FIFO_ENABLE_64 | 
                    FIFO_TRIGGER_14);

        /* 无调制解调器设置 */            
        out8(devext->modem_ctrl_reg, 0x00);
        /* 无刮伤寄存器设置 */            
        out8(devext->scratch_reg, 0x00);
    }

    return IO_SUCCESS;
}

static iostatus_t serial_exit(driver_object_t *driver)
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

iostatus_t serial_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = serial_enter;
    driver->driver_exit = serial_exit;

    driver->dispatch_function[IOREQ_OPEN] = serial_open;
    driver->dispatch_function[IOREQ_CLOSE] = serial_close;
    driver->dispatch_function[IOREQ_READ] = serial_read;
    driver->dispatch_function[IOREQ_WRITE] = serial_write;
    driver->dispatch_function[IOREQ_DEVCTL] = serial_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    kprint(PRINT_DEBUG "serial_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    return status;
}

static __init void serial_driver_entry(void)
{
    if (driver_object_create(serial_driver_func) < 0) {
        kprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(serial_driver_entry);
