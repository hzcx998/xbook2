#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <string.h>

#include <xbook/driver.h>
#include <xbook/task.h>
#include <xbook/virmem.h>
#include <xbook/clock.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define DRV_NAME "buzzer-pcspeaker"
#define DRV_VERSION "0.1"

#define DEV_NAME "buzzer"

// #define DEBUG_DRV

/*
PPI （Programmable Peripheral Interface）

060-067  8255 Programmable Peripheral Interface  (PC,XT, PCjr)
060 8255 Port A keyboard input/output buffer (output PCjr)
061 8255 Port B output
062 8255 Port C input
063 8255 Command/Mode control register
*/
#define PPI_KEYBOARD        0X60
#define PPI_OUTPUT          0X61
#define PPI_INPUT           0X62
#define PPI_COMMAND_MODE    0X63


/* PIT （Programmable Interval Timer）8253/8254 可编程中断计时器 */

/*
Port 40h, 8253 Counter 0 Time of Day Clock (normally mode 3)
*/
#define PIT_COUNTER0	0x40

/*
Port 41h, 8253 Counter 1 RAM Refresh Counter (normally mode 2)
*/
#define PIT_COUNTER1	0x41

/*
Port 42h, 8253 Counter 2 Cassette and Speaker Functions
*/
#define PIT_COUNTER2	0x42

/*
可编程中断计时器,控制字寄存器
Port 43h, 8253 Mode Control Register, data format:

	|7|6|5|4|3|2|1|0|  Mode Control Register
	 | | | | | | | `---- 0=16 binary counter, 1=4 decade BCD counter
	 | | | | `--------- counter mode bits
	 | | `------------ read/write/latch format bits
	 `--------------- counter select bits (also 8254 read back command)

Read Back Command Format  (8254 only)

	|7|6|5|4|3|2|1|0| Read Back Command (written to Mode Control Reg)
	 | | | | | | | `--- must be zero
	 | | | | | | `---- select counter 0
	 | | | | | `----- select counter 1
	 | | | | `------ select counter 2
	 | | | `------- 0 = latch status of selected counters
	 | | `-------- 0 = latch count of selected counters
	 `----------- 11 = read back command

Read Back Command Status (8254 only, read from counter register)

	|7|6|5|4|3|2|1|0|  Read Back Command Status
	 | | | | | | | `--- 0=16 binary counter, 1=4 decade BCD counter
	 | | | | `-------- counter mode bits (see Mode Control Reg above)
	 | | `----------- read/write/latch format (see Mode Control Reg)
	 | `------------ 1=null count (no count set), 0=count available
	 `------------- state of OUT pin (1=high, 0=low)

*/
#define PIT_CTRL	0x43


/* Mode Control Register */
enum CtrlModeBits {
    /* Bits 76 Counter Select Bits */
    PIT_MODE_COUNTER_0 = (0<<6),            /* 00  select counter 0 */
    PIT_MODE_COUNTER_1 = (1<<6),            /* 01  select counter 1 */
    PIT_MODE_COUNTER_2 = (1<<7),            /* 10  select counter 2 */
    PIT_MODE_READ_BACK = (1<<6)|(1<<7),     /* 11  read back command (8254 only, illegal on 8253, see below) */
    /* Bits 54  Read/Write/Latch Format Bits */
    PIT_MODE_LPCV = (0<<4),                 /* 00  latch present counter value */
    PIT_MODE_MSB = (1<<4),                  /* 01  read/write of MSB only */
    PIT_MODE_LSB = (1<<5),                  /* 10  read/write of LSB only */
    PIT_MODE_MSB_LSB = (1<<4)|(1<<5),       /* 11  read/write LSB, followed by write of MSB */
    /* Bits 321  Counter Mode Bits */
    
    /*
    000  mode 0, interrupt on terminal count;  countdown, interrupt,
	     then wait for a new mode or count; loading a new count in the
	     middle of a count stops the countdown
    */
    PIT_MODE_0 = (0<<1), 
    /*
    001  mode 1, programmable one-shot; countdown with optional
	     restart; reloading the counter will not affect the countdown
	     until after the following trigger
    */
    PIT_MODE_1 = (1<<1), 
    /*
    010  mode 2, rate generator; generate one pulse after 'count' CLK
	     cycles; output remains high until after the new countdown has
	     begun; reloading the count mid-period does not take affect
	     until after the period

    */
    PIT_MODE_2 = (1<<2), 
    /*
    011  mode 3, square wave rate generator; generate one pulse after
	     'count' CLK cycles; output remains high until 1/2 of the next
	     countdown; it does this by decrementing by 2 until zero, at
	     which time it lowers the output signal, reloads the counter
	     and counts down again until interrupting at 0; reloading the
	     count mid-period does not take affect until after the period

    */
    PIT_MODE_3 = (1<<1)|(1<<2), 
    /*
    100  mode 4, software triggered strobe; countdown with output high
	     until counter zero;  at zero output goes low for one CLK
	     period;  countdown is triggered by loading counter;  reloading
	     counter takes effect on next CLK pulse

    */
    PIT_MODE_4 = (1<<3), 
    /*
    101  mode 5, hardware triggered strobe; countdown after triggering
	     with output high until counter zero; at zero output goes low
	     for one CLK period
    */
    PIT_MODE_5 = (1<<1)|(1<<3), 
    
    /* Bits 0  Counter Mode Bits */
    PIT_MODE_BINARY = (0<<0),       /* 0 0= 16 binary counter */
    PIT_MODE_BCD = (1<<0),          /* 1 1= 4 decade BCD counter */
};

#define TIMER_FREQ     1193180 	/* 时钟的频率 */

/**
 * buzzer_on - 播放声音
 * 
 */
static void buzzer_on()
{
    uint8_t tmp;
    
    tmp = in8(PPI_OUTPUT);
    /* 开始播放，设定端口的低2位为1即可 */
    if (tmp != (tmp | 3)) {
 		out8(PPI_OUTPUT, tmp | 3);
 	}
}

/**
 * buzzer_off - 关闭声音
 * 
 */
static void buzzer_off()
{
    uint8_t tmp;
    
    tmp = in8(PPI_OUTPUT);
    /* 停止播放，需要设定端口的低2位为0 */
    out8(PPI_OUTPUT, tmp & 0xfc);
}

static void buzzer_set_freq(uint32_t frequence)
{
    if (frequence < 1) {
        frequence = 1;
    }
    if (frequence > 20000) {
        frequence = 20000;
    }
    
    uint32_t div;
 	
    // 求要传入的频率
 	div = TIMER_FREQ / (frequence * (HZ / 100));
 	
    /* 即将设置蜂鸣器的频率 */
    out8(PIT_CTRL, PIT_MODE_COUNTER_2 | PIT_MODE_MSB_LSB | 
            PIT_MODE_3 | PIT_MODE_BINARY);

    /* 设置低位和高位数据 */
 	out8(PIT_COUNTER2, (uint8_t) (div));
 	out8(PIT_COUNTER2, (uint8_t) (div >> 8));
}
#if 0
/**
 * buzzer_beep - 发出声音
 * @freq: 发声频率
 * 
 * 发声频率决定音高
 * 人能识别的HZ是20KHZ~20HZ，最低frequence取值20~20000
 */
static void buzzer_beep(uint32_t freq)
{
    /* 设置工作频率 */
 	buzzer_set_freq(freq);
    /* 播放 */
    buzzer_on();
}
#endif

static iostatus_t rtl8139_open(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;
    ioreq->io_status.status = status;
    ioreq->io_status.infomation = 0;

    buzzer_set_freq(20000);    /* 初始化为最高的频率，听不见 */

    io_complete_request(ioreq);
    return status;
}

/* 单声道，采样频率为44.1khz，8位 */

static iostatus_t buzzer_write(device_object_t *device, io_request_t *ioreq)
{
    iostatus_t status = IO_SUCCESS;

    ioreq->io_status.status = status;
    ioreq->io_status.infomation = ioreq->parame.write.length;

    /* 将缓冲区里面的数据写入到蜂鸣器中，以一种标准的频率，以及数据大小 */

    io_complete_request(ioreq);
    return status;
}

iostatus_t buzzer_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;

    iostatus_t status;
    
    switch (ctlcode)
    {
    case SNDIO_PLAY:
        buzzer_on();
        status = IO_SUCCESS;
        break;
    case SNDIO_STOP:
        buzzer_off();
        status = IO_SUCCESS;
        break;
    case SNDIO_SETFREQ:
        buzzer_set_freq(ioreq->parame.devctl.arg);
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


static iostatus_t buzzer_enter(driver_object_t *driver)
{
    iostatus_t status;
    device_object_t *devobj;
    /* 初始化一些其它内容 */
    status = io_create_device(driver, 0, DEV_NAME, DEVICE_TYPE_BEEP, &devobj);
    if (status != IO_SUCCESS) {
        keprint(PRINT_ERR "buzzer_enter: create device failed!\n");
        return status;
    }

    /* neighter io mode */
    devobj->flags = 0;

    return status;
}

static iostatus_t buzzer_exit(driver_object_t *driver)
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

iostatus_t buzzer_driver_func(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = buzzer_enter;
    driver->driver_exit = buzzer_exit;

    
    driver->dispatch_function[IOREQ_OPEN] = rtl8139_open;
    driver->dispatch_function[IOREQ_WRITE] = buzzer_write;
    driver->dispatch_function[IOREQ_DEVCTL] = buzzer_devctl;

    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#ifdef DEBUG_DRV
    keprint(PRINT_DEBUG "buzzer_driver_func: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}

static __init void buzzer_driver_entry(void)
{
    if (driver_object_create(buzzer_driver_func) < 0) {
        keprint(PRINT_ERR "[driver]: %s create driver failed!\n", __func__);
    }
}

driver_initcall(buzzer_driver_entry);
