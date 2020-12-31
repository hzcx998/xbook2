#ifndef _XGUI_BITMAP_H
#define _XGUI_BITMAP_H

/* 属于客户端API部分 */

#include <xgui_color.h>

typedef struct {
    unsigned int width;
    unsigned int height;
    xgui_color_t *bits;
} xgui_bitmap_t;

static inline void xgui_bitmap_init(xgui_bitmap_t *bmp, unsigned int width, unsigned int height, xgui_color_t *bits)
{
    bmp->width = width;
    bmp->height = height;
    bmp->bits = bits;
}

xgui_bitmap_t *xgui_bitmap_create(unsigned int width, unsigned int height);
int xgui_bitmap_destroy(xgui_bitmap_t *bitmap);

void xgui_bitmap_putpixel(xgui_bitmap_t *bmp, int x, int y, xgui_color_t color);
int xgui_bitmap_getpixel(xgui_bitmap_t *bmp, int x, int y, xgui_color_t *color);

static inline void xgui_bitmap_putpixel_unsafe(xgui_bitmap_t *bmp, int x, int y, xgui_color_t color)
{
    bmp->bits[y * bmp->width + x] = color;
}

static inline void xgui_bitmap_getpixel_unsafe(xgui_bitmap_t *bmp, int x, int y, xgui_color_t *color)
{
    *color = bmp->bits[y * bmp->width + x];
}

void xgui_bitmap_vline(xgui_bitmap_t *bmp, int x, int y1, int y2, xgui_color_t color);
void xgui_bitmap_hline(xgui_bitmap_t *bmp, int x1, int y, int x2, xgui_color_t color);
void xgui_bitmap_line(xgui_bitmap_t *bmp, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_bitmap_rect_ex(xgui_bitmap_t *bmp, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_bitmap_rectfill_ex(xgui_bitmap_t *bmp, int x1, int y1, int x2, int y2, xgui_color_t color);
void xgui_bitmap_rect(xgui_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
void xgui_bitmap_rectfill(xgui_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, xgui_color_t color);
void xgui_bitmap_clear(xgui_bitmap_t *bmp);

void xgui_bitmap_char(
        xgui_bitmap_t *bmp, 
        int x,
        int y,
        char ch,
        xgui_color_t color);

void xgui_bitmap_text(
    xgui_bitmap_t *bmp, 
    int x,
    int y,
    char *text,
    xgui_color_t color);

#endif /* _XGUI_BITMAP_H */