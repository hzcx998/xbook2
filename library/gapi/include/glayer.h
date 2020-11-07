#ifndef _GAPI_LAYER_H
#define _GAPI_LAYER_H

#include <stdint.h>
#include <gbitmap.h>
#include <gshape.h>

typedef int g_layer_t;

/* 单个进程能持有的图层数量 */
#define G_LAYER_NR  8

enum {
    LAYER_REGION_DRAG = 1,      /* drag occur region */
    LAYER_REGION_RESIZE,        /* resize occur region */
    LAYER_REGION_RESIZEMIN,     /* resize minim region */
};

/* 图层标志 */
enum {
    LAYER_WINDOW    = (1 << 0),   /* 窗口标志 */
    LAYER_FLOAT     = (1 << 1),   /* 浮动块标志 */
    LAYER_FIXED     = (1 << 2),   /* 固定位置标志 */
};

int g_new_layer(int x, int y, uint32_t width, uint32_t height);
int g_del_layer(int layer);

int g_move_layer(int layer, int x, int y);
int g_set_layer_z(int layer, int z);

int g_refresh_layer(int layer, int left, int top, int right, int bottom);

int g_paint_layer(int layer, int x, int y, g_bitmap_t *bmp);
int g_paint_layer_ex(int layer, int x, int y, g_bitmap_t *bmp);

#define g_refresh_layer_rect(l, x, y, w, h) g_refresh_layer((l), (x), (y), ((x) + (w)), ((y) + (h)))

int g_get_layer_wintop();
int g_set_layer_wintop(int top);

int g_get_layer_focus();
int g_set_layer_focus(int ly);

int g_set_layer_region(int layer, int type, int left, int top, int right, int bottom);

int g_set_layer_flags(int layer, uint32_t flags);

int g_focus_layer(int ly);
int g_resize_layer(int ly, int x, int y, uint32_t width, uint32_t height);

int g_focus_layer_win_top();

int g_get_layer_desktop();
int g_set_layer_desktop(int id);

int g_sync_layer_bitmap(
    int layer,
    g_rect_t *rect,
    g_color_t *bitmap,
    g_region_t *region);

int g_sync_layer_bitmap_ex(
    int layer,
    g_rect_t *rect,
    g_color_t *bitmap,
    g_region_t *region);

int g_copy_layer_bitmap(
    int layer,
    g_rect_t *rect,
    g_color_t *bitmap,
    g_region_t *region);

int g_get_icon_path(int layer, char *path, uint32_t len);
int g_set_icon_path(int layer, char *path);

#endif /* _GAPI_LAYER_H */