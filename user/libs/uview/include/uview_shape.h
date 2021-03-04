#ifndef _LIB_UVIEW_SHAPE_H
#define _LIB_UVIEW_SHAPE_H

#include <stdint.h>

typedef struct {
    int x;
    int y;
} uview_point_t;

typedef struct {
    int x;
    int y;
    union {
        unsigned int uw;
        int sw;
    } w;
    union {
        unsigned int uh;
        int sh;
    } h;
} uview_rect_t;

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} uview_region_t;

static inline void uview_region_reset(uview_region_t *region)
{
    region->left = -1;
    region->top = -1;
    region->right = -1;
    region->bottom = -1;
}

static inline void uview_region_init(uview_region_t *region, int left, int top, int right, int bottom)
{
    region->left = left;
    region->top = top;
    region->right = right;
    region->bottom = bottom;
}

static inline int uview_region_valid(uview_region_t *region)
{
    if (region->left != -1 && region->right != -1 &&
        region->top != -1 && region->bottom != -1)
        return 1;
    return 0;
}

#define uview_region_in_range(region, x, y) \
        ((region)->left <= (x) && (x) < (region)->right && \
        (region)->top <= (y) && (y) < (region)->bottom)


static inline void uview_rect_reset(uview_rect_t *rect)
{
    rect->x = 0;
    rect->y = 0;
    rect->w.uw = 0;
    rect->h.uh = 0;
}

static inline int uview_rect_valid(uview_rect_t *rect)
{
    if (rect->w.uw > 0 && rect->h.uh > 0)
        return 1;
    return 0;
}

#endif  /* _LIB_UVIEW_SHAPE_H */