#ifndef _XGUI_GRAPH_H
#define _XGUI_GRAPH_H

#include <stdint.h>
#include "xgui_color.h"
#include "xgui_view.h"
#include "xgui_bitmap.h"

int xgui_graph_putpixel(xgui_view_t *view, int x, int y, xgui_color_t color);
int xgui_graph_getpixel(xgui_view_t *view, int x, int y, xgui_color_t *color);
void xgui_graph_clear(xgui_view_t *view);
void xgui_graph_vline(xgui_view_t *view, int x, int y1, int y2, xgui_color_t color);
void xgui_graph_hline(xgui_view_t *view, int x1, int y, int x2, xgui_color_t color);
void xgui_graph_line(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_graph_rect_ex(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_graph_rectfill_ex(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_graph_rect(xgui_view_t *view, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
void xgui_graph_rectfill(xgui_view_t *view, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
#define xgui_graph_viewfill(view, color) \
        xgui_graph_rectfill(view, 0, 0, (view)->width, (view)->height, color)
void xgui_graph_bitcopy(xgui_view_t *view, int x, int y, 
        xgui_bitmap_t *bmp, int bx, int by, int width, int height);

#endif /* _XGUI_GRAPH_H */