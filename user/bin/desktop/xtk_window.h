#ifndef _LIB_XTK_WINDOW_H
#define _LIB_XTK_WINDOW_H

#include <stddef.h>
#include "xtk_spirit.h"

enum {
    XTK_WINDOW_ACTIVE       = (1 << 1), /* 窗口处于激活状态 */
    XTK_WINDOW_SHOW         = (1 << 2), /* 窗口处于显示状态 */
};

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

typedef struct {
    xtk_spirit_t *title;

} xtk_window_navigation_t;

typedef struct {
    xtk_spirit_t spirit;    // 放到第一个成员，实现继承
    int content_width;
    int content_height;
    xtk_window_style_t *style;
    int view;               // 窗口视图
    uint32_t winflgs;       // 窗口的标志
    xtk_window_navigation_t navigation;
} xtk_window_t;

#define XTK_WINDOW(spirit)  ((xtk_window_t *)(spirit))
xtk_spirit_t *xtk_window_create(char *title, int x, int y, int width, int height, uint32_t flags);


#endif /* _LIB_XTK_WINDOW_H */