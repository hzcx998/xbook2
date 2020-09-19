#include <arch/config.h>

#ifdef X86_CONSOLE_HW

#include <xbook/debug.h>
#include <arch/io.h>
#include <arch/debug.h>
#include <arch/interrupt.h>

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

#define DISPLAY_VRAM 0x800b8000

#define	CRTC_ADDR_REG	0x3D4	/* CRT Controller Registers - Addr Register */
#define	CRTC_DATA_REG	0x3D5	/* CRT Controller Registers - Data Register */
#define	START_ADDR_H	0xC	/* reg index of video mem start addr (MSB) */
#define	START_ADDR_L	0xD	/* reg index of video mem start addr (LSB) */
#define	CURSOR_H	    0xE	/* reg index of cursor position (MSB) */
#define	CURSOR_L	    0xF	/* reg index of cursor position (LSB) */
#define	V_MEM_BASE	    DISPLAY_VRAM	/* base of color video memory */
#define	V_MEM_SIZE	    0x8000	/* 32K: B8000H -> BFFFFH */

#define SCREEN_UP -1
#define SCREEN_DOWN 1

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define SCREEN_SIZE (80 * 25)

#define COLOR_DEFAULT	(MAKE_COLOR(TEXT_BLACK, TEXT_WHITE))

/* 控制台结构 */
struct console_object {
    unsigned int originalAddr;          /* 控制台对应的显存的位置 */
    unsigned int screenSize;       /* 控制台占用的显存大小 */
    unsigned char color;                /* 字符的颜色 */
    int x, y;                  /* 偏移坐标位置 */
};

/* 控制台表 */
struct console_object console_object;

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

static unsigned int get_cursor()
{
	unsigned int posLow, posHigh;		//设置光标位置的高位的低位
	//取得光标位置
	out8(CRTC_ADDR_REG, CURSOR_H);			//光标高位
	posHigh = in8(CRTC_DATA_REG);
	out8(CRTC_ADDR_REG, CURSOR_L);			//光标低位
	posLow = in8(CRTC_DATA_REG);
	
	return (posHigh<<8 | posLow);	//返回合成后的值
}

static void set_start_addr(unsigned short addr)
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
static void flush(struct console_object *obj)
{
    /* 计算光标位置，并设置 */
    set_cursor(obj->originalAddr + obj->y * SCREEN_WIDTH + obj->x);
    set_start_addr(obj->originalAddr);
}

/**
 * clean_screen - 清除控制台
 * @console: 控制台
 */
void clean_screen(struct console_object *obj)
{
    /* 指向显存 */
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + obj->originalAddr * 2);
	int i;
	for(i = 0; i < obj->screenSize * 2; i += 2){
		*vram++ = '\0';  /* 所有字符都置0 */
        *vram++ = COLOR_DEFAULT;  /* 颜色设置为黑白 */
	}
    obj->x = 0;
    obj->y = 0;
    flush(obj);
}

#ifdef DEBUG_CONSOLE
void DumpConsole(struct console_object *obj)
{
    printk(PART_TIP "----Console----\n");
    printk(PART_TIP "origin:%d size:%d x:%d y:%d color:%x chrdev:%x\n",
        obj->originalAddr, obj->screenSize, obj->x, obj->x, obj->color, obj->chrdev);
}
#endif /* DEBUG_CONSOLE */

/**
 * scroll_sceen - 滚屏
 * @console: 控制台
 * @direction: 滚动方向
 *             - SCREEN_UP: 向上滚动
 *             - SCREEN_DOWN: 向下滚动
 * 
 */
static void scroll_sceen(struct console_object *obj, int direction)
{
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + obj->originalAddr * 2);
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
        obj->y--;
	}
	flush(obj);
}

/**
 * put_char - 控制台上输出一个字符
 * @console: 控制台
 * @ch: 字符
 */
static void put_char(struct console_object *obj, char ch)
{
    #ifndef CONFIG_PRINT_CONSOLE
    return;
    #endif
	unsigned char *vram = (unsigned char *)(V_MEM_BASE + 
        (obj->originalAddr + obj->y * SCREEN_WIDTH + obj->x) *2) ;
	switch(ch){
        case '\r':
            break;
		case '\n':
            // 如果是回车，那还是要把回车写进去
            *vram++ = '\0';
            *vram = COLOR_DEFAULT;
            obj->x = 0;
            obj->y++;
            
			break;
		case '\b':
            if (obj->x >= 0 && obj->y >= 0) {
                obj->x--;
                /* 调整为上一行尾 */
                if (obj->x < 0) {
                    obj->x = SCREEN_WIDTH - 1;
                    obj->y--;
                    /* 对y进行修复 */
                    if (obj->y < 0)
                        obj->y = 0;
                }
                *(vram-2) = '\0';
				*(vram-1) = COLOR_DEFAULT;
            }
			break;
		default: 
            *vram++ = ch;
			*vram = obj->color;

			obj->x++;
            if (obj->x > SCREEN_WIDTH - 1) {
                obj->x = 0;
                obj->y++;
            }
			break;
	}
    /* 滚屏 */
    while (obj->y > SCREEN_HEIGHT - 1) {
        scroll_sceen(obj, SCREEN_DOWN);
    }

    flush(obj);
}

/**
 * console_putchar - 控制台调试输出
 * @ch:字符
 */
void console_putchar(char ch)
{
    
    put_char(&console_object, ch);
}

/**
 * init_console_hw - 初始化控制台调试驱动
 */
void init_console_hw()
{
    struct console_object *obj = &console_object;

    /* 设置屏幕大小 */
    obj->screenSize = SCREEN_SIZE;

	/* 控制台起始地址 */
    obj->originalAddr = 0;
    
    /* 设置默认颜色 */
    obj->color = COLOR_DEFAULT;

    /* 消除编译未使用提示 */
    get_cursor();

    clean_screen(obj);
    /* 默认在左上角 */
    obj->x = 0;
    obj->y = 0;
    set_cursor(obj->y * SCREEN_WIDTH + obj->x);
}
#endif