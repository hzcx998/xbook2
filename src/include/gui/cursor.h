#ifndef __TERMINAL_CURSOR_H__
#define __TERMINAL_CURSOR_H__

#include <stddef.h>
#include <gui/color.h>

#define CON_CURSOR_COLOR    COLOR_WHITE

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
    GUI_COLOR color;     /* 光标颜色 */
    int x;              /* 光标的字符位置 */
    int y;              /* 光标的字符位置 */
    char visual;        /* 是否可见 */
} con_cursor_t;

extern con_cursor_t cursor;

void init_con_cursor();
void set_cursor_size();
void draw_cursor();
void draw_char(char ch);
void get_cursor(int *x, int *y);
void move_cursor(int x, int y);
void set_cursor_shape(int shape);
void set_cursor_color(GUI_COLOR color);
void move_cursor_off(int x, int y);
void cursor_pos_check();
void clean_cursor();
void focus_cursor();
void print_cursor();

#endif  /* __TERMINAL_CURSOR_H__ */