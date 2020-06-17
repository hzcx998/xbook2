#ifndef __TERMINAL_APP_H__
#define __TERMINAL_APP_H__

#include <sgi/sgi.h>
#include <stddef.h>

#define APP_NAME        "terminal"
#define APP_VERSION     0x01

/*
光标: 大中小，形状，颜色。
命令记录：缓冲区
编辑选项: 复制粘贴文本
字体编码：代码页。ANSI,utf-8
字体：大小，类型，字符宽高
布局：屏幕缓冲区大小（可以存放多少字），窗口大小（窗口内显示的字符数），窗口位置。
颜色：屏幕文字，屏幕背景
*/

#define CON_CURSOR_COLOR    SGIC_BLUE

enum cursor_shape {
    CS_SOLID_FRAME = 0,     /* 实心框:  8*16 */
    CS_HOLLOW_BOX,          /* 空心框:  8*16 */
    CS_VERTICAL_BAR,        /* 竖线:    1*16 */
    CS_UNDERLINE,           /* 下划线:  8*1 */
    CS_MAX_NR,
};

typedef struct {
    char width;         /* 光标宽度 */
    char height;        /* 光标高度 */
    char shape;         /* 光标形状 */
    SGI_Argb color;     /* 光标颜色 */
    int x;              /* 光标的字符位置 */
    int y;              /* 光标的字符位置 */
} con_cursor_t;

/*


*/


void init_con_cursor();
void set_cursor_size();
void draw_cursor();
void con_put_str(char *str);
void draw_char(char ch);
void get_cursor(int *x, int *y);
void move_cursor(int x, int y);

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
/// 窗口相关    
    SGI_Display *display;       /* 屏幕的显示 */
    SGI_Window win;             /* 屏幕的窗口 */
    SGI_FontInfo *font; /* 绑定一个字体 */

/// 函数指针
    void (*outc) (char);
    void (*outs) (char *);
    void (*clear) ();
    
} con_screen_t;

extern con_screen_t screen;

int init_con_screen();
void scroll_screen(int dir, int lines, int cursorx, int cursory);
void move_cursor_off(int x, int y);
int cprintf(const char *fmt, ...);

/* 单个命令缓冲区大小 */
#define CMD_BUF_SIZE    128

/* 命令缓冲区数量 */
#define CMD_BUF_NR    10

typedef struct {
    char cmdbuf[CMD_BUF_SIZE];
} cmd_buf_t;

void print_prompt();


#endif  /* __TERMINAL_APP_H__ */