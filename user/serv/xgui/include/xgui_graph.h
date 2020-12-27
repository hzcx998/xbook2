#ifndef _XGUI_GRAPH_H
#define _XGUI_GRAPH_H

#include <stdint.h>
#include "xgui_view.h"
#include <xgui_bitmap.h>

int server_xgui_render_putpixel(xgui_view_t *view, int x, int y, xgui_color_t color);
int server_xgui_render_getpixel(xgui_view_t *view, int x, int y, xgui_color_t *color);
void server_xgui_render_clear(xgui_view_t *view);
void server_xgui_render_vline(xgui_view_t *view, int x, int y1, int y2, xgui_color_t color);
void server_xgui_render_hline(xgui_view_t *view, int x1, int y, int x2, xgui_color_t color);
void server_xgui_render_line(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void server_xgui_render_rect_ex(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void server_xgui_render_rectfill_ex(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void server_xgui_render_rect(xgui_view_t *view, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
void server_xgui_render_rectfill(xgui_view_t *view, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
#define server_xgui_render_viewfill(view, color) \
        server_xgui_render_rectfill(view, 0, 0, (view)->width, (view)->height, color)
void server_xgui_render_bitblt(xgui_view_t *view, int x, int y, 
        xgui_bitmap_t *bmp, int bx, int by, int width, int height);
void server_xgui_render_char(
        xgui_view_t *view, 
        int x,
        int y,
        char ch,
        xgui_color_t color);
        
void server_xgui_render_text(
    xgui_view_t *view, 
    int x,
    int y,
    char *text,
    xgui_color_t color);
    
#endif /* _XGUI_GRAPH_H */