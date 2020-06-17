#ifndef __TERMINAL_CONSOLE_H__
#define __TERMINAL_CONSOLE_H__

#include <sgi/sgi.h>
#include <stddef.h>

/* 80*25*20 = 10kb */
#define CON_FRAME_NR    20

#define CON_CHAR_WIDTH  8
#define CON_CHAR_HEIGHT 16

#define CON_CLOUMNS     40 
#define CON_ROWS        15

#define CON_BUFFER_SIZE (CON_CLOUMNS * CON_ROWS * CON_FRAME_NR)

#define CON_SCREEN_WIDTH (CON_CHAR_WIDTH * CON_CLOUMNS)
#define CON_SCREEN_HEIGHT (CON_CHAR_HEIGHT * CON_ROWS)

#define CON_SCREEN_BG_COLOR    SGIC_GREEN
#define CON_SCREEN_FONT_COLOR  SGIC_RED
#define CON_SCREEN_SELECT_COLOR  SGIC_YELLOW


/* ANSC */
#define CON_CODEPAGE    0

#define CON_SCROLL_UP   -1
#define CON_SCROLL_DOWN 1

typedef struct {
    char *buffer;       /* 字符缓冲区 */
    char *cur_pos;      /* 当前缓冲区的起始位置 */
    size_t buflen;      /* 缓冲区长度：最大长度 */
    short width;    /* 窗口宽度 */
    short height;   /* 窗口高度 */
    char columns;   /* 窗口列数 */
    char rows;      /* 窗口行数 */
    char char_width;    /* 字符宽度 */
    char char_height;   /* 字符高度 */
    int codepage;       /* 代码页 */
    SGI_Argb background_color;  /* 背景颜色 */
    SGI_Argb font_color;        /* 字体颜色 */
    SGI_Argb select_color;      /* 选中时的颜色 */
    
    int mousex;     /* 鼠标横坐标 */
    int mousey;     /* 鼠标纵坐标 */

/// 窗口相关    
    SGI_Display *display;       /* 屏幕的显示 */
    SGI_Window win;             /* 屏幕的窗口 */
    SGI_FontInfo *font; /* 绑定一个字体 */

/// 函数指针
    void (*outc) (char);
    void (*outs) (char *);
    void (*clear) ();
    void (*clear_area) (int, int, unsigned int, unsigned int);
    
} con_screen_t;

extern con_screen_t screen;

int init_con_screen();
void scroll_screen(int dir, int lines, int cursorx, int cursory);
int cprintf(const char *fmt, ...);
void con_set_chars(char ch, int counts, int x, int y);
void print_prompt();
void con_put_str(char *str);
void con_ouput_visual(char ch, int x, int y);
void con_region_chars(int x0, int y0, int x1, int y1);
void con_flush();

static inline void con_get_char(char *ch, int x, int y)
{
    if (screen.cur_pos + y * screen.columns + x > screen.buffer + screen.buflen)
        return;

	*ch = screen.cur_pos[y * screen.columns + x];
}

static inline void con_set_char(char ch, int x, int y)
{
    if (screen.cur_pos + y * screen.columns + x > screen.buffer + screen.buflen)
        return;
    
	//保存字符
	screen.cur_pos[y * screen.columns + x] = ch;
}


#endif  /* __TERMINAL_CONSOLE_H__ */