#ifndef _BOSH_CONSOLE_H
#define _BOSH_CONSOLE_H

#include <stddef.h>
#include <gcolor.h>

/* 80*25*20 = 10kb */
#define CON_FRAME_NR    20

#define CON_CHAR_WIDTH  8
#define CON_CHAR_HEIGHT 16

#define CON_SCREEN_BG_COLOR    GC_BLACK
#define CON_SCREEN_FONT_COLOR  GC_WHITE

#define CON_MOUSE_COLOR  GC_BLUE

/* ANSC */
#define CON_CODEPAGE    0

#define CON_SCROLL_UP   -1
#define CON_SCROLL_DOWN 1

#define CON_RDPIPE_BUF_LEN  1024

typedef struct {
    char *buffer;       /* 字符缓冲区 */
    char *cur_pos;      /* 当前缓冲区的起始位置 */
    size_t buflen;      /* 缓冲区长度：最大长度 */
    short width;    /* 窗口宽度 */
    short height;   /* 窗口高度 */
    short columns;   /* 窗口列数 */
    short rows;      /* 窗口行数 */
    char char_width;    /* 字符宽度 */
    char char_height;   /* 字符高度 */
    int columns_width;   /* 总列宽度 */
    int rows_height;      /* 总行高度 */
    int codepage;       /* 代码页 */
    g_color_t background_color;  /* 背景颜色 */
    g_color_t font_color;        /* 字体颜色 */
    g_color_t mouse_color;  /* 鼠标颜色 */
/// 函数指针
    void (*outc) (char);
    void (*outs) (char *);
    void (*clear) ();
    void (*flush) ();
    void (*clear_area) (int, int, unsigned int, unsigned int);
} con_screen_t;

extern con_screen_t con_screen;

int init_console();
void scroll_screen(int dir, int lines, int cursorx, int cursory);
int con_printf(const char *fmt, ...);
void con_set_chars(char ch, int counts, int x, int y);
void print_prompt();
void con_put_str(char *str);
void con_ouput_visual(char ch, int x, int y);
void con_region_chars(int x0, int y0, int x1, int y1);
void con_flush();
void con_flush2(int mx0, int my0, int mx1, int my1);
void con_flush_area(int x0, int y0, int x1, int y1);
void con_get_chars(char *buf, int counts, int x, int y);
void con_set_back_color(g_color_t color);
void con_set_font_color(g_color_t color);
void con_putchar(char ch);
int con_get_key(int kcode, int kmod);
int con_xmit_key(int kcode, int kmod);

static inline void con_get_char(char *ch, int x, int y)
{
    if (con_screen.cur_pos + y * con_screen.columns + x > con_screen.buffer + con_screen.buflen)
        return;

	*ch = con_screen.cur_pos[y * con_screen.columns + x];
}

static inline void con_set_char(char ch, int x, int y)
{
    if (con_screen.cur_pos + y * con_screen.columns + x > con_screen.buffer + con_screen.buflen)
        return;
    
	//保存字符
	con_screen.cur_pos[y * con_screen.columns + x] = ch;
}

int con_loop();
int exit_console();

#define shell_printf con_printf
#define shell_putchar con_putchar

#endif  /* _BOSH_CONSOLE_H */