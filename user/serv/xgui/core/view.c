#include "xgui_view.h"
#include "xgui_hal.h"
#include "xgui_misc.h"
#include <stdlib.h>
#include <stdio.h>

static LIST_HEAD(xgui_view_show_list_head);
static LIST_HEAD(xgui_view_global_list_head);

xgui_view_t *xgui_view_new(int x, int y, int width, int height)
{
    xgui_view_t *view = malloc(sizeof(xgui_view_t));
    if (view == NULL) {
        printf("malloc for view failed!\n");
        return NULL;
    }
    view->section = xgui_section_new(width, height);
    if (!view->section) {
        printf("new section failed!\n");
        free(view);
        return NULL;
    }
    view->x = x;
    view->y = y;
    view->z = -1;
    view->width = width;
    view->height = height;
    /* 添加到全局链表上，以备显示时加入显示链表 */
    list_add(&view->list, &xgui_view_global_list_head);
    return view;
}

int xgui_view_put(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (xgui_section_put(view->section) < 0)
        return -1;
    list_del_init(&view->list);
    free(view);
    return 0;
}

static int xgui_view_count_z(xgui_view_t *view)
{
    if (!view)
        return -1;
    int z = 0;
    xgui_view_t *tmp;
    list_for_each_owner (tmp, &xgui_view_show_list_head, list) {
        if (tmp == view) {
            return z;
        }
        z++;
    }
    return z;
}

int xgui_view_show(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &xgui_view_global_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add_tail(&view->list, &xgui_view_show_list_head);
    view->z = xgui_view_count_z(view);
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, view->z, view->z);
    return 0;
}

int xgui_view_hide(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &xgui_view_show_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add(&view->list, &xgui_view_global_list_head);
    view->z = -1;
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, 0, xgui_view_get_top()->z);
    return 0;
}

int xgui_view_move_to_top(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &xgui_view_show_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add_tail(&view->list, &xgui_view_show_list_head);
    view->z = xgui_view_count_z(view);
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, view->z, xgui_view_get_top()->z);
    return 0;
}

int xgui_view_move_to_bottom(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &xgui_view_show_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add(&view->list, &xgui_view_show_list_head);
    view->z = xgui_view_count_z(view);
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, 0, xgui_view_get_top()->z);
    return 0;
}

int xgui_view_move_under_top(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &xgui_view_show_list_head) || list_length(&xgui_view_show_list_head) < 2) {
        return -1;
    }
    list_del_init(&view->list);
    xgui_view_t *top_view = xgui_view_get_top();
    list_add_before(&view->list, &top_view->list);
    view->z = xgui_view_count_z(view);
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, view->z, xgui_view_get_top()->z);
    return 0;
}

int xgui_view_slide(xgui_view_t *view, int x, int y)
{
    if (!view)
        return -1;
    int old_x = view->x;
    int old_y = view->y;
    view->x = x;
    view->y = y;
    if (view->z >= 0) {
        int x0, y0, x1, y1;
        x0 = min(old_x, x);
        y0 = min(old_y, y);
        x1 = max(old_x + view->width, x + view->width);
        y1 = max(old_y + view->height, y + view->height);
        xgui_view_refresh_by_z(x0, y0, x1, y1, 0, view->z);
    }
    return 0;
}

xgui_view_t *xgui_view_get_top()
{
    xgui_view_t *view = list_last_owner_or_null(&xgui_view_show_list_head, xgui_view_t, list);
    return view;
}

xgui_view_t *xgui_view_get_bottom()
{
    xgui_view_t *view = list_first_owner_or_null(&xgui_view_show_list_head, xgui_view_t, list);
    return view;
}

int xgui_view_refresh_one(xgui_view_t *view)
{
    if (!view)
        return -1;
    xgui_bitmap_t bitmap;
    bitmap.region.x = 0;
    bitmap.region.y = 0;
    bitmap.region.w = view->width;
    bitmap.region.h = view->height;
    bitmap.width = view->width;
    bitmap.height = view->height;
    bitmap.colors = (xgui_color_t *)view->section->addr;
    xgui_screen_write_bitmap(view->x, view->y, &bitmap);
    return 0;
}

int xgui_view_refresh(xgui_view_t *view)
{
    if (!view)
        return -1;
    xgui_view_t *tmp;
    int show_flag = 0;
    list_for_each_owner (tmp, &xgui_view_show_list_head, list) {
        if (tmp == view) {
            show_flag = 1;
        }
        if (show_flag) {
            xgui_view_refresh_one(tmp);
        }
    }
    return 0;
}

void xgui_view_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1)
{
    int view_left, view_top, view_right, view_buttom;

    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > xgui_screen_get_width() - 1)
        right = xgui_screen_get_width() - 1;
	if (buttom > xgui_screen_get_height() - 1)
        buttom = xgui_screen_get_height() - 1;
    
    int vx, vy;

    int z = 0;
    xgui_color_t color;
    xgui_color_t *buf;
    xgui_view_t *view;
    list_for_each_owner (view, &xgui_view_show_list_head, list) {
        if (z >= z0 && z <= z1) {
            view_left = left - view->x;
            view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (view_top < 0)
                view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            for (vy = view_top; vy < view_buttom; vy++) {
                for (vx = view_left; vx < view_right; vx++) {
                    buf = (xgui_color_t *)view->section->addr;
                    color = buf[vy * view->width + vx];
                    xgui_screen_write_pixel(view->x + vx, view->y + vy, color);
                }
            }
        }
        z++;
    }
}

/**
 * button: 桌面
 * 一般窗口
 * 系统预留窗口
 * top: 鼠标
*/

int xgui_view_init()
{
    xgui_view_t *view0 = xgui_view_new(0, 0, 800, 600);
    xgui_section_fill_rect(view0->section, XCOLOR_RED);
    xgui_view_show(view0);

    xgui_view_t *view = xgui_view_new(100, 100, 400, 300);
    xgui_section_fill_rect(view->section, XCOLOR_BLUE);
    xgui_view_show(view);

    view = xgui_view_new(300, 300, 400, 300);
    xgui_section_fill_rect(view->section, XCOLOR_GREEN);
    xgui_view_show(view);

    view = xgui_view_new(400, 300, 32, 32);
    xgui_section_fill_rect(view->section, XCOLOR_WHITE);
    xgui_view_show(view);

    return 0;
}