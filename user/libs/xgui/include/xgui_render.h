#ifndef _XGUI_RENDER_H
#define _XGUI_RENDER_H

#include <xgui_misc.h>
#include <xgui_bitmap.h>
#include <xgui_dotfont.h>

int xgui_render_bitblt(int handle, int x, int y, 
        xgui_bitmap_t *bmp, xgui_rect_t *rect);
int xgui_render_bitblt_reverse(int handle, int x, int y, 
        xgui_bitmap_t *bmp, xgui_rect_t *rect);
int xgui_render_putpixel(int handle, int x, int y, xgui_color_t color);
int xgui_render_getpixel(int handle, int x, int y, xgui_color_t *color);
void xgui_render_clear(int handle);
void xgui_render_line(int handle, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_render_rect_ex(int handle, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_render_rectfill_ex(int handle, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_render_rect(int handle, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
void xgui_render_rectfill(int handle, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
void xgui_render_char(
        int handle, 
        int x,
        int y,
        char ch,
        xgui_color_t color);
void xgui_render_text(
    int handle, 
    int x,
    int y,
    char *text,
    xgui_color_t color);

#endif /* _XGUI_RENDER_H */