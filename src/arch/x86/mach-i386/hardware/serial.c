#include <arch/config.h>

#ifdef X86_SERIAL_HW

#include <xbook/debug.h>
#include <stdint.h>
#include <xbook/bitops.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <arch/config.h>

/* 串口的地址是IO地址 */
#define COM1_BASE   0X3F8
#define COM2_BASE   0X2F8
#define COM3_BASE   0X3E8
#define COM4_BASE   0X2E8

/* 最大波特率值 */
#define MAX_BAUD_VALUE  115200

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

#define DEFAULT_BAUD_VALUE  19200
#define DEFAULT_DIVISOR_VALUE (MAX_BAUD_VALUE / DEFAULT_BAUD_VALUE)

#define SERIAL_IRQ_4    IRQ4
#define SERIAL_IRQ_3    IRQ3

/* 串口调试端口的索引 */
#define SERIAL_DEGUB_IDX    0

#define SERIAL_SEND_TIMEOUT

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

enum line_ctrl_regBits {
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
enum intr_enable_regBits {
    INTR_RECV_DATA_AVALIABLE = 1,        /* Enable Received Data Available Interrupt */
    INTR_TRANSMIT_HOLDING    = (1 << 1), /* Enable Transmitter Holding Register Empty Interrupt */
    INTR_RECV_LINE_STATUS    = (1 << 2), /* Enable Receiver Line Status Interrupt */
    INTR_MODEM_STATUS        = (1 << 3), /* Enable Modem Status Interrupt */
    INTR_SLEEP_MODE          = (1 << 4), /* Enable Sleep Mode(16750) */
    INTR_LOW_POWER_MODE      = (1 << 5), /* Enable Low Power Mode(16750) */
    INTR_RESERVED1           = (1 << 6), /* Reserved */
    INTR_RESERVED2           = (1 << 7), /* Reserved */
};

enum line_status_regBits {
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

#define MAX_COM_NR  4 

/* 默认初始化的串口数 */
#define DEFAULT_COM_NR  2 

typedef struct {
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
} serial_hardware_t;

/* 4个串口 */
serial_hardware_t serial_obj;

static int serial_send(serial_hardware_t *obj, char data)
{
    #ifdef SERIAL_SEND_TIMEOUT
    int timeout = 0x100000;
    while (!(in8(obj->line_status_reg) & 
        LINE_STATUS_EMPTY_TRANSMITTER_HOLDING) && timeout--);
    #else
    while (!(in8(obj->line_status_reg) & 
        LINE_STATUS_EMPTY_TRANSMITTER_HOLDING));
    #endif
    out8(obj->data_reg, data);
    return 0;
}

void serial_hardware_putchar(char ch)
{
    /* 如果是回车，就需要发送一个'\r'，以兼d容unix/linux操作系统的输出 */
    if(ch == '\n') {
        serial_send(&serial_obj, '\r');
    }
	serial_send(&serial_obj, ch);
}

void serial_hardware_init()
{
    serial_hardware_t *obj = &serial_obj;
    char irq;
    uint16_t iobase;

    iobase = COM1_BASE;
    irq = 4;

    obj->iobase                         = iobase;
    obj->data_reg                       = iobase + 0;
    obj->divisor_low_reg                = iobase + 0;
    obj->intr_enable_reg                = iobase + 1;
    obj->divisor_high_reg               = iobase + 1;
    obj->intr_indenty_reg               = iobase + 2;
    obj->line_ctrl_reg                  = iobase + 3;
    obj->modem_ctrl_reg                 = iobase + 4;
    obj->line_status_reg                = iobase + 5;
    obj->modem_status_reg               = iobase + 6;
    obj->scratch_reg                    = iobase + 7;

    obj->irq = irq;

    out8(obj->line_ctrl_reg, LINE_DLAB);

    out8(obj->divisor_low_reg, low8(DEFAULT_DIVISOR_VALUE));
    out8(obj->divisor_high_reg, high8(DEFAULT_DIVISOR_VALUE));
    
    out8(obj->line_ctrl_reg, LINE_WORD_LENGTH_8 | 
            LINE_STOP_BIT_1 | LINE_PARITY_NO);
    
    /* 关闭所有串口中断，所以无需注册中断号 */
    out8(obj->intr_enable_reg, INTR_RECV_DATA_AVALIABLE | 
        INTR_RECV_LINE_STATUS | INTR_LOW_POWER_MODE); 

    out8(obj->fifo_reg, FIFO_ENABLE | FIFO_CLEAR_TRANSMIT |
                FIFO_CLEAR_RECEIVE | FIFO_ENABLE_64 | 
                FIFO_TRIGGER_14);
          
    out8(obj->modem_ctrl_reg, 0x00);          
    out8(obj->scratch_reg, 0x00);
}
#endif