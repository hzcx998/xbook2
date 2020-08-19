#include "desktop.h"
#include "gwindow.h"
#include <graph.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

LIST_HEAD(gwindow_list_head);

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
/*
#define GW_BACK_COLOR  GC_RGB(245, 245, 245)
#define GW_BOARD_COLOR GC_RGB(200, 200, 200)
#define GW_FRONT_COLOR GC_RGB(230, 230, 230)
*/
#define GW_BACK_COLOR  GC_RGB(235, 235, 235)
#define GW_BOARD_COLOR GC_RGB(190, 190, 190)
#define GW_FRONT_COLOR GC_RGB(210, 210, 210)

static void g_paint_window(int ly, uint32_t width, uint32_t height)
{
    g_layer_rect_fill(ly, 0, 0, width, height, GW_BACK_COLOR);
    g_layer_rect(ly, 0, 0, width, height, GW_BOARD_COLOR);
    g_layer_line(ly, 0, 24, width, 24, GW_BOARD_COLOR);

    g_layer_rect_fill(ly, 16, 4, 16, 16, GW_FRONT_COLOR);
    g_layer_rect_fill(ly, 16 + 16 * 1 + 8 * 1, 4, 16, 16, GW_FRONT_COLOR);
    g_layer_rect_fill(ly, 16 + 16 * 2 + 8 * 2, 4, 16, 16, GW_FRONT_COLOR);

}

int g_new_window(char *title, int x, int y, uint32_t width, uint32_t height, uint32_t flags)
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
    gw->flags = flags;
    gw->width = width;
    gw->height = height;
    
    list_add_tail(&gw->wlist, &gwindow_list_head);
    g_paint_window(ly, width, height);
    return ly;
}

int g_del_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    if (g_layer_del(gw->layer) < 0)
        return -1;
    list_del(&gw->wlist);
    free(gw);
    return 0;
}


int g_show_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    int wintop = g_layer_get_wintop();
    return g_layer_z(gw->layer, wintop);
}

int g_hide_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    return g_layer_z(gw->layer, -1);
}

int g_update_window(int win)
{
    g_window_t *gw = g_find_window(win);
    if (!gw)
        return -1;
    return g_layer_refresh(gw->layer, 0, 0, gw->width, gw->height);
}
