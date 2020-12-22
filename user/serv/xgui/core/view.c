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

int xgui_view_show(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &xgui_view_global_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add_tail(&view->list, &xgui_view_show_list_head);
    view->z = 0;
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
    return 0;
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
    xgui_screen_out_bitmap(view->x, view->y, &bitmap);
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

int xgui_view_init()
{
    xgui_view_t *view0 = xgui_view_new(0, 0, 800, 600);
    xgui_view_show(view0);
    xgui_section_fill_rect(view0->section, XCOLOR_RED);
    //xgui_view_refresh(view0);
    xgui_view_t *view = xgui_view_new(100, 100, 400, 300);
    xgui_view_show(view);
    xgui_section_fill_rect(view->section, XCOLOR_BLUE);
    //xgui_view_refresh(view);
    view = xgui_view_new(300, 300, 400, 300);
    xgui_view_show(view);
    xgui_section_fill_rect(view->section, XCOLOR_GREEN);
    xgui_view_refresh(view0);
    
    return 0;
}