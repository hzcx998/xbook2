#ifndef _WINCTL_H
#define _WINCTL_H

#include <gapi.h>
#include <sys/list.h>

/* 窗口控制图标宽高 */
#define WINCTL_ICON_SIZE       36

#define WINCTL_WIDTH       48

#define WINCTL_BACK_COLOR    GC_RGB(164, 164, 164)
#define WINCTL_ACTIVE_COLOR  GC_RGB(192, 192, 192)

typedef struct {
    bool                ishidden;       /* 窗口隐藏 */
    bool                isfocus;        /* 窗口聚焦 */
    g_touch_t           *button;        /* 按钮 */
    g_layer_t           layer;          /* 要控制的图层 */
    list_t              list;           /* 窗口控制链表 */
} winctl_t;

/* 窗口控制管理器 */
typedef struct {
    g_layer_t           layer;          /* 所在图层 */
    g_color_t           back_color;          /* 背景颜色 */
    g_color_t           active_color;       /* 激活后的颜色 */
    list_t              winctl_list_head;   /* 窗口控制链表 */
} winctl_manager_t;

extern winctl_manager_t winctl_manager;
extern winctl_t *winctl_last;
int init_winctl_manager(g_layer_t layer);
winctl_t *create_winctl(g_layer_t layer);
int destroy_winctl(winctl_t *winctl);
winctl_t *winctl_find_by_layer(g_layer_t layer);

void winctl_get_focus(winctl_t *winctl);
void winctl_lost_focus(winctl_t *winctl);

void winctl_paint(winctl_t *winctl);
int winctl_set_icon(winctl_t *winctl);

#endif  /* _WINCTL_H */
