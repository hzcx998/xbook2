#ifndef _XGUI_VIEW_H
#define _XGUI_VIEW_H

#include <stddef.h>
#include <sys/list.h>
#include "xgui_section.h"

/* 视图用来表达逻辑上的图层 */
typedef struct {
    list_t list;        // 显示的视图链表
    list_t global_list; // 存在的视图链表
    int id;
    int width;
    int height;
    int x;
    int y;
    int z;
    xgui_section_t *section;
} xgui_view_t;

int xgui_view_init();
xgui_view_t *xgui_view_create(int x, int y, int width, int height);
int xgui_view_destroy(xgui_view_t *view);
xgui_view_t *xgui_view_get_top();
xgui_view_t *xgui_view_get_bottom();
xgui_view_t *xgui_view_find_by_id(int id);
int xgui_view_set_xy(xgui_view_t *view, int x, int y);

void xgui_view_set_z(xgui_view_t *view, int z);
int xgui_view_move_to_top(xgui_view_t *view);
int xgui_view_move_to_bottom(xgui_view_t *view);
int xgui_view_move_under_top(xgui_view_t *view);
int xgui_view_move_upper_top(xgui_view_t *view);
int xgui_view_hide(xgui_view_t *view);
int xgui_view_show(xgui_view_t *view);

void xgui_refresh_view_by_z(int left, int top, int right, int buttom, int z0, int z1);
void xgui_view_refresh(xgui_view_t *view, int left, int top, int right, int buttom);
#define view_self_refresh(view) xgui_view_refresh((view), 0, 0, view->width, view->height)

#endif /* _XGUI_VIEW_H */