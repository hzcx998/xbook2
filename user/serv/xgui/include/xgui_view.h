#ifndef _XGUI_VIEW_H
#define _XGUI_VIEW_H

#include <stddef.h>
#include <sys/list.h>
#include "xgui_section.h"

/* 视图用来表达逻辑上的图层 */
typedef struct {
    list_t list;        // 视图构成一个有序链表
    int width;
    int height;
    int x;
    int y;
    int z;
    xgui_section_t *section;
} xgui_view_t;

int xgui_view_init();
xgui_view_t *xgui_view_new(int x, int y, int width, int height);
int xgui_view_put(xgui_view_t *view);
int xgui_view_show(xgui_view_t *view);
int xgui_view_hide(xgui_view_t *view);
int xgui_view_refresh(xgui_view_t *view);
xgui_view_t *xgui_view_get_top();
xgui_view_t *xgui_view_get_bottom();
int xgui_view_slide(xgui_view_t *view, int x, int y);
void xgui_view_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1);
int xgui_view_move_to_top(xgui_view_t *view);
int xgui_view_move_to_bottom(xgui_view_t *view);
int xgui_view_move_under_top(xgui_view_t *view);

#endif /* _XGUI_VIEW_H */