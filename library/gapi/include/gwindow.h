#ifndef _GAPI_WINDOW_H
#define _GAPI_WINDOW_H

#include <stdint.h>
#include <sys/list.h>
#include "gcolor.h"
#include "gshape.h"

#define GW_TITLE_LEN    64

#define GW_ON_BACK_COLOR  GC_RGB(245, 245, 245)
#define GW_ON_BOARD_COLOR GC_RGB(200, 200, 200)
#define GW_ON_FRONT_COLOR GC_RGB(230, 230, 230)

#define GW_OFF_BACK_COLOR  GC_RGB(235, 235, 235)
#define GW_OFF_BOARD_COLOR GC_RGB(190, 190, 190)
#define GW_OFF_FRONT_COLOR GC_RGB(210, 210, 210)

enum {
    GW_MAXIM        = (1 << 0),
};

/* 图形窗口结构 */
typedef struct {
    list_t wlist;       /* 进程的窗口链表，一个进程可以有多个窗口 */
    int layer;          /* 窗口对应的图层，图层是内核的图形抽象单元 */
    uint32_t flags;     /* 窗口的标志 */
    /* 窗口的位置和大小（和图层同步） */
    int x;              
    int y;       
    uint32_t width;
    uint32_t height;
    g_rect_t backup;    /* 窗口信息备份，当使用最大化功能时需要 */
    char title[GW_TITLE_LEN];   /* 窗口的标题 */
} g_window_t;

int g_new_window(char *title, int x, int y, uint32_t width, uint32_t height);
int g_del_window(int win);
int g_show_window(int win);

int g_update_window(int win);

int g_hide_window(int win);
g_window_t *g_find_window(int win);
int g_resize_window(int win, uint32_t width, uint32_t height);
int g_focus_window(int win, int turn);
int g_maxim_window(int win);

#endif  /* _GAPI_WINDOW_H */