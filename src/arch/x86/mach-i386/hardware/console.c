#include <arch/config.h>

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
#define TEXT_YELLOW  0x6     /* 0110 */
#define TEXT_MAGENTA 0x3     /* 0011 */

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

typedef struct {
    unsigned int originalAddr;          /* 控制台对应的显存的位置 */
    unsigned int screenSize;            /* 控制台占用的显存大小 */
    unsigned char color;                /* 字符的颜色 */
    int x, y;                           /* 偏移坐标位置 */
    int esc_step;   
    unsigned char ready_color;          /* 准备设置成的颜色 */
} console_hardware_t;

console_hardware_t console_obj;

static void set_cursor(unsigned short cursor)
{
	unsigned long flags;
    interrupt_save_and_disable(flags);
	out8(CRTC_ADDR_REG, CURSOR_H);
	out8(CRTC_DATA_REG, (cursor >> 8) & 0xFF);
	out8(CRTC_ADDR_REG, CURSOR_L);
	out8(CRTC_DATA_REG, cursor & 0xFF);
    interrupt_restore_state(flags);
}

static unsigned int get_cursor()
{
	unsigned int posLow, posHigh;
	out8(CRTC_ADDR_REG, CURSOR_H);
	posHigh = in8(CRTC_DATA_REG);
	out8(CRTC_ADDR_REG, CURSOR_L);
	posLow = in8(CRTC_DATA_REG);
	return (posHigh<<8 | posLow);
}

static void set_start_addr(unsigned short addr)
{
	unsigned long flags;
    interrupt_save_and_disable(flags);
	out8(CRTC_ADDR_REG, START_ADDR_H);
	out8(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out8(CRTC_ADDR_REG, START_ADDR_L);
	out8(CRTC_DATA_REG, addr & 0xFF);
    interrupt_restore_state(flags);
}

static void flush(console_hardware_t *obj)
{
    set_cursor(obj->originalAddr + obj->y * SCREEN_WIDTH + obj->x);
    set_start_addr(obj->originalAddr);
}

void clean_screen(console_hardware_t *obj)
{
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + obj->originalAddr * 2);
	int i;
	for(i = 0; i < obj->screenSize * 2; i += 2){
		*vram++ = '\0';
        *vram++ = COLOR_DEFAULT;
	}
    obj->x = 0;
    obj->y = 0;
    flush(obj);
}

#ifdef DEBUG_CONSOLE
void dump_console(console_hardware_t *obj)
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
static void scroll_sceen(console_hardware_t *obj, int direction)
{
    unsigned char *vram = (unsigned char *)(V_MEM_BASE + obj->originalAddr * 2);
    int i;
                
	if(direction == SCREEN_UP){
        for (i = SCREEN_WIDTH * 2 * 24; i > SCREEN_WIDTH * 2; i -= 2) {
            vram[i] = vram[i - SCREEN_WIDTH * 2];
            vram[i + 1] = vram[i + 1 - SCREEN_WIDTH * 2];
        }
        for (i = 0; i < SCREEN_WIDTH * 2; i += 2) {
            vram[i] = '\0';
            vram[i + 1] = COLOR_DEFAULT;
        }

	}else if(direction == SCREEN_DOWN){
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

static void put_char(console_hardware_t *obj, char ch)
{
	unsigned char *vram = (unsigned char *)(V_MEM_BASE + 
        (obj->originalAddr + obj->y * SCREEN_WIDTH + obj->x) *2) ;
    if ('0' < ch && ch <= '9') {
        if (obj->esc_step == 5) {
            char _ch = ch - '0';
            switch (_ch)
            {
            case 1:
                obj->ready_color = MAKE_COLOR(TEXT_BLACK, TEXT_RED);
                obj->esc_step++;
                return;
            case 2:
                obj->ready_color = MAKE_COLOR(TEXT_BLACK, TEXT_GREEN);
                obj->esc_step++;
                return;
            case 3:
                obj->ready_color = MAKE_COLOR(TEXT_BLACK, TEXT_YELLOW);
                obj->esc_step++;
                return;
            case 4:
                obj->ready_color = MAKE_COLOR(TEXT_BLACK, TEXT_BLUE);
                obj->esc_step++;
                return;
            case 5:
                obj->ready_color = MAKE_COLOR(TEXT_BLACK, TEXT_MAGENTA);
                obj->esc_step++;
                return;
            case 7:
                obj->ready_color = MAKE_COLOR(TEXT_BLACK, TEXT_WHITE);
                obj->esc_step++;
                return;
            default:
                break;
            }
        }
    }
    switch(ch){
        case '\e': // break start
            obj->esc_step++;
            break;
        case '\r':
            break;
		case '\n':
            *vram++ = '\0';
            *vram = COLOR_DEFAULT;
            obj->x = 0;
            obj->y++;
            
			break;
		case '\b':
            if (obj->x >= 0 && obj->y >= 0) {
                obj->x--;
                if (obj->x < 0) {
                    obj->x = SCREEN_WIDTH - 1;
                    obj->y--;
                    if (obj->y < 0)
                        obj->y = 0;
                }
                *(vram-2) = '\0';
				*(vram-1) = COLOR_DEFAULT;
            }
			break;
        case '[':
            if (obj->esc_step == 1) {
                obj->esc_step++;
                break;
            }
        case '0':
            if (obj->esc_step == 2) {
                obj->esc_step++;
                break;
            }
        case ';':
            if (obj->esc_step == 3) {
                obj->esc_step++;
                break;
            }
        case 'm':
            if (obj->esc_step == 3) {
                obj->esc_step = 0;
                obj->color = COLOR_DEFAULT;
                break;
            } else if (obj->esc_step == 6) {
                obj->esc_step = 0;
                obj->color = obj->ready_color;
                break;
            }
        case '3':
            if (obj->esc_step == 4) {
                obj->esc_step++;
                break;
            }
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
    while (obj->y > SCREEN_HEIGHT - 1) {
        scroll_sceen(obj, SCREEN_DOWN);
    }

    flush(obj);
}

void console_hardware_putchar(char ch)
{
    put_char(&console_obj, ch);
}

void console_hardware_init()
{
    console_hardware_t *obj = &console_obj;

    obj->screenSize = SCREEN_SIZE;

    obj->originalAddr = 0;
    
    obj->color = COLOR_DEFAULT;
    obj->esc_step = 0;
    obj->ready_color = 0;
    get_cursor();

    clean_screen(obj);
    obj->x = 0;
    obj->y = 0;
    set_cursor(obj->y * SCREEN_WIDTH + obj->x);
}
