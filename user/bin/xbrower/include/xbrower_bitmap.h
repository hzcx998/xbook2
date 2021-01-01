#ifndef _XGUI_BITMAP_H
#define _XGUI_BITMAP_H

/* 属于客户端API部分 */

#include <xbrower_color.h>

typedef struct {
    unsigned int width;
    unsigned int height;
    xbrower_color_t *bits;
} xbrower_bitmap_t;

static inline void xbrower_bitmap_init(xbrower_bitmap_t *bmp, unsigned int width, unsigned int height, xbrower_color_t *bits)
{
    bmp->width = width;
    bmp->height = height;
    bmp->bits = bits;
}

xbrower_bitmap_t *xbrower_bitmap_create(unsigned int width, unsigned int height);
int xbrower_bitmap_destroy(xbrower_bitmap_t *bitmap);

void xbrower_bitmap_putpixel(xbrower_bitmap_t *bmp, int x, int y, xbrower_color_t color);
int xbrower_bitmap_getpixel(xbrower_bitmap_t *bmp, int x, int y, xbrower_color_t *color);

static inline void xbrower_bitmap_putpixel_unsafe(xbrower_bitmap_t *bmp, int x, int y, xbrower_color_t color)
{
    bmp->bits[y * bmp->width + x] = color;
}

static inline void xbrower_bitmap_getpixel_unsafe(xbrower_bitmap_t *bmp, int x, int y, xbrower_color_t *color)
{
    *color = bmp->bits[y * bmp->width + x];
}

void xbrower_bitmap_vline(xbrower_bitmap_t *bmp, int x, int y1, int y2, xbrower_color_t color);
void xbrower_bitmap_hline(xbrower_bitmap_t *bmp, int x1, int y, int x2, xbrower_color_t color);
void xbrower_bitmap_line(xbrower_bitmap_t *bmp, int x1, int y1, int x2, int y2, xbrower_color_t color);
void xbrower_bitmap_rect_ex(xbrower_bitmap_t *bmp, int x1, int y1, int x2, int y2, xbrower_color_t color);
void xbrower_bitmap_rectfill_ex(xbrower_bitmap_t *bmp, int x1, int y1, int x2, int y2, xbrower_color_t color);
void xbrower_bitmap_rect(xbrower_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, xbrower_color_t color);
void xbrower_bitmap_rectfill(xbrower_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, xbrower_color_t color);
void xbrower_bitmap_clear(xbrower_bitmap_t *bmp);

#endif /* _XGUI_BITMAP_H */