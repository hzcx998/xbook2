#ifndef __TERMINAL_CONSOLE_H__
#define __TERMINAL_CONSOLE_H__

#include <stddef.h>
#include <graph/color.h>

/* 80*25*20 = 10kb */
#define CON_FRAME_NR    20

#define CON_CHAR_WIDTH  8
#define CON_CHAR_HEIGHT 16

#define CON_CLOUMNS     80 
#define CON_ROWS        25

#define CON_BUFFER_SIZE (CON_CLOUMNS * CON_ROWS * CON_FRAME_NR)

#define CON_SCREEN_WIDTH (CON_CHAR_WIDTH * CON_CLOUMNS)
#define CON_SCREEN_HEIGHT (CON_CHAR_HEIGHT * CON_ROWS)

#define CON_SCREEN_BG_COLOR    COLOR_BLACK
#define CON_SCREEN_FONT_COLOR  COLOR_WHITE


#define CON_MOUSE_COLOR  COLOR_BLUE

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
    char columns;   /* 窗口列数 */
    char rows;      /* 窗口行数 */
    char char_width;    /* 字符宽度 */
    char char_height;   /* 字符高度 */
    int columns_width;   /* 总列宽度 */
    int rows_height;      /* 总行高度 */
    int codepage;       /* 代码页 */
    GUI_COLOR background_color;  /* 背景颜色 */
    GUI_COLOR font_color;        /* 字体颜色 */

    GUI_COLOR mouse_color;  /* 鼠标颜色 */
    
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
void con_flush2(int mx0, int my0, int mx1, int my1);
void con_flush_area(int x0, int y0, int x1, int y1);
void con_get_chars(char *buf, int counts, int x, int y);
void con_set_back_color(GUI_COLOR color);
void con_set_font_color(GUI_COLOR color);

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

int con_event_poll(char *buf, int pid);
int con_event_loop(char *buf, int count);
int con_loop();


#endif  /* __TERMINAL_CONSOLE_H__ */