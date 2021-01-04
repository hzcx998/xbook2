#ifndef _LIB_VIEW_SHAPE_H
#define _LIB_VIEW_SHAPE_H

#include <stdint.h>

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} view_region_t;

static inline void view_region_reset(view_region_t *rect)
{
    rect->left = -1;
    rect->top = -1;
    rect->right = -1;
    rect->bottom = -1;
}

static inline void view_region_init(view_region_t *rect, int left, int top, int right, int bottom)
{
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

static inline int view_region_valid(view_region_t *rect)
{
    if (rect->left != -1 && rect->right != -1 &&
        rect->top != -1 && rect->bottom != -1)
        return 1;
    return 0;
}

#define view_region_in_range(rect, x, y) \
        ((rect)->left <= (x) && (x) < (rect)->right && \
        (rect)->top <= (y) && (y) < (rect)->bottom)

#endif  /* _LIB_VIEW_SHAPE_H */