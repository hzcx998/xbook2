#ifndef _LIB_XTK_SURFACE_H
#define _LIB_XTK_SURFACE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int x, y;
    uint32_t w, h;
} xtk_rect_t;

static inline void xtk_rect_init(xtk_rect_t *rect, int x, int y, uint32_t w, uint32_t h)
{
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
}

static inline int xtk_rect_valid(xtk_rect_t *rect)
{
    if (rect->w <= 0 || rect->h <= 0)
        return 0;
    return 1;
}

static inline void xtk_rect_merge(xtk_rect_t *dst, xtk_rect_t *src)
{
    dst->x = min(dst->x, src->x);
    dst->y = min(dst->y, src->y);
    dst->w = max(dst->w, src->w);
    dst->h = max(dst->h, src->h);
}

typedef struct {
    uint32_t w, h;
    uint32_t *pixels;
} xtk_surface_t;

xtk_surface_t *xtk_surface_create(uint32_t width, uint32_t height);
int xtk_surface_destroy(xtk_surface_t *surface);

static inline void xtk_surface_init(xtk_surface_t *surface, uint32_t width, uint32_t height, uint32_t *pixels)
{
    surface->w = width;
    surface->h = height;
    surface->pixels = pixels;
}

void xtk_surface_putpixel(xtk_surface_t *surface, int x, int y, uint32_t color);
int xtk_surface_getpixel(xtk_surface_t *surface, int x, int y, uint32_t *color);

static inline void xtk_surface_putpixel_unsafe(xtk_surface_t *surface, int x, int y, uint32_t color)
{
    surface->pixels[y * surface->w + x] = color;
}

static inline void xtk_surface_getpixel_unsafe(xtk_surface_t *surface, int x, int y, uint32_t *color)
{
    *color = surface->pixels[y * surface->h + x];
}

void xtk_surface_vline(xtk_surface_t *surface, int x, int y1, int y2, uint32_t color);
void xtk_surface_hline(xtk_surface_t *surface, int x1, int y, int x2, uint32_t color);
void xtk_surface_line(xtk_surface_t *surface, int x1, int y1, int x2, int y2, uint32_t color);
void xtk_surface_rect_ex(xtk_surface_t *surface, int x1, int y1, int x2, int y2, uint32_t color);
void xtk_surface_rectfill_ex(xtk_surface_t *surface, int x1, int y1, int x2, int y2, uint32_t color);
void xtk_surface_rect(xtk_surface_t *surface, int x, int y, uint32_t width, uint32_t height, uint32_t color);
void xtk_surface_rectfill(xtk_surface_t *surface, int x, int y, uint32_t width, uint32_t height, uint32_t color);
void xtk_surface_clear(xtk_surface_t *surface);

void xtk_surface_blit(xtk_surface_t *src, xtk_rect_t *srcrect, xtk_surface_t *dst, xtk_rect_t *dstrect);

#endif /* _LIB_XTK_SURFACE_H */