#ifndef _LIB_UVIEW_SHAPE_H
#define _LIB_UVIEW_SHAPE_H

#include <stdint.h>

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} uview_region_t;

static inline void uview_region_reset(uview_region_t *rect)
{
    rect->left = -1;
    rect->top = -1;
    rect->right = -1;
    rect->bottom = -1;
}

static inline void uview_region_init(uview_region_t *rect, int left, int top, int right, int bottom)
{
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

static inline int uview_region_valid(uview_region_t *rect)
{
    if (rect->left != -1 && rect->right != -1 &&
        rect->top != -1 && rect->bottom != -1)
        return 1;
    return 0;
}

#define uview_region_in_range(rect, x, y) \
        ((rect)->left <= (x) && (x) < (rect)->right && \
        (rect)->top <= (y) && (y) < (rect)->bottom)

#endif  /* _LIB_UVIEW_SHAPE_H */