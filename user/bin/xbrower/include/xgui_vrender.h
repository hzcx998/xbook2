#ifndef _XGUI_VRENDER_H
#define _XGUI_VRENDER_H

#include <stdint.h>
#include "xgui_view.h"
#include <xgui_bitmap.h>

int xgui_vrender_putpixel(xgui_view_t *view, int x, int y, xgui_color_t color);
int xgui_vrender_getpixel(xgui_view_t *view, int x, int y, xgui_color_t *color);
void xgui_vrender_clear(xgui_view_t *view);
void xgui_vrender_vline(xgui_view_t *view, int x, int y1, int y2, xgui_color_t color);
void xgui_vrender_hline(xgui_view_t *view, int x1, int y, int x2, xgui_color_t color);
void xgui_vrender_line(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_vrender_rect_ex(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_vrender_rectfill_ex(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_vrender_rect(xgui_view_t *view, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
void xgui_vrender_rectfill(xgui_view_t *view, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
#define xgui_vrender_viewfill(view, color) \
        xgui_vrender_rectfill(view, 0, 0, (view)->width, (view)->height, color)
void xgui_vrender_bitblt(xgui_view_t *view, int x, int y, 
        xgui_bitmap_t *bmp, int bx, int by, int width, int height);
void xgui_vrender_char(
        xgui_view_t *view, 
        int x,
        int y,
        char ch,
        xgui_color_t color);
        
void xgui_vrender_text(
    xgui_view_t *view,  
    int x,
    int y,
    char *text,
    xgui_color_t color);
    
#endif /* _XGUI_VRENDER_H */