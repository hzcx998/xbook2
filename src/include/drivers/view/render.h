#ifndef _XBOOK_DRIVERS_VIEW_VRENDER_H
#define _XBOOK_DRIVERS_VIEW_VRENDER_H

#include <stdint.h>
#include "drivers/view/view.h"
#include <drivers/view/bitmap.h>

#define view_render_putpixel_fast(view, x, y, color) \
    do { \
        view_color_t *_vbuf = (view_color_t *)(view)->section->addr; \
        _vbuf[(y) * (view)->width + (x)] = color; \
    } while (0)

int view_render_putpixel(view_t *view, int x, int y, view_color_t color);
int view_render_getpixel(view_t *view, int x, int y, view_color_t *color);
void view_render_clear(view_t *view);
void view_render_vline(view_t *view, int x, int y1, int y2, view_color_t color);
void view_render_hline(view_t *view, int x1, int y, int x2, view_color_t color);
void view_render_line(view_t *view, int x1, int y1, int x2, int y2, view_color_t color);
void view_render_rect_ex(view_t *view, int x1, int y1, int x2, int y2, view_color_t color);
void view_render_rectfill_ex(view_t *view, int x1, int y1, int x2, int y2, view_color_t color);
void view_render_rect(view_t *view, int x, int y, uint32_t width, uint32_t height, view_color_t color);
void view_render_rectfill(view_t *view, int x, int y, uint32_t width, uint32_t height, view_color_t color);
#define view_render_viewfill(view, color) \
        view_render_rectfill(view, 0, 0, (view)->width, (view)->height, color)
void view_render_bitblt(view_t *view, int x, int y, 
        view_bitmap_t *bmp, int bx, int by, int width, int height);
void view_render_draw_shade(view_t *shade, view_rect_t *rect, int draw);

#endif /* _XBOOK_DRIVERS_VIEW_VRENDER_H */