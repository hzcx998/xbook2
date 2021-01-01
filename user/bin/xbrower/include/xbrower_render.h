#ifndef _XGUI_VRENDER_H
#define _XGUI_VRENDER_H

#include <stdint.h>
#include "xbrower_view.h"
#include <xbrower_bitmap.h>

int xbrower_render_putpixel(xbrower_view_t *view, int x, int y, xbrower_color_t color);
int xbrower_render_getpixel(xbrower_view_t *view, int x, int y, xbrower_color_t *color);
void xbrower_render_clear(xbrower_view_t *view);
void xbrower_render_vline(xbrower_view_t *view, int x, int y1, int y2, xbrower_color_t color);
void xbrower_render_hline(xbrower_view_t *view, int x1, int y, int x2, xbrower_color_t color);
void xbrower_render_line(xbrower_view_t *view, int x1, int y1, int x2, int y2, xbrower_color_t color);
void xbrower_render_rect_ex(xbrower_view_t *view, int x1, int y1, int x2, int y2, xbrower_color_t color);
void xbrower_render_rectfill_ex(xbrower_view_t *view, int x1, int y1, int x2, int y2, xbrower_color_t color);
void xbrower_render_rect(xbrower_view_t *view, int x, int y, uint32_t width, uint32_t height, xbrower_color_t color);
void xbrower_render_rectfill(xbrower_view_t *view, int x, int y, uint32_t width, uint32_t height, xbrower_color_t color);
#define xbrower_render_viewfill(view, color) \
        xbrower_render_rectfill(view, 0, 0, (view)->width, (view)->height, color)
void xbrower_render_bitblt(xbrower_view_t *view, int x, int y, 
        xbrower_bitmap_t *bmp, int bx, int by, int width, int height);

#endif /* _XGUI_VRENDER_H */