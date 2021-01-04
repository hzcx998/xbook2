#ifndef _LIB_VIEW_BITMAP_H
#define _LIB_VIEW_BITMAP_H

#include <stdint.h>

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int *bits;
} viewbmp_t;

typedef struct {
    int x;      // 视图x
    int y;      // 视图y
    int bx;     // 位图x
    int by;     // 位图y
    int bw;     // 位图宽度
    int bh;     // 位图高度
    viewbmp_t bmp;
    char refresh;
} viewio_t;

void viewbmp_putpixel(viewbmp_t *bmp, int x, int y, uint32_t color)
{
    if (!bmp)
        return;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return;
    bmp->bits[y * bmp->width + x] = color;
}

void viewbmp_rectfill(viewbmp_t *bmp, int w, int h, uint32_t color)
{
    if (!bmp)
        return;
    int x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            viewbmp_putpixel(bmp, x, y, color);
        }
    }
}

#endif  /* _LIB_VIEW_BITMAP_H */