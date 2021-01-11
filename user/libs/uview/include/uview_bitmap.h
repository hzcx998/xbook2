#ifndef _LIB_UVIEW_BITMAP_H
#define _LIB_UVIEW_BITMAP_H

#include <stdint.h>

#include "uview_color.h"

typedef struct {
    unsigned int width;
    unsigned int height;
    uview_color_t *bits;
} uview_bitmap_t;

typedef struct {
    int x;      // 视图x
    int y;      // 视图y
    int bx;     // 位图x
    int by;     // 位图y
    int bw;     // 位图宽度
    int bh;     // 位图高度
    uview_bitmap_t bmp;
    char refresh;
} uview_io_t;

uview_bitmap_t *uview_bitmap_create(unsigned int width, unsigned int height);
int uview_bitmap_destroy(uview_bitmap_t *bitmap);

static inline void uview_bitmap_init(uview_bitmap_t *vbmp, unsigned int width, unsigned int height, uview_color_t *bits)
{
    vbmp->width = width;
    vbmp->height = height;
    vbmp->bits = bits;
}
void uview_bitmap_putpixel(uview_bitmap_t *bmp, int x, int y, uview_color_t color);
int uview_bitmap_getpixel(uview_bitmap_t *bmp, int x, int y, uview_color_t *color);

static inline void uview_bitmap_putpixel_unsafe(uview_bitmap_t *bmp, int x, int y, uview_color_t color)
{
    bmp->bits[y * bmp->width + x] = color;
}

static inline void uview_bitmap_getpixel_unsafe(uview_bitmap_t *bmp, int x, int y, uview_color_t *color)
{
    *color = bmp->bits[y * bmp->width + x];
}

void uview_bitmap_vline(uview_bitmap_t *bmp, int x, int y1, int y2, uview_color_t color);
void uview_bitmap_hline(uview_bitmap_t *bmp, int x1, int y, int x2, uview_color_t color);
void uview_bitmap_line(uview_bitmap_t *bmp, int x1, int y1, int x2, int y2, uview_color_t color);
void uview_bitmap_rect_ex(uview_bitmap_t *bmp, int x1, int y1, int x2, int y2, uview_color_t color);
void uview_bitmap_rectfill_ex(uview_bitmap_t *bmp, int x1, int y1, int x2, int y2, uview_color_t color);
void uview_bitmap_rect(uview_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, uview_color_t color);
void uview_bitmap_rectfill(uview_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, uview_color_t color);
void uview_bitmap_clear(uview_bitmap_t *bmp);

#endif  /* _LIB_UVIEW_BITMAP_H */