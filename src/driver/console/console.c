#include <xbook/debug.h>
#include <xbook/device.h>
#include <xbook/unit.h>
#include <arch/interrupt.h>
#include <arch/io.h>
#include <sys/ioctl.h>

#define DRV_NAME "console"
#define DRV_VERSION "0.1"

//#define DEBUG_CONSOLE

#define DISPLAY_VRAM 0x800b8000

#define	CRTC_ADDR_REG	0x3D4	/* CRT Controller Registers - Addr Register */
#define	CRTC_DATA_REG	0x3D5	/* CRT Controller Registers - Data Register */
#define	START_ADDR_H	0xC	/* reg index of video mem start addr (MSB) */
#define	START_ADDR_L	0xD	/* reg index of video mem start addr (LSB) */
#define	CURSOR_H	    0xE	/* reg index of cursor position (MSB) */
#define	CURSOR_L	    0xF	/* reg index of cursor position (LSB) */
#define	V_MEM_BASE	    DISPLAY_VRAM	/* base of color video memory */
#define	V_MEM_SIZE	    0x8000	/* 32K: B8000H -> BFFFFH */

#define COLOR_DEFAULT	(MAKE_COLOR(TEXT_BLACK, TEXT_WHITE))

#define MAX_CONSOLE_NR	    CONSOLE_MINORS

/*
颜色生成方法
MAKE_COLOR(BLUE, RED)
MAKE_COLOR(BLACK, RED) | BRIGHT
MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH
*/

#define TEXT_BLACK   0x0     /* 0000 */
#define TEXT_WHITE   0x7     /* 0111 */
#define TEXT_RED     0x4     /* 0100 */
#define TEXT_GREEN   0x2     /* 0010 */
#define TEXT_BLUE    0x1     /* 0001 */
#define TEXT_FLASH   0x80    /* 1000 0000 */
#define TEXT_BRIGHT  0x08    /* 0000 1000 */
#define	MAKE_COLOR(x,y)	((x<<4) | y) /* MAKE_COLOR(Background,Foreground) */

#define SCREEN_UP -1
#define SCREEN_DOWN 1

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define SCREEN_SIZE (80 * 25)

/* 控制台结构 */
typedef struct console_driver {
    unsigned int original_addr;          /* 控制台对应的显存的位置 */
    unsigned int screen_size;       /* 控制台占用的显存大小 */
    unsigned char color;                /* 字符的颜色 */
    int x, y;                  /* 偏移坐标位置 */
    /* 字符设备 */
    device_t *dev;
} console_driver_t;

/* 控制台表 */
console_driver_t console_device[MAX_CONSOLE_NR];

#if 1
static unsigned short get_cursor()
{
	unsigned short pos_low, pos_high;		//设置光标位置的高位的低位
	//取得光标位置
	out8(CRTC_ADDR_REG, CURSOR_H);			//光标高位
	pos_high = in8(CRTC_DATA_REG);
	out8(CRTC_ADDR_REG, CURSOR_L);			//光标低位
	pos_low = in8(CRTC_DATA_REG);
	
	return (pos_high<<8 | pos_low);	//返回合成后的值
}
#endif

static void set_cursor(unsigned short cursor)
{
	//设置光标位置 0-2000

	//执行前保存flags状态，然后关闭中断
	unsigned long flags;
    save_intr(flags);
	out8(CRTC_ADDR_REG, CURSOR_H);			//光标高位
	out8(CRTC_DATA_REG, (cursor >> 8) & 0xFF);
	out8(CRTC_ADDR_REG, CURSOR_L);			//光标低位
	out8(CRTC_DATA_REG, cursor & 0xFF);
	//恢复之前的flags状态
	restore_intr(flags);
}

static void set_video_start_addr(unsigned short addr)
{
	//执行前保存flags状态，然后关闭中断
	unsigned long flags;
    save_intr(flags);

	out8(CRTC_ADDR_REG, START_ADDR_H);
	out8(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out8(CRTC_ADDR_REG, START_ADDR_L);
	out8(CRTC_DATA_REG, addr & 0xFF);
	//恢复之前的flags状态
	restore_intr(flags);
}

/**
 * flush - 刷新光标和起始位置
 * @console: 控制台
 */
static void flush(console_driver_t *console)
{
    /* 计算光标位置，并设置 */
    set_cursor(console->original_addr + console->y * SCREEN_WIDTH + console->x);
    set_video_start_addr(console->original_addr);
}

/**
 * console_clean - 清除控制台
 * @console: 控制台
 */
static void console_clean(console_driver_t *console)
{
    /* 指向显存 */
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + console->original_addr * 2);
	int i;
	for(i = 0; i < console->screen_size * 2; i += 2){
		*vram++ = '\0';  /* 所有字符都置0 */
        *vram++ = COLOR_DEFAULT;  /* 颜色设置为黑白 */
	}
    console->x = 0;
    console->y = 0;
    flush(console);
}

#ifdef DEBUG_CONSOLE
static void dump_console(console_driver_t *console)
{
    printk(PART_TIP "----Console----\n");
    printk(PART_TIP "origin:%d size:%d x:%d y:%d color:%x dev:%x\n",
        console->original_addr, console->screen_size, console->x, console->x, console->color, console->dev);
}
#endif /* DEBUG_CONSOLE */

/**
 * console_scroll - 滚屏
 * @console: 控制台
 * @direction: 滚动方向
 *             - SCREEN_UP: 向上滚动
 *             - SCREEN_DOWN: 向下滚动
 * 
 */
static void console_scroll(console_driver_t *console, int direction)
{
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + console->original_addr * 2);
    int i;
                
	if(direction == SCREEN_UP){
        /* 起始地址 */
        for (i = SCREEN_WIDTH * 2 * 24; i > SCREEN_WIDTH * 2; i -= 2) {
            vram[i] = vram[i - SCREEN_WIDTH * 2];
            vram[i + 1] = vram[i + 1 - SCREEN_WIDTH * 2];
        }
        for (i = 0; i < SCREEN_WIDTH * 2; i += 2) {
            vram[i] = '\0';
            vram[i + 1] = COLOR_DEFAULT;
        }

	}else if(direction == SCREEN_DOWN){
        /* 起始地址 */
        for (i = 0; i < SCREEN_WIDTH * 2 * 24; i += 2) {
            vram[i] = vram[i + SCREEN_WIDTH * 2];
            vram[i + 1] = vram[i + 1 + SCREEN_WIDTH * 2];
        }
        for (i = SCREEN_WIDTH * 2 * 24; i < SCREEN_WIDTH * 2 * 25; i += 2) {
            vram[i] = '\0';
            vram[i + 1] = COLOR_DEFAULT;
        }
        console->y--;
	}
	flush(console);
}

/**
 * console_outchar - 控制台上输出一个字符
 * @console: 控制台
 * @ch: 字符
 */
static void console_outchar(console_driver_t *console, char ch)
{
	unsigned char *vram = (unsigned char *)(V_MEM_BASE + 
        (console->original_addr + console->y * SCREEN_WIDTH + console->x) *2) ;
	switch(ch){
		case '\n':
            // 如果是回车，那还是要把回车写进去
            *vram++ = '\0';
            *vram = COLOR_DEFAULT;
            console->x = 0;
            console->y++;
            
			break;
		case '\b':
            if (console->x >= 0 && console->y >= 0) {
                console->x--;
                /* 调整为上一行尾 */
                if (console->x < 0) {
                    console->x = SCREEN_WIDTH - 1;
                    console->y--;
                    /* 对y进行修复 */
                    if (console->y < 0)
                        console->y = 0;
                }
                *(vram-2) = '\0';
				*(vram-1) = COLOR_DEFAULT;
            }
			break;
		default: 
            *vram++ = ch;
			*vram = console->color;

			console->x++;
            if (console->x > SCREEN_WIDTH - 1) {
                console->x = 0;
                console->y++;
            }
			break;
	}
    /* 滚屏 */
    while (console->y > SCREEN_HEIGHT - 1) {
        console_scroll(console, SCREEN_DOWN);
    }

    flush(console);
}

/**
 * console_write - 控制台写入数据
 * @device: 设备
 * @off: 偏移（未使用）
 * @buf: 缓冲区
 * @count: 字节数量
 * 
 */
static int console_write(device_t *device, off_t off, void *buffer, size_t count)
{
    /* 获取控制台 */
    struct console_driver *local = (struct console_driver *)dev_get_local(device);
    char *buf = (char *)buffer;
	while (count > 0 && *buf) {
		/* 输出字符到控制台 */
        console_outchar(local, *buf);
		buf++;
		count--;
	}
    return 0;
}

/**
 * console_putc - 往控制台设备输出一个字符
 * @device: 设备
 * @ch: 字符
 */
static int console_putc(device_t *device, unsigned long ch)
{
    /* 获取控制台 */
    struct console_driver *local = (struct console_driver *)dev_get_local(device);
    
    console_outchar(local, ch);
    return 0;
}

/**
 * console_gotoxy - 光标移动到一个指定位置
 * @xy: 位置，x是高16位，y是低16位
 */
static void console_gotoxy(console_driver_t *console, unsigned int xy)
{
    console->x = (xy >> 16) & 0xffff;
    console->y = xy & 0xffff;

    /* fix pos */
    if (console->x < 0)
        console->x = 0;
    if (console->x >= SCREEN_WIDTH)
        console->x = SCREEN_WIDTH - 1;
    if (console->y < 0)
        console->y = 0;
    if (console->y >= SCREEN_HEIGHT)
        console->y = SCREEN_HEIGHT - 1;
    flush(console);    
}

/**
 * console_set_color - 设置控制台字符颜色
 * @color: 颜色
 */
static void console_set_color(console_driver_t *console, unsigned char color)
{
	console->color = color;
}

/**
 * console_ioctl - 控制台的IO控制
 * @device: 设备
 * @cmd: 命令
 * @arg: 参数
 * 
 * 成功返回0，失败返回-1
 */
static int console_ioctl(device_t *device, unsigned int cmd, unsigned long arg)
{
    /* 获取控制台 */
    struct console_driver *local = (struct console_driver *)dev_get_local(device);
    
	int retval = 0;
	switch (cmd)
	{
    case CONIO_SETCOLOR:
        console_set_color(local, arg);
        break;
    case CONIO_SCROLL:
        console_scroll(local, arg);
        break;
    case CONIO_CLEAN:
        console_clean(local);
        break;
    case CONIO_SETCURSOR:
        console_gotoxy(local, arg);
        break;
	default:
		/* 失败 */
		retval = -1;
		break;
	}

	return retval;
}


static device_ops_t ops = {
	.ioctl = console_ioctl, 
	.putc = console_putc,
    .write = console_write,
};

static driver_info_t drvinfo = {
    .name = DRV_NAME,
    .version = DRV_VERSION,
    .owner = "jason hu",
};

static void __console_init(console_driver_t *console, int id)
{

    /* 设置屏幕大小 */
    console->screen_size = SCREEN_SIZE;

	/* 控制台起始地址 */
    console->original_addr      = id * SCREEN_SIZE;
    
    /* 设置默认颜色 */
    console->color = COLOR_DEFAULT;

    /* 默认在左上角 */
    if (id == 0) {
        /* 继承现有位置 */
        unsigned short cursor = get_cursor();
        console->x = cursor % SCREEN_WIDTH;
        console->y = cursor / SCREEN_WIDTH;
    } else {
        /* 默认左上角位置 */
        console->x = 0;
        console->y = 0;
    }
}

/**
 * ConsoleInitScreen - 初始化控制台屏幕
 * @tty: 控制台所属的终端
 */
static int console_init_one(console_driver_t *console, int id)
{
    /* 初始化控制台信息 */
    __console_init(console, id);

    /* register device */
    console->dev = dev_alloc(MKDEV(CONSOLE_MAJOR, id));
    if (console->dev == NULL) {
        printk(KERN_ERR "alloc dev for console failed!\n");
        return -1;
    }

    console->dev->ops = &ops;
    console->dev->drvinfo = &drvinfo;

    dev_make_name(devname, "con", id);
    if (register_device(console->dev, devname, DEVTP_CHAR, console)) {
        printk(KERN_ERR "register dev for console failed!\n");
        dev_free(console->dev);
        return -1;
    }
	return 0;
}

/**
 * console_init - 初始化控制台驱动
 */
static int console_init()
{
    
   	int i;
    /* 初始化所有控制台 */
    for (i = 0; i < MAX_CONSOLE_NR; i++) {
        if (console_init_one(&console_device[i], i))
            return -1;
    }
    return 0;
}


/**
 * console_exit - 退出驱动
 */
static void console_exit()
{
   	int i;
    /* 初始化所有控制台 */
    for (i = 0; i < MAX_CONSOLE_NR; i++) {
        unregister_device(console_device[i].dev);
        dev_free(console_device[i].dev);
    }
}

EXPORT_UNIT(console_unit, "console", console_init, console_exit);
