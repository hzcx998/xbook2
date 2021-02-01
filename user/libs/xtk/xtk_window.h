#ifndef _LIB_XTK_WINDOW_H
#define _LIB_XTK_WINDOW_H

#include <stddef.h>
#include <stdbool.h>
#include <sys/list.h>
#include "xtk_spirit.h"
#include "xtk_view.h"
#include "xtk_timer.h"

enum {
    XTK_WINDOW_ACTIVE       = (1 << 1), /* 窗口处于激活状态 */
    XTK_WINDOW_SHOW         = (1 << 2), /* 窗口处于显示状态 */
    XTK_WINDOW_MAXIM        = (1 << 3), /* 窗口处于最大化状态 */
    XTK_WINDOW_RESIZABLE    = (1 << 4), /* 可调整窗口大小 */
    XTK_WINDOW_DISABLERESIZE = (1 << 5), /* 禁止调整大小标志 */
};

typedef enum {
    XTK_WINDOW_TOPLEVEL = 0,    /* 普通的顶层窗口 */
    XTK_WINDOW_POPUP,           /* 弹出窗口 */
} xtk_window_type_t;

#define XTK_WINDOW_WIDTH_DEFAULT    320
#define XTK_WINDOW_HEIGHT_DEFAULT   240

typedef struct {
    int border_thick;       // 边框宽度
    int navigation_height;  // 导航高度
    uview_color_t background_color_active;
    uview_color_t background_color_inactive;
    uview_color_t front_color_active;
    uview_color_t front_color_inactive;
    uview_color_t border_color_active;
    uview_color_t border_color_inactive;
    uview_color_t text_color_active;
    uview_color_t text_color_inactive;
} xtk_window_style_t;

typedef enum {
    XTK_WIN_POS_NONE = 0,   /* 不固定 */
    XTK_WIN_POS_CENTER,     /* 居中 */
    XTK_WIN_POS_MOUSE,      /* 出现在鼠标位置 */
    XTK_WIN_POS_CENTER_ALWAYS,      /* 窗口总是居中 */
} xtk_window_position_t;

typedef struct {
    xtk_spirit_t *title;
} xtk_window_navigation_t;

typedef void (*xtk_window_routine_t) (xtk_spirit_t *, uview_msg_t *);
typedef void (*xtk_win_paint_callback_t) (xtk_spirit_t *, xtk_rect_t *);

typedef struct {
    xtk_spirit_t spirit;            // 放到第一个成员，实现继承
    xtk_spirit_t window_spirit;     // 绘制窗口内容
    int content_width;
    int content_height;
    xtk_window_style_t *style;
    uint32_t winflgs;       // 窗口的标志
    xtk_window_navigation_t navigation;
    xtk_window_type_t type;
    xtk_window_routine_t routine;
    xtk_win_paint_callback_t paint_callback;
    xtk_rect_t backup_win_info;
    xtk_rect_t invalid_rect;    // 无效区域，用PAIN消息
    list_t timer_list_head;     // 定时器
    xtk_surface_t mmap_surface;
} xtk_window_t;

#define XTK_WINDOW(spirit)  ((xtk_window_t *)(spirit))

xtk_spirit_t *xtk_window_create(xtk_window_type_t type);
int xtk_window_destroy(xtk_window_t *window);
int xtk_window_set_title(xtk_window_t *window, char *title);
int xtk_window_set_default_size(xtk_window_t *window, int width, int height);
int xtk_window_set_resizable(xtk_window_t *window, bool resizable);
int xtk_window_set_fixed(xtk_window_t *window, bool fiexed);
int xtk_window_set_position(xtk_window_t *window, xtk_window_position_t pos);
int xtk_window_set_routine(xtk_window_t *window, xtk_window_routine_t routine);
int xtk_window_set_active(xtk_window_t *window, bool is_active);
int xtk_window_reset_mobile_area(xtk_window_t *window);
int xtk_window_resize(xtk_window_t *window, int width, int height);
int xtk_window_get_screen(xtk_window_t *window, int *width, int *height);
int xtk_window_get_position(xtk_window_t *window, int *x, int *y);
int xtk_window_resize_to_screen(xtk_window_t *window);

xtk_surface_t *xtk_window_get_surface(xtk_window_t *window);

int xtk_window_show(xtk_window_t *window);

int xtk_window_flip(xtk_window_t *window);
int xtk_window_update(xtk_window_t *window, int x, int y, int w, int h);

int xtk_window_main(xtk_spirit_t *spirit, uview_msg_t *msg);
int xtk_window_quit(xtk_spirit_t *spirit);

void xtk_window_filter_msg(xtk_window_t *window, uview_msg_t *msg);

int xtk_window_draw_border(xtk_window_t *window, 
        int is_active, int redraw_bg);
int xtk_window_draw_no_border(xtk_window_t *window);

int xtk_window_load_mouse_cursors(xtk_window_t *window, char *pathname);
int xtk_window_maxim(xtk_window_t *window);

int xtk_window_invalid_rect(xtk_window_t *window, xtk_rect_t *rect);
int xtk_window_invalid_window(xtk_window_t *window);
int xtk_window_paint(xtk_window_t *window);
int xtk_window_get_invalid(xtk_window_t *window, xtk_rect_t *rect);
int xtk_window_refresh(xtk_window_t *window, int x, int y, int w, int h);

int xtk_window_paint_callback(xtk_window_t *window, xtk_win_paint_callback_t callback);

uint32_t xtk_window_add_timer(xtk_window_t *window, uint32_t interval, xtk_timer_callback_t function, void *data);
int xtk_window_remove_timer(xtk_window_t *window, uint32_t timer_id);
int xtk_window_restart_timer(xtk_window_t *window, uint32_t timer_id);

int xtk_window_mmap(xtk_window_t *window);
int xtk_window_munmap(xtk_window_t *window);
xtk_surface_t *xtk_window_get_mmap_surface(xtk_window_t *window);

#endif /* _LIB_XTK_WINDOW_H */