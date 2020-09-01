#ifndef _GAPI_PIXMAP_H
#define _GAPI_PIXMAP_H

#include <stdint.h>
#include <sys/list.h>
#include "gcolor.h"
#include "gfont.h"

typedef struct {
    list_t list;
    unsigned int width;
    unsigned int height;
    g_color_t *buffer;
} g_bitmap_t;

g_bitmap_t *g_new_bitmap(unsigned int width, unsigned int height);
int g_del_bitmap(g_bitmap_t *bitmap);
int g_del_bitmap_all();

void g_putpixel(g_bitmap_t *bmp, int x, int y, g_color_t color);
int g_getpixel(g_bitmap_t *bmp, int x, int y, g_color_t *color);

static inline void _g_putpixel(g_bitmap_t *bmp, int x, int y, g_color_t color)
{
    bmp->buffer[y * bmp->width + x] = color;
}

static inline void _g_getpixel(g_bitmap_t *bmp, int x, int y, g_color_t *color)
{
    *color = bmp->buffer[y * bmp->width + x];
}

void g_vline(g_bitmap_t *bmp, int x, int y1, int y2, g_color_t color);
void g_hline(g_bitmap_t *bmp, int x1, int y, int x2, g_color_t color);
void g_line(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color);

void g_rect_ex(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color);
void g_rectfill_ex(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color);

void g_rect(g_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, g_color_t color);
void g_rectfill(g_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, g_color_t color);

void g_char(
    g_bitmap_t *bmp, 
    int x,
    int y,
    char ch,
    g_color_t color);

void g_char_ex(
    g_bitmap_t *bmp, 
    int x,
    int y,
    char ch,
    g_color_t color,
    g_font_t *font);

void g_bitmap_clear(g_bitmap_t *bmp);

#endif /* _GAPI_PIXMAP_H */