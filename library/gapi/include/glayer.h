#ifndef _GAPI_LAYER_H
#define _GAPI_LAYER_H

#include <stdint.h>

enum {
    LAYER_REGION_DRAG = 1,
    LAYER_REGION_RESIZE,
};

/* 图层标志 */
enum {
    LAYER_WINDOW    = (1 << 0),   /* 窗口标志 */
    LAYER_FLOAT     = (1 << 1),   /* 浮动块标志 */
    LAYER_FIXED     = (1 << 2),   /* 固定位置标志 */
};

int g_layer_new(int x, int y, uint32_t width, uint32_t height);
int g_layer_del(int layer);
int g_layer_move(int layer, int x, int y);
int g_layer_z(int layer, int z);

int g_layer_outp(int layer, int x, int y, uint32_t color);
int g_layer_inp(int layer, int x, int y, uint32_t *color);
int g_layer_line(int layer, int x0, int y0, int x1, int y1, uint32_t color);
int g_layer_rect(int layer, int x, int y, int width, int height, uint32_t color);
int g_layer_rect_fill(int layer, int x, int y, int width, int height, uint32_t color);
int g_layer_pixmap(int layer, int x, int y, int width, int height, uint32_t *pixels, int bps);
int g_layer_refresh(int layer, int left, int top, int right, int bottom);

int g_layer_get_wintop();
int g_layer_set_wintop(int top);

int g_layer_get_focus();
int g_layer_set_focus(int ly);

int g_layer_set_region(int layer, int type, int left, int top, int right, int bottom);

int g_layer_set_flags(int layer, uint32_t flags);

int g_layer_focus(int ly);
int g_layer_resize(int ly, int x, int y, uint32_t width, uint32_t height);

int g_layer_focus_win_top();

#endif /* _GAPI_LAYER_H */