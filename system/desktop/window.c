#include "desktop.h"
#include "gwindow.h"
#include <graph.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

LIST_HEAD(gwindow_list_head);

/**
 * 根据窗口id（图层id）查找一个窗口
 */
g_window_t *g_find_window(int win)
{
    g_window_t *gw;
    list_for_each_owner (gw, &gwindow_list_head, wlist) {
        if (gw->layer == win) {
            return gw;
        }
    }
    return NULL;
}

/**
 * 绘制窗口样式
 * @turn: 是否为聚焦状态，1为聚焦，0为失焦
 * 
 */
static void g_paint_window(int ly, uint32_t width, uint32_t height, int turn)
{
    uint32_t back, board, front;
    if (turn) {
        back = GW_ON_BACK_COLOR;
        board = GW_ON_BOARD_COLOR;
        front = GW_ON_FRONT_COLOR;
    } else {
        back = GW_OFF_BACK_COLOR;
        board = GW_OFF_BOARD_COLOR;
        front = GW_OFF_FRONT_COLOR;
    }
    g_layer_rect_fill(ly, 0, 0, width, height, back);
    g_layer_rect(ly, 0, 0, width, height, board);
    g_layer_line(ly, 0, 24, width, 24, board);

    g_layer_rect_fill(ly, 16, 4, 16, 16, front);
    g_layer_rect_fill(ly, 16 + 16 * 1 + 8 * 1, 4, 16, 16, front);
    g_layer_rect_fill(ly, 16 + 16 * 2 + 8 * 2, 4, 16, 16, front);
}

/**
 * 创建一个新的窗口，成功返回窗口标识，失败返回-1
 * 
 */
int g_new_window(char *title, int x, int y, uint32_t width, uint32_t height)
{
    if (!title || !width || !height)
        return -1;

    int ly = g_layer_new(x, y, width, height);
    if (ly < 0) {
        return -1;
    }
    
    /* alloc window */
    g_window_t *gw = malloc(sizeof(g_window_t));
    if (gw == NULL) {
        g_layer_del(ly);
        return -1;
    }
    memset(gw->title, 0, GW_TITLE_LEN);
    strcpy(gw->title, title);
    gw->layer = ly;
    gw->flags = 0;
    gw->x = x;
    gw->y = y;
    gw->width = width;
    gw->height = height;
    
    list_add_tail(&gw->wlist, &gwindow_list_head);

    g_layer_set_region(ly, LAYER_REGION_DRAG, 0, 0, width, 24);
    g_layer_set_region(ly, LAYER_REGION_RESIZE, 4, 4, width-4, height - 4);
    g_layer_set_flags(ly, LAYER_WINDOW);
    gw->backup.y = gw->backup.x = 0;
    gw->backup.width = gw->backup.height = 0;

    g_paint_window(ly, width, height, 1);
    return ly;
}

/**
 * 收到调整窗口大小的消息时需要重新绘制整个窗口
 */
int g_resize_window(int win, uint32_t width, uint32_t height)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    if (!width || !height)
        return -1;
    gw->width = width;
    gw->height = height;
    g_layer_set_region(gw->layer, LAYER_REGION_DRAG, 0, 0, width, 24);
    g_layer_set_region(gw->layer, LAYER_REGION_RESIZE, 4, 4, width-4, height - 4);
    
    g_paint_window(gw->layer, width, height, 1);
    /* TODO: 重绘内容 */

    g_layer_refresh(gw->layer, 0, 0, width, height);
    return 0;
}

/**
 * 聚焦窗口
 * @turn: 聚焦开关，on为聚焦，off为丢焦
 */
int g_focus_window(int win, int turn)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    g_paint_window(gw->layer, gw->width, gw->height, turn);
    g_layer_refresh(gw->layer, 0, 0, gw->width, gw->height);    
    return 0;
}

/**
 * 根据窗口标识（图层id）删除一个窗口。
 * 
 */
int g_del_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    if (g_layer_del(gw->layer) < 0)
        return -1;
    list_del(&gw->wlist);
    free(gw);
    g_layer_focus_win_top(); /* 删除后需要聚焦顶层窗口 */
    return 0;
}

/**
 * 窗口第一次显示在桌面上
 */
int g_show_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    int wintop = g_layer_get_wintop();
    g_layer_z(gw->layer, wintop);
    return g_layer_focus_win_top(); /* 显示后需要聚焦顶层窗口 */
}

/**
 * 隐藏指定窗口，当点击最小化时会执行这个操作。
*/
int g_hide_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    g_layer_z(gw->layer, -1);
    g_layer_focus_win_top(); /* 隐藏后需要聚焦顶层窗口 */
    return 0;
}

/**
 * 更新整个窗口，包括窗口样式
*/
int g_update_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    return g_layer_refresh(gw->layer, 0, 0, gw->width, gw->height);
}

/**
 * 窗口执行最大化操作。
 * 如果不是最大化，那么进行最大化。
 * 如果已经是最大化，那么就恢复之前窗口的大小和位置。
 */
int g_maxim_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    g_rect_t rect;
    if (gw->flags & GW_MAXIM) {
        /* 处于最大化状态，恢复原来窗口大小 */
        rect.x = gw->backup.x;
        rect.y = gw->backup.y;
        rect.width = gw->backup.width;
        rect.height = gw->backup.height;
        gw->flags &= ~GW_MAXIM;  /* 取消最大化状态 */
    } else {    /* 进行最大化 */
        gw->backup.x = gw->x;
        gw->backup.y = gw->y;
        gw->backup.width = gw->width;
        gw->backup.height = gw->height;
        
        rect.x = 0;
        rect.y = TASKBAR_HEIGHT;
        rect.width = desktop_width;
        rect.height = desktop_height - TASKBAR_HEIGHT;
        gw->flags |= GW_MAXIM;  /* 最大化状态 */
    }
    if (g_layer_resize(gw->layer, rect.x, rect.y, rect.width, rect.height) < 0)
        return -1;
    if (g_layer_focus(gw->layer) < 0) {
        /* TODO: 恢复原来的大小 */
        return -1;
    }
    return 0;
}
