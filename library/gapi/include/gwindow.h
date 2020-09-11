#ifndef _GAPI_WINDOW_H
#define _GAPI_WINDOW_H

#include <stdint.h>
#include <sys/list.h>
#include "gcolor.h"
#include "gshape.h"
#include "gbitmap.h"

#define GW_TITLE_LEN    64

#define GW_ON_BACK_COLOR  GC_ARGB(128, 245, 245, 245)
#define GW_ON_BOARD_COLOR GC_RGB(200, 200, 200)
#define GW_ON_FRONT_COLOR GC_RGB(230, 230, 230)
#define GW_ON_FONT_COLOR  GC_RGB(25, 25, 25)

#define GW_OFF_BACK_COLOR  GC_ARGB(128, 235, 235, 235)
#define GW_OFF_BOARD_COLOR GC_RGB(190, 190, 190)
#define GW_OFF_FRONT_COLOR GC_RGB(210, 210, 210)
#define GW_OFF_FONT_COLOR  GC_RGB(128, 128, 128)

enum {
    GW_MAXIM        = (1 << 0),
    GW_RESIZE       = (1 << 1),
    GW_RESIZE_EX    = (1 << 2),
    GW_NO_MAXIM     = (1 << 3), /* 没有最大化按钮 */
    GW_NO_MINIM     = (1 << 4), /* 没有最小化按钮 */
};

#define GW_BTN_SIZE 16

/* 窗口标题栏高度 */
#define GW_TITLE_BAR_HIGHT 24

/* 图形窗口结构 */
typedef struct {
    list_t wlist;       /* 进程的窗口链表，一个进程可以有多个窗口 */
    list_t touch_list;  /* 窗口触碰块链表 */
    int layer;          /* 窗口对应的图层，图层是内核的图形抽象单元 */
    uint32_t flags;     /* 窗口的标志 */
    /* 窗口的位置和大小（和图层同步） */
    int x;              
    int y;       
    uint32_t width;     
    uint32_t height;
    uint32_t win_width;     /* 窗体宽度 */
    uint32_t win_height;    /* 窗体高度 */
    g_rect_t backup;    /* 窗口信息备份，当使用最大化功能时需要 */
    g_region_t body_region; /* 窗体区域 */
    g_rect_t invalid_rect;  /* 脏矩形 */
    g_bitmap_t *wbmp;       /* 窗口绘制时的位图 */
    char title[GW_TITLE_LEN];   /* 窗口的标题 */
} g_window_t;

int g_new_window(char *title, int x, int y, uint32_t width, uint32_t height, uint32_t flags);
int g_del_window(int win);
int g_del_window_all();
int g_show_window(int win);

int g_hide_window(int win);
g_window_t *g_find_window(int win);
int g_resize_window(int win, uint32_t width, uint32_t height);
int g_focus_window(int win, int turn);
int g_maxim_window(int win);

int g_enable_window_resize(int win);
int g_disable_window_resize(int win);
int g_set_window_minresize(int win, uint32_t min_width, uint32_t min_height);

int g_window_put_point(int win, int x, int y, g_color_t color);
int g_window_get_point(int win, int x, int y, g_color_t *color);
int g_window_rect_fill(int win, int x, int y, uint32_t width, uint32_t height, g_color_t color);
int g_window_rect(int win, int x, int y, uint32_t width, uint32_t height, g_color_t color);
int g_refresh_window_rect(int win, int x, int y, uint32_t width, uint32_t height);
int g_refresh_window_region(int win, int left, int top, int right, int bottom);
int g_window_paint(int win, int x, int y, g_bitmap_t *bmp);
int g_window_paint_ex(int win, int x, int y, g_bitmap_t *bmp);

int g_update_window(int win);
int g_invalid_rect(int win, int x, int y, uint32_t width, uint32_t height);
int g_invalid_window(int win);
int g_get_invalid(int win, int *x, int *y, uint32_t *width, uint32_t *height);

int g_window_char(
    int win,
    int x,
    int y,
    char ch,
    uint32_t color);
int g_window_text(
    int win,
    int x,
    int y,
    char *text,
    uint32_t color);
int g_set_window_icon(int win, char *path);

#endif  /* _GAPI_WINDOW_H */