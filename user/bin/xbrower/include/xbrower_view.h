#ifndef _XGUI_VIEW_H
#define _XGUI_VIEW_H

#include <stddef.h>
#include <sys/list.h>
#include "xbrower_section.h"

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
    xbrower_section_t *section;
} xbrower_view_t;

int xbrower_view_init();
void xbrower_view_exit();
xbrower_view_t *xbrower_view_create(int x, int y, int width, int height);
int xbrower_view_destroy(xbrower_view_t *view);
xbrower_view_t *xbrower_view_get_top();
xbrower_view_t *xbrower_view_get_bottom();
xbrower_view_t *xbrower_view_find_by_id(int id);
int xbrower_view_set_xy(xbrower_view_t *view, int x, int y);

void xbrower_view_set_z(xbrower_view_t *view, int z);
int xbrower_view_move_to_top(xbrower_view_t *view);
int xbrower_view_move_to_bottom(xbrower_view_t *view);
int xbrower_view_move_under_top(xbrower_view_t *view);
int xbrower_view_move_upper_top(xbrower_view_t *view);
int xbrower_view_hide(xbrower_view_t *view);
int xbrower_view_show(xbrower_view_t *view);

void xbrower_refresh_view_by_z(int left, int top, int right, int buttom, int z0, int z1);
void xbrower_view_refresh(xbrower_view_t *view, int left, int top, int right, int buttom);
#define view_self_refresh(view) xbrower_view_refresh((view), 0, 0, view->width, view->height)

#endif /* _XGUI_VIEW_H */