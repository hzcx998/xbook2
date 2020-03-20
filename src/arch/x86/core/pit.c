#include "io.h"
#include "cmos.h"
#include "config.h"
#include "time.h"
#include <xbook/debug.h>

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
#define COUNTER0_VALUE  (TIMER_FREQ / __HZ)	    /* pit count0 数值 */

/**
 * __init_pit_clock - 初始化pit时钟
 */ 
void __init_pit_clock()
{
    printk("count value:%x low %x high %x\n", COUNTER0_VALUE, COUNTER0_VALUE & 0xff, (COUNTER0_VALUE >> 8));

	//初始化时钟
	__out8(PIT_CTRL, PIT_MODE_2 | PIT_MODE_MSB_LSB | 
            PIT_MODE_COUNTER_0 | PIT_MODE_BINARY);
	__out8(PIT_COUNTER0, (unsigned char) (COUNTER0_VALUE & 0xff));
	__out8(PIT_COUNTER0, (unsigned char) (COUNTER0_VALUE >> 8) & 0xff);   
}
