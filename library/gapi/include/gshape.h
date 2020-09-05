#ifndef _GAPI_SHAPE_H
#define _GAPI_SHAPE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int x, y;
} g_point_t;

typedef struct {
    int x, y;
    unsigned int width, height;
} g_rect_t;

typedef struct {
    int x0, y0;
    int x1, y1;
} g_line_t;

typedef struct {
    g_rect_t rect;
    unsigned int *pixles;
    int bps;
} g_pixmap_t;

typedef struct {
    int left, top;
    int right, bottom;
} g_region_t;

static inline void g_region_init(g_region_t *rg)
{
    rg->left = -1;
    rg->top = -1;
    rg->right = -1;
    rg->bottom = -1;
}

/**
 * check region whether valid
 */
static inline int g_region_valid(g_region_t *rg)
{
    if (rg->left != -1 && rg->right != -1 &&
        rg->top != -1 && rg->bottom != -1)
        return 1;
    return 0;
}

#define g_region_in(rg, x, y) \
        ((rg)->left <= (x) && (x) < (rg)->right && \
        (rg)->top <= (y) && (y) < (rg)->bottom)

static inline void g_rect_init(g_rect_t *rect)
{
    rect->x = 0;
    rect->y = 0;
    rect->width = 0;
    rect->height = 0;
}

/**
 * check rect whether valid
 */
static inline int g_rect_valid(g_rect_t *rect)
{
    if (rect->width > 0 && rect->height > 0)
        return 1;
    return 0;
} 

#define g_rect_in(rect, _x, _y) \
        ((rect)->x <= (_x) && (_x) < ((rect)->x + (rect)->width) && \
        (rect)->y <= (_y) && (_y) < ((rect)->y + (rect)->height))
        

static inline void g_rect_set(g_rect_t *rect, int x, int y, uint32_t width, uint32_t height)
{
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
}

static inline void g_rect_merge(g_rect_t *rect1, g_rect_t *rect2)
{
    rect1->x = min(rect1->x, rect2->x);
    rect1->y = min(rect1->y, rect2->y);
    rect1->width = max(rect1->width, rect2->width);
    rect1->height = max(rect1->height, rect2->height);
}

#endif /* _GAPI_SHAPE_H */