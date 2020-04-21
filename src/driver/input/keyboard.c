#include <xbook/debug.h>
#include <xbook/bitops.h>
#include <xbook/vsprintf.h>
#include <xbook/vine.h>
#include <xbook/driver.h>
#include <xbook/fifoio.h>
#include <xbook/task.h>
#include <arch/io.h>
#include <arch/interrupt.h>

#define DRV_NAME "input-keyboard"
#define DRV_VERSION "0.1"

#define DEV_NAME "kbd"

#define DEBUG_LOCAL 0

#define DEV_FIFO_BUF_LEN     64


/* 把小键盘上的数值修复成主键盘上的值 */
#define CONFIG_PAD_FIX

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

    /* EEH, Echo响应。Keyboard使用EEh响应从60h发来的Echo请求。 */
    KBC_RET_ECHO            = 0xEE,

    /* F0H, 在Scan code set 2和Scan code set 3中，被用作Break Code的前缀。*/
    KBC_RET_BREAK           = 0xF0,
    /* FAH, ACK。当Keyboard任何时候收到一个来自于60h端口的合法命令或合法数据之后，
    都回复一个FAh。 */
    KBC_RET_ACK             = 0xFA,
    
    /* FCH, BAT失败代码。如果键盘检测失败，则会将此字节发送到8042 Output Register中。 */
    KBC_RET_BAT_BAD         = 0xFC,

    /* FEH, 当Keyboard任何时候收到一个来自于60h端口的非法命令或非法数据之后，
    或者数据的奇偶交验错误，都回复一个FEh，要求系统重新发送相关命令或数据。 */
    KBC_RET_RESEND          = 0xFE,
};

/* 单独发送给键盘的命令，有别于键盘控制器命令
这些命令是发送到数据端口0x60，而不是命令0x64，
如果有参数，就在发送一次到0x60即可。
 */
enum kbd_cmds {
    /* LED灯亮/灭，参数如下：
        位2：Caps Lock灯 1（亮）/0（灭）
        位1：Num Lock灯 1（亮）/0（灭）
        位0：Scroll Lock灯 1（亮）/0（灭）
     */
    KEY_CMD_LED_CODE        =  0xED,    /* 控制LED灯 */   
    
    /* 扫码集的参数：
        0x01： 取得当前扫描码（有返回值）
        0x02： 代表第一套扫描码
        0x03： 代表第二套扫描码
        0x04： 代表第三套扫描码
     */
    KEY_CMD_SET_SCAN_CODE   =  0xF0,    /* 设置键盘使用的扫码集*/        
    KEY_CMD_GET_DEVICE_ID   =  0xF2,    /* 获取键盘设备的ID号（2B） */                
    KEY_CMD_START_SCAN      =  0xF4,    /* 开启键盘扫描 */
    KEY_CMD_STOP_SCAN       =  0xF5,    /* 停止键盘扫描 */
    KEY_CMD_RESTART         =  0xFF,    /* 重启键盘 */
};

/* 键盘控制器配置 */
#define KBC_CONFIG	(KBC_CFG_ENABLE_KEY_INTR | KBC_CFG_ENABLE_MOUSE_INTR | \
                    KBC_CFG_INIT_DONE | KBC_CFG_SCAN_CODE_TRANS)

/* 等待键盘控制器可写入，当输入缓冲区为空后才可以写入 */
#define WAIT_KBC_WRITE()    while (in8(KBC_STATUS) & KBC_STATUS_INPUT_BUF_FULL)
/* 等待键盘控制器可读取，当输出缓冲区为空后才可以读取 */
#define WAIT_KBC_READ()    while (in8(KBC_STATUS) & KBC_STATUS_OUT_BUF_FULL)

#define KEYMAP_COLS	3	/* Number of columns in keymap */
#define MAX_SCAN_CODE_NR	0x80	/* Number of scan codes (rows in keymap) */


/* raw key value = code passed to tty & MASK_RAW
the value can be found either in the keymap column 0
or in the list below */
#define KBD_MASK_RAW	0x01FF		

/* 键盘值屏蔽，通过key和mask与，就可以得出数值 */
#define KBD_KEY_MASK	0x01FF		

#define KBD_FLAG_EXT	0x0100		/* Normal function keys		*/

/* 按键的一些标志 */
#define KBD_FLAG_BREAK_MASK	0x0080		/* Break Code			*/

/* Special keys */
#define KBD_ESC		(0x01 + KBD_FLAG_EXT)	/* Esc		*/
#define KBD_TAB		(0x02 + KBD_FLAG_EXT)	/* Tab		*/
#define KBD_ENTER		(0x03 + KBD_FLAG_EXT)	/* Enter	*/
#define KBD_BACKSPACE	(0x04 + KBD_FLAG_EXT)	/* BackSpace	*/

#define KBD_GUI_L		(0x05 + KBD_FLAG_EXT)	/* L GUI	*/
#define KBD_GUI_R		(0x06 + KBD_FLAG_EXT)	/* R GUI	*/
#define KBD_APPS		(0x07 + KBD_FLAG_EXT)	/* APPS	*/

/* Shift, Ctrl, Alt */
#define KBD_SHIFT_L		(0x08 + KBD_FLAG_EXT)	/* L Shift	*/
#define KBD_SHIFT_R		(0x09 + KBD_FLAG_EXT)	/* R Shift	*/
#define KBD_CTRL_L		(0x0A + KBD_FLAG_EXT)	/* L Ctrl	*/
#define KBD_CTRL_R		(0x0B + KBD_FLAG_EXT)	/* R Ctrl	*/
#define KBD_ALT_L		(0x0C + KBD_FLAG_EXT)	/* L Alt	*/
#define KBD_ALT_R		(0x0D + KBD_FLAG_EXT)	/* R Alt	*/

/* Lock keys */
#define KBD_CAPS_LOCK	(0x0E + KBD_FLAG_EXT)	/* Caps Lock	*/
#define	KBD_NUM_LOCK	(0x0F + KBD_FLAG_EXT)	/* Number Lock	*/
#define KBD_SCROLL_LOCK	(0x10 + KBD_FLAG_EXT)	/* Scroll Lock	*/

/* Function keys */
#define KBD_F1		(0x11 + KBD_FLAG_EXT)	/* F1		*/
#define KBD_F2		(0x12 + KBD_FLAG_EXT)	/* F2		*/
#define KBD_F3		(0x13 + KBD_FLAG_EXT)	/* F3		*/
#define KBD_F4		(0x14 + KBD_FLAG_EXT)	/* F4		*/
#define KBD_F5		(0x15 + KBD_FLAG_EXT)	/* F5		*/
#define KBD_F6		(0x16 + KBD_FLAG_EXT)	/* F6		*/
#define KBD_F7		(0x17 + KBD_FLAG_EXT)	/* F7		*/
#define KBD_F8		(0x18 + KBD_FLAG_EXT)	/* F8		*/
#define KBD_F9		(0x19 + KBD_FLAG_EXT)	/* F9		*/
#define KBD_F10		(0x1A + KBD_FLAG_EXT)	/* F10		*/
#define KBD_F11		(0x1B + KBD_FLAG_EXT)	/* F11		*/
#define KBD_F12		(0x1C + KBD_FLAG_EXT)	/* F12		*/

/* Control Pad */
#define KBD_PRINTSCREEN	(0x1D + KBD_FLAG_EXT)	/* Print Screen	*/
#define KBD_PAUSEBREAK	(0x1E + KBD_FLAG_EXT)	/* Pause/Break	*/
#define KBD_INSERT		(0x1F + KBD_FLAG_EXT)	/* Insert	*/
#define KBD_DELETE		(0x20 + KBD_FLAG_EXT)	/* Delete	*/
#define KBD_HOME		(0x21 + KBD_FLAG_EXT)	/* Home		*/
#define KBD_END		(0x22 + KBD_FLAG_EXT)	/* End		*/
#define KBD_PAGEUP		(0x23 + KBD_FLAG_EXT)	/* Page Up	*/
#define KBD_PAGEDOWN	(0x24 + KBD_FLAG_EXT)	/* Page Down	*/
#define KBD_UP		(0x25 + KBD_FLAG_EXT)	/* Up		*/
#define KBD_DOWN		(0x26 + KBD_FLAG_EXT)	/* Down		*/
#define KBD_LEFT		(0x27 + KBD_FLAG_EXT)	/* Left		*/
#define KBD_RIGHT		(0x28 + KBD_FLAG_EXT)	/* Right	*/

/* ACPI keys */
#define KBD_POWER		(0x29 + KBD_FLAG_EXT)	/* Power	*/
#define KBD_SLEEP		(0x2A + KBD_FLAG_EXT)	/* Sleep	*/
#define KBD_WAKE		(0x2B + KBD_FLAG_EXT)	/* Wake Up	*/

/* Num Pad */
#define KBD_PAD_SLASH	(0x2C + KBD_FLAG_EXT)	/* /		*/
#define KBD_PAD_STAR	(0x2D + KBD_FLAG_EXT)	/* *		*/
#define KBD_PAD_MINUS	(0x2E + KBD_FLAG_EXT)	/* -		*/
#define KBD_PAD_PLUS	(0x2F + KBD_FLAG_EXT)	/* +		*/
#define KBD_PAD_ENTER	(0x30 + KBD_FLAG_EXT)	/* Enter	*/
#define KBD_PAD_DOT		(0x31 + KBD_FLAG_EXT)	/* .		*/
#define KBD_PAD_0		(0x32 + KBD_FLAG_EXT)	/* 0		*/
#define KBD_PAD_1		(0x33 + KBD_FLAG_EXT)	/* 1		*/
#define KBD_PAD_2		(0x34 + KBD_FLAG_EXT)	/* 2		*/
#define KBD_PAD_3		(0x35 + KBD_FLAG_EXT)	/* 3		*/
#define KBD_PAD_4		(0x36 + KBD_FLAG_EXT)	/* 4		*/
#define KBD_PAD_5		(0x37 + KBD_FLAG_EXT)	/* 5		*/
#define KBD_PAD_6		(0x38 + KBD_FLAG_EXT)	/* 6		*/
#define KBD_PAD_7		(0x39 + KBD_FLAG_EXT)	/* 7		*/
#define KBD_PAD_8		(0x3A + KBD_FLAG_EXT)	/* 8		*/
#define KBD_PAD_9		(0x3B + KBD_FLAG_EXT)	/* 9		*/
#define KBD_PAD_UP		KBD_PAD_8			/* Up		*/
#define KBD_PAD_DOWN	KBD_PAD_2			/* Down		*/
#define KBD_PAD_LEFT	KBD_PAD_4			/* Left		*/
#define KBD_PAD_RIGHT	KBD_PAD_6			/* Right	*/
#define KBD_PAD_HOME	KBD_PAD_7			/* Home		*/
#define KBD_PAD_END		KBD_PAD_1			/* End		*/
#define KBD_PAD_PAGEUP	KBD_PAD_9			/* Page Up	*/
#define KBD_PAD_PAGEDOWN	KBD_PAD_3			/* Page Down	*/
#define KBD_PAD_INS		KBD_PAD_0			/* Ins		*/
#define KBD_PAD_MID		KBD_PAD_5			/* Middle key	*/
#define KBD_PAD_DEL		KBD_PAD_DOT			/* Del		*/

/* 按键码 */
#define KEYCODE_NONE		0			/* 没有按键 */


/* 控制标志 */
enum InputKeycodeFlags {
    KBD_FLAG_KEY_MASK  = 0x1FF,        /* 键值的mask值 */
    KBD_FLAG_SHIFT_L   = 0x0200,		/* Shift key			*/
    KBD_FLAG_SHIFT_R   = 0x0400,		/* Shift key			*/
    KBD_FLAG_CTRL_L    = 0x0800,		/* Control key			*/
    KBD_FLAG_CTRL_R    = 0x1000,		/* Control key			*/
    KBD_FLAG_ALT_L     = 0x2000,		/* Alternate key		*/
    KBD_FLAG_ALT_R     = 0x4000,		/* Alternate key		*/
    KBD_FLAG_PAD	    = 0x8000,		/* keys in num pad		*/
    KBD_FLAG_NUM	    = 0x10000,	    /* 数字锁		*/
    KBD_FLAG_CAPS	    = 0x20000,	    /* 数字锁		*/
    KBD_FLAG_BREAK	    = 0x40000,		/* Break Code   */
};

/* Keymap for US MF-2 ext-> */
static unsigned int keymap[MAX_SCAN_CODE_NR * KEYMAP_COLS] = {

/* scan-code			!Shift		Shift		E0 XX	*/
/* ==================================================================== */
/* 0x00 - none		*/	0,		0,		0,
/* 0x01 - ESC		*/	KBD_ESC,    KBD_ESC,		0,
/* 0x02 - '1'		*/	'1',		'!',		0,
/* 0x03 - '2'		*/	'2',		'@',		0,
/* 0x04 - '3'		*/	'3',		'#',		0,
/* 0x05 - '4'		*/	'4',		'$',		0,
/* 0x06 - '5'		*/	'5',		'%',		0,
/* 0x07 - '6'		*/	'6',		'^',		0,
/* 0x08 - '7'		*/	'7',		'&',		0,
/* 0x09 - '8'		*/	'8',		'*',		0,
/* 0x0A - '9'		*/	'9',		'(',		0,
/* 0x0B - '0'		*/	'0',		')',		0,
/* 0x0C - '-'		*/	'-',		'_',		0,
/* 0x0D - '='		*/	'=',		'+',		0,
/* 0x0E - BS		*/	KBD_BACKSPACE,	KBD_BACKSPACE,	0,
/* 0x0F - TAB		*/	KBD_TAB,		KBD_TAB,		0,
/* 0x10 - 'q'		*/	'q',		'Q',		0,
/* 0x11 - 'w'		*/	'w',		'W',		0,
/* 0x12 - 'e'		*/	'e',		'E',		0,
/* 0x13 - 'r'		*/	'r',		'R',		0,
/* 0x14 - 't'		*/	't',		'T',		0,
/* 0x15 - 'y'		*/	'y',		'Y',		0,
/* 0x16 - 'u'		*/	'u',		'U',		0,
/* 0x17 - 'i'		*/	'i',		'I',		0,
/* 0x18 - 'o'		*/	'o',		'O',		0,
/* 0x19 - 'p'		*/	'p',		'P',		0,
/* 0x1A - '['		*/	'[',		'{',		0,
/* 0x1B - ']'		*/	']',		'}',		0,
/* 0x1C - CR/LF		*/	KBD_ENTER,		KBD_ENTER,		KBD_PAD_ENTER,
/* 0x1D - l. Ctrl	*/	KBD_CTRL_L,		KBD_CTRL_L,		KBD_CTRL_R,
/* 0x1E - 'a'		*/	'a',		'A',		0,
/* 0x1F - 's'		*/	's',		'S',		0,
/* 0x20 - 'd'		*/	'd',		'D',		0,
/* 0x21 - 'f'		*/	'f',		'F',		0,
/* 0x22 - 'g'		*/	'g',		'G',		0,
/* 0x23 - 'h'		*/	'h',		'H',		0,
/* 0x24 - 'j'		*/	'j',		'J',		0,
/* 0x25 - 'k'		*/	'k',		'K',		0,
/* 0x26 - 'l'		*/	'l',		'L',		0,
/* 0x27 - ';'		*/	';',		':',		0,
/* 0x28 - '\''		*/	'\'',		'"',		0,
/* 0x29 - '`'		*/	'`',		'~',		0,
/* 0x2A - l. SHIFT	*/	KBD_SHIFT_L,	KBD_SHIFT_L,	0,
/* 0x2B - '\'		*/	'\\',		'|',		0,
/* 0x2C - 'z'		*/	'z',		'Z',		0,
/* 0x2D - 'x'		*/	'x',		'X',		0,
/* 0x2E - 'c'		*/	'c',		'C',		0,
/* 0x2F - 'v'		*/	'v',		'V',		0,
/* 0x30 - 'b'		*/	'b',		'B',		0,
/* 0x31 - 'n'		*/	'n',		'N',		0,
/* 0x32 - 'm'		*/	'm',		'M',		0,
/* 0x33 - ','		*/	',',		'<',		0,
/* 0x34 - '.'		*/	'.',		'>',		0,
/* 0x35 - '/'		*/	'/',		'?',		KBD_PAD_SLASH,
/* 0x36 - r. SHIFT	*/	KBD_SHIFT_R,	KBD_SHIFT_R,	0,
/* 0x37 - '*'		*/	'*',		'*',    	0,
/* 0x38 - ALT		*/	KBD_ALT_L,		KBD_ALT_L,  	KBD_ALT_R,
/* 0x39 - ' '		*/	' ',		' ',		0,
/* 0x3A - caps_lock	*/	KBD_CAPS_LOCK,	KBD_CAPS_LOCK,	0,
/* 0x3B - F1		*/	KBD_F1,		KBD_F1,		0,
/* 0x3C - F2		*/	KBD_F2,		KBD_F2,		0,
/* 0x3D - F3		*/	KBD_F3,		KBD_F3,		0,
/* 0x3E - F4		*/	KBD_F4,		KBD_F4,		0,
/* 0x3F - F5		*/	KBD_F5,		KBD_F5,		0,
/* 0x40 - F6		*/	KBD_F6,		KBD_F6,		0,
/* 0x41 - F7		*/	KBD_F7,		KBD_F7,		0,
/* 0x42 - F8		*/	KBD_F8,		KBD_F8,		0,
/* 0x43 - F9		*/	KBD_F9,		KBD_F9,		0,
/* 0x44 - F10		*/	KBD_F10,    KBD_F10,		0,
/* 0x45 - num_lock	*/	KBD_NUM_LOCK,	KBD_NUM_LOCK,	0,
/* 0x46 - ScrLock	*/	KBD_SCROLL_LOCK,	KBD_SCROLL_LOCK,	0,
/* 0x47 - Home		*/	KBD_PAD_HOME,	'7',		KBD_HOME,
/* 0x48 - CurUp		*/	KBD_PAD_UP,		'8',		KBD_UP,
/* 0x49 - PgUp		*/	KBD_PAD_PAGEUP,	'9',		KBD_PAGEUP,
/* 0x4A - '-'		*/	KBD_PAD_MINUS,	'-',		0,
/* 0x4B - Left		*/	KBD_PAD_LEFT,	'4',		KBD_LEFT,
/* 0x4C - MID		*/	KBD_PAD_MID,	'5',		0,
/* 0x4D - Right		*/	KBD_PAD_RIGHT,	'6',		KBD_RIGHT,
/* 0x4E - '+'		*/	KBD_PAD_PLUS,	'+',		0,
/* 0x4F - End		*/	KBD_PAD_END,	'1',		KBD_END,
/* 0x50 - Down		*/	KBD_PAD_DOWN,	'2',		KBD_DOWN,
/* 0x51 - PgDown	*/	KBD_PAD_PAGEDOWN,	'3',		KBD_PAGEDOWN,
/* 0x52 - Insert	*/	KBD_PAD_INS,	'0',		KBD_INSERT,
/* 0x53 - Delete	*/	KBD_PAD_DOT,	'.',		KBD_DELETE,
/* 0x54 - Enter		*/	0,		0,		0,
/* 0x55 - ???		*/	0,		0,		0,
/* 0x56 - ???		*/	0,		0,		0,
/* 0x57 - F11		*/	KBD_F11,		KBD_F11,		0,	
/* 0x58 - F12		*/	KBD_F12,		KBD_F12,		0,	
/* 0x59 - ???		*/	0,		0,		0,	
/* 0x5A - ???		*/	0,		0,		0,	
/* 0x5B - ???		*/	0,		0,		KBD_GUI_L,	
/* 0x5C - ???		*/	0,		0,		KBD_GUI_R,	
/* 0x5D - ???		*/	0,		0,		KBD_APPS,	
/* 0x5E - ???		*/	0,		0,		0,	
/* 0x5F - ???		*/	0,		0,		0,
/* 0x60 - ???		*/	0,		0,		0,
/* 0x61 - ???		*/	0,		0,		0,	
/* 0x62 - ???		*/	0,		0,		0,	
/* 0x63 - ???		*/	0,		0,		0,	
/* 0x64 - ???		*/	0,		0,		0,	
/* 0x65 - ???		*/	0,		0,		0,	
/* 0x66 - ???		*/	0,		0,		0,	
/* 0x67 - ???		*/	0,		0,		0,	
/* 0x68 - ???		*/	0,		0,		0,	
/* 0x69 - ???		*/	0,		0,		0,	
/* 0x6A - ???		*/	0,		0,		0,	
/* 0x6B - ???		*/	0,		0,		0,	
/* 0x6C - ???		*/	0,		0,		0,	
/* 0x6D - ???		*/	0,		0,		0,	
/* 0x6E - ???		*/	0,		0,		0,	
/* 0x6F - ???		*/	0,		0,		0,	
/* 0x70 - ???		*/	0,		0,		0,	
/* 0x71 - ???		*/	0,		0,		0,	
/* 0x72 - ???		*/	0,		0,		0,	
/* 0x73 - ???		*/	0,		0,		0,	
/* 0x74 - ???		*/	0,		0,		0,	
/* 0x75 - ???		*/	0,		0,		0,	
/* 0x76 - ???		*/	0,		0,		0,	
/* 0x77 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x7A - ???		*/	0,		0,		0,	
/* 0x7B - ???		*/	0,		0,		0,	
/* 0x7C - ???		*/	0,		0,		0,	
/* 0x7D - ???		*/	0,		0,		0,	
/* 0x7E - ???		*/	0,		0,		0,
/* 0x7F - ???		*/	0,		0,		0
};


/*
	回车键:	把光标移到第一列
	换行键:	把光标前进到下一行
*/


/*====================================================================================*
				Appendix: Scan code set 1
 *====================================================================================*

KEY	MAKE	BREAK	-----	KEY	MAKE	BREAK	-----	KEY	MAKE	BREAK
--------------------------------------------------------------------------------------
A	1E	9E		9	0A	8A		[	1A	9A
B	30	B0		`	29	89		INSERT	E0,52	E0,D2
C	2E	AE		-	0C	8C		HOME	E0,47	E0,C7
D	20	A0		=	0D	8D		PG UP	E0,49	E0,C9
E	12	92		\	2B	AB		DELETE	E0,53	E0,D3
F	21	A1		BKSP	0E	8E		END	E0,4F	E0,CF
G	22	A2		SPACE	39	B9		PG DN	E0,51	E0,D1
H	23	A3		TAB	0F	8F		U ARROW	E0,48	E0,C8
I	17	97		CAPS	3A	BA		L ARROW	E0,4B	E0,CB
J	24	A4		L SHFT	2A	AA		D ARROW	E0,50	E0,D0
K	25	A5		L CTRL	1D	9D		R ARROW	E0,4D	E0,CD
L	26	A6		L GUI	E0,5B	E0,DB		NUM	45	C5
M	32	B2		L ALT	38	B8		KP /	E0,35	E0,B5
N	31	B1		R SHFT	36	B6		KP *	37	B7
O	18	98		R CTRL	E0,1D	E0,9D		KP -	4A	CA
P	19	99		R GUI	E0,5C	E0,DC		KP +	4E	CE
Q	10	19		R ALT	E0,38	E0,B8		KP EN	E0,1C	E0,9C
R	13	93		APPS	E0,5D	E0,DD		KP .	53	D3
S	1F	9F		ENTER	1C	9C		KP 0	52	D2
T	14	94		ESC	01	81		KP 1	4F	CF
U	16	96		F1	3B	BB		KP 2	50	D0
V	2F	AF		F2	3C	BC		KP 3	51	D1
W	11	91		F3	3D	BD		KP 4	4B	CB
X	2D	AD		F4	3E	BE		KP 5	4C	CC
Y	15	95		F5	3F	BF		KP 6	4D	CD
Z	2C	AC		F6	40	C0		KP 7	47	C7
0	0B	8B		F7	41	C1		KP 8	48	C8
1	02	82		F8	42	C2		KP 9	49	C9
2	03	83		F9	43	C3		]	1B	9B
3	04	84		F10	44	C4		;	27	A7
4	05	85		F11	57	D7		'	28	A8
5	06	86		F12	58	D8		,	33	B3

6	07	87		PRTSCRN	E0,2A	E0,B7		.	34	B4
					E0,37	E0,AA

7	08	88		SCROLL	46	C6		/	35	B5

8	09	89		PAUSE E1,1D,45	-NONE-				
				      E1,9D,C5


-----------------
ACPI Scan Codes:
-------------------------------------------
Key		Make Code	Break Code
-------------------------------------------
Power		E0, 5E		E0, DE
Sleep		E0, 5F		E0, DF
Wake		E0, 63		E0, E3


-------------------------------
Windows Multimedia Scan Codes:
-------------------------------------------
Key		Make Code	Break Code
-------------------------------------------
Next Track	E0, 19		E0, 99
Previous Track	E0, 10		E0, 90
Stop		E0, 24		E0, A4
Play/Pause	E0, 22		E0, A2
Mute		E0, 20		E0, A0
Volume Up	E0, 30		E0, B0
Volume Down	E0, 2E		E0, AE
Media Select	E0, 6D		E0, ED
E-Mail		E0, 6C		E0, EC
Calculator	E0, 21		E0, A1
My Computer	E0, 6B		E0, EB
WWW Search	E0, 65		E0, E5
WWW Home	E0, 32		E0, B2
WWW Back	E0, 6A		E0, EA
WWW Forward	E0, 69		E0, E9
WWW Stop	E0, 68		E0, E8
WWW Refresh	E0, 67		E0, E7
WWW Favorites	E0, 66		E0, E6

*=====================================================================================*/

typedef struct _device_extension {
    device_object_t *device_object; /* 设备对象 */
    char irq;           /* irq号 */
    
    int	code_with_e0;	/* 携带E0的值 */
	int	shift_left;	/* l shift state */
	int	shift_right;	/* r shift state */
	int	alt_left;	/* l alt state	 */
	int	alt_right;	/* r left state	 */
	int	ctl_left;	/* l ctrl state	 */
	int	ctl_right;	/* l ctrl state	 */
	int	caps_lock;	/* Caps Lock	 */
	int	num_lock;	/* Num Lock	 */
	int	scroll_lock;	/* Scroll Lock	 */
	int	column;		/* 数据位于哪一列 */

    fifo_io_t fifoio;
    unsigned int keycode;       /* 解析出来的键值 */
} device_extension_t;


/* 等待键盘控制器应答，如果不是回复码就一直等待
这个本应该是宏的，但是在vmware虚拟机中会卡在那儿，所以改成宏类函数
 */
static void WAIT_KBC_ACK()
{
	unsigned char read;
	do {
		read = in8(KBC_READ_DATA);
	} while ((read =! KBC_RET_ACK));
}

/**
 * set_leds - 设置键盘led灯状态
 */
static void set_leds(device_extension_t *ext)
{
	/* 先合成成为一个数据，后面写入寄存器 */
	unsigned char leds = (ext->caps_lock << 2) | 
        (ext->num_lock << 1) | ext->scroll_lock;
	
	/* 数据指向led */
	WAIT_KBC_WRITE();
	out8(KBC_WRITE_DATA, KEY_CMD_LED_CODE);
	WAIT_KBC_ACK();
	/* 写入新的led值 */
	WAIT_KBC_WRITE();
	out8(KBC_WRITE_DATA, leds);
    WAIT_KBC_ACK();
}

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

/**
 * AnalysisKeyboard - 按键分析 
 */
unsigned int keyboard_do_read(device_extension_t *ext)
{
	unsigned char scan_code;
	int make;
	
	unsigned int key = 0;
	unsigned int *keyrow;

	ext->code_with_e0 = 0;

	scan_code = get_bytes_from_buf(ext);
	
	/* 检查是否是0xe1打头的数据 */
	if(scan_code == 0xe1){
		int i;
		unsigned char pausebrk_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
		int is_pausebreak = 1;
		for(i = 1; i < 6; i++){
			if (get_bytes_from_buf(ext) != pausebrk_scode[i]) {
				is_pausebreak = 0;
				break;
			}
		}
		if (is_pausebreak) {
			key = KBD_PAUSEBREAK;
		}
	} else if(scan_code == 0xe0){
		/* 检查是否是0xe0打头的数据 */
		scan_code = get_bytes_from_buf(ext);

		//PrintScreen 被按下
		if (scan_code == 0x2A) {
			if (get_bytes_from_buf(ext) == 0xE0) {
				if (get_bytes_from_buf(ext) == 0x37) {
					key = KBD_PRINTSCREEN;
					make = 1;
				}
			}
		}
		//PrintScreen 被释放
		if (scan_code == 0xB7) {
			if (get_bytes_from_buf(ext) == 0xE0) {
				if (get_bytes_from_buf(ext) == 0xAA) {
					key = KBD_PRINTSCREEN;
					make = 0;
				}
			}
		}
		//不是PrintScreen, 此时scan_code为0xE0紧跟的那个值. 
		if (key == 0) {
			ext->code_with_e0 = 1;
		}
	}if ((key != KBD_PAUSEBREAK) && (key != KBD_PRINTSCREEN)) {
		/* 处理一般字符 */
		make = (scan_code & KBD_FLAG_BREAK_MASK ? 0 : 1);

		//先定位到 keymap 中的行 
		keyrow = &keymap[(scan_code & 0x7F) * KEYMAP_COLS];
		
		ext->column = 0;
		int caps = ext->shift_left || ext->shift_right;
		if (ext->caps_lock) {
			if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')){
				caps = !caps;
			}
		}
        /* 如果大写打开 */
		if (caps) {
			ext->column = 1;
		}

        /* 如果有0xE0数据 */
		if (ext->code_with_e0) {
			ext->column = 2;
		}
		/* 读取列中的数据 */
		key = keyrow[ext->column];
		
        /* shift，ctl，alt变量设置，
        caps，num，scroll锁设置 */
		switch(key) {
		case KBD_SHIFT_L:
			ext->shift_left = make;
			break;
		case KBD_SHIFT_R:
			ext->shift_right = make;
			break;
		case KBD_CTRL_L:
			ext->ctl_left = make;
			break;
		case KBD_CTRL_R:
			ext->ctl_right = make;
			break;
		case KBD_ALT_L:
			ext->alt_left = make;
			break;
		case KBD_ALT_R:
			ext->alt_left = make;
			break;
		case KBD_CAPS_LOCK:
			if (make) {
				ext->caps_lock   = !ext->caps_lock;
				set_leds(ext);
			}
			break;
		case KBD_NUM_LOCK:
			if (make) {
				ext->num_lock    = !ext->num_lock;
				set_leds(ext);
			}
			break;
		case KBD_SCROLL_LOCK:
			if (make) {
				ext->scroll_lock = !ext->scroll_lock;
				set_leds(ext);
			}
			break;	
		default:
			break;
		}
        int pad = 0;
        //首先处理小键盘
        if ((key >= KBD_PAD_SLASH) && (key <= KBD_PAD_9)) {
            pad = 1;
#ifdef CONFIG_PAD_FIX
            switch(key) {
            case KBD_PAD_SLASH:
                key = '/';
                break;
            case KBD_PAD_STAR:
                key = '*';
                break;
            case KBD_PAD_MINUS:
                key = '-';
                break;
            case KBD_PAD_PLUS:
                key = '+';
                break;
            case KBD_PAD_ENTER:
                key = KBD_ENTER;
                break;
            default:
                if (ext->num_lock &&
                    (key >= KBD_PAD_0) &&
                    (key <= KBD_PAD_9)) 
                {
                    key = key - KBD_PAD_0 + '0';
                }else if (ext->num_lock &&
                    (key == KBD_PAD_DOT)) 
                { 
                    key = '.';
                }else{
                    switch(key) {
                    case KBD_PAD_HOME:
                        key = KBD_HOME;
                        
                        break;
                    case KBD_PAD_END:
                        key = KBD_END;
                        
                        break;
                    case KBD_PAD_PAGEUP:
                        key = KBD_PAGEUP;
                        
                        break;
                    case KBD_PAD_PAGEDOWN:
                        key = KBD_PAGEDOWN;
                        
                        break;
                    case KBD_PAD_INS:
                        key = KBD_INSERT;
                        break;
                    case KBD_PAD_UP:
                        key = KBD_UP;
                        break;
                    case KBD_PAD_DOWN:
                        key = KBD_DOWN;
                        break;
                    case KBD_PAD_LEFT:
                        key = KBD_LEFT;
                        break;
                    case KBD_PAD_RIGHT:
                        key = KBD_RIGHT;
                        break;
                    case KBD_PAD_DOT:
                        key = KBD_DELETE;
                        break;
                    default:
                        break;
                    }
                }
                break;
            }
#endif /* CONFIG_PAD */
        }
        /* 如果有组合件，就需要合成成为组合后的按钮，可以是ctl+alt+shift+按键的格式 */
        key |= ext->shift_left	? KBD_FLAG_SHIFT_L	: 0;
        key |= ext->shift_right	? KBD_FLAG_SHIFT_R	: 0;
        key |= ext->ctl_left	? KBD_FLAG_CTRL_L	: 0;
        key |= ext->ctl_right	? KBD_FLAG_CTRL_R	: 0;
        key |= ext->alt_left	? KBD_FLAG_ALT_L	: 0;
        key |= ext->alt_right	? KBD_FLAG_ALT_R	: 0;
        key |= pad      ? KBD_FLAG_PAD      : 0;

        /* 如果是BREAK,就需要添加BREAK标志 */
        key |= make ? 0: KBD_FLAG_BREAK;
        
        /* 设置锁标志 */
        key |= ext->num_lock ? KBD_FLAG_NUM : 0;
        key |= ext->caps_lock ? KBD_FLAG_CAPS : 0;

        /* 把按键输出 */
        return key;
	}
    return KEYCODE_NONE;
}

/**
 * keyboard_handler - 时钟中断处理函数
 * @irq: 中断号
 * @data: 中断的数据
 */
static int keyboard_handler(unsigned long irq, unsigned long data)
{
    device_extension_t *ext = (device_extension_t *) data;
	/* 先从硬件获取按键数据 */
	unsigned char scan_code = in8(KBC_READ_DATA);

    /* 把数据放到io队列 */
    fifo_io_put(&ext->fifoio, scan_code);
}

iostatus_t keyboard_read(device_object_t *device, io_request_t *ioreq)
{
    device_extension_t *ext = device->device_extension;
    
    iostatus_t status = IO_SUCCESS;
    /* 直接返回读取的数据 */
    ioreq->io_status.infomation = ext->keycode;

    if (!ext->keycode)
        status = IO_FAILED;

    ext->keycode = 0; /* 读取后置0 */

    ioreq->io_status.status = status;
    /* 调用完成请求 */
    io_complete_request(ioreq);

    return status;
}

iostatus_t keyboard_devctl(device_object_t *device, io_request_t *ioreq)
{
    unsigned int ctlcode = ioreq->parame.devctl.code;

    iostatus_t status;

    switch (ctlcode)
    {
    case DEVCTL_CODE_TEST:
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "keyboard_devctl: code=%x arg=%x\n", ctlcode, ioreq->parame.devctl.arg);
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

/* 用内核线程来处理到达的数据 */
void kbd_thread(void *arg) {
    device_extension_t *ext = (device_extension_t *) arg;
    unsigned int key;
    while (1) {
        key = keyboard_do_read(ext);
        if (key > 0) {
            ext->keycode = key;
#if DEBUG_LOCAL == 1
        printk(KERN_DEBUG "kbd_thread: key:%c\n", key);
#endif
        }
    }
}

static iostatus_t keyboard_enter(driver_object_t *driver)
{
    iostatus_t status;
    
    device_object_t *devobj;
    device_extension_t *devext;
    char irq;

    /* 初始化一些其它内容 */
    status = io_create_device(driver, sizeof(device_extension_t), DEV_NAME, DEVICE_TYPE_KEYBOARD, &devobj);

    if (status != IO_SUCCESS) {
        printk(KERN_ERR "keyboard_enter: create device failed!\n");
        return status;
    }
    /* buffered io mode */
    devobj->flags = 0;
    devext = (device_extension_t *)devobj->device_extension;
    devext->device_object = devobj;
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "keyboard_enter: device extension: device name=%s object=%x\n",
        devext->device_name.text, devext->device_object);
#endif        
    devext->irq = IRQ1;
    devext->keycode = 0;
	/* 初始化私有数据 */
	devext->code_with_e0 = 0;
	
	devext->shift_left	= devext->shift_right = 0;
	devext->alt_left	= devext->alt_right   = 0;
	devext->ctl_left	= devext->ctl_right  = 0;
	
	devext->caps_lock   = 0;
	devext->num_lock    = 1;
	devext->scroll_lock = 0;
    unsigned char *buf = kmalloc(DEV_FIFO_BUF_LEN);
    if (buf == NULL) {
        status = IO_FAILED;
        printk(KERN_DEBUG "keyboard_enter: alloc buf failed!\n");
        return status;
    }
    fifo_io_init(&devext->fifoio, buf, DEV_FIFO_BUF_LEN);

    /* 注册时钟中断并打开中断，因为设定硬件过程中可能产生中断，所以要提前打开 */	
	register_irq(devext->irq, keyboard_handler, IRQF_DISABLED, "IRQ1", DRV_NAME, (uint32_t)devext);
    
    /* 初始化键盘控制器 */

    /* 发送写配置命令 */
    WAIT_KBC_WRITE();
	out8(KBC_CMD, KBC_CMD_WRITE_CONFIG);
    WAIT_KBC_ACK();
    
    /* 往数据端口写入配置值 */
    WAIT_KBC_WRITE();
	out8(KBC_WRITE_DATA, KBC_CONFIG);
	WAIT_KBC_ACK();

    /* 启动一个内核线程来处理数据 */
    kthread_start("kbd", TASK_PRIO_RT, kbd_thread, devext);

    return status;
}

static iostatus_t keyboard_exit(driver_object_t *driver)
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

iostatus_t keyboard_driver_vine(driver_object_t *driver)
{
    iostatus_t status = IO_SUCCESS;
    
    /* 绑定驱动信息 */
    driver->driver_enter = keyboard_enter;
    driver->driver_exit = keyboard_exit;

    driver->dispatch_function[IOREQ_READ] = keyboard_read;
    driver->dispatch_function[IOREQ_DEVCTL] = keyboard_devctl;
    
    /* 初始化驱动名字 */
    string_new(&driver->name, DRV_NAME, DRIVER_NAME_LEN);
#if DEBUG_LOCAL == 1
    printk(KERN_DEBUG "keyboard_driver_vine: driver name=%s\n",
        driver->name.text);
#endif
    
    return status;
}
