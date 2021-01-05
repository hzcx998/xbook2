#ifndef _XBOOK_DRIVERS_VIEW_BITMAP_H
#define _XBOOK_DRIVERS_VIEW_BITMAP_H

/* 属于客户端API部分 */

#include <drivers/view/color.h>

typedef struct {
    unsigned int width;
    unsigned int height;
    view_color_t *bits;
} view_bitmap_t;

static inline void view_bitmap_init(view_bitmap_t *bmp, unsigned int width, unsigned int height, view_color_t *bits)
{
    bmp->width = width;
    bmp->height = height;
    bmp->bits = bits;
}

view_bitmap_t *view_bitmap_create(unsigned int width, unsigned int height);
int view_bitmap_destroy(view_bitmap_t *bitmap);

void view_bitmap_putpixel(view_bitmap_t *bmp, int x, int y, view_color_t color);
int view_bitmap_getpixel(view_bitmap_t *bmp, int x, int y, view_color_t *color);

static inline void view_bitmap_putpixel_unsafe(view_bitmap_t *bmp, int x, int y, view_color_t color)
{
    bmp->bits[y * bmp->width + x] = color;
}

static inline void view_bitmap_getpixel_unsafe(view_bitmap_t *bmp, int x, int y, view_color_t *color)
{
    *color = bmp->bits[y * bmp->width + x];
}

void view_bitmap_vline(view_bitmap_t *bmp, int x, int y1, int y2, view_color_t color);
void view_bitmap_hline(view_bitmap_t *bmp, int x1, int y, int x2, view_color_t color);
void view_bitmap_line(view_bitmap_t *bmp, int x1, int y1, int x2, int y2, view_color_t color);
void view_bitmap_rect_ex(view_bitmap_t *bmp, int x1, int y1, int x2, int y2, view_color_t color);
void view_bitmap_rectfill_ex(view_bitmap_t *bmp, int x1, int y1, int x2, int y2, view_color_t color);
void view_bitmap_rect(view_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, view_color_t color);
void view_bitmap_rectfill(view_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, view_color_t color);
void view_bitmap_clear(view_bitmap_t *bmp);

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int *bits;
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

#endif /* _XBOOK_DRIVERS_VIEW_BITMAP_H */