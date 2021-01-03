#ifndef _XBOOK_DRIVERS_VIEW_MISC_H
#define _XBOOK_DRIVERS_VIEW_MISC_H

typedef struct {
    int x;
    int y;
} view_point_t;

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
} view_rect_t;

typedef struct {
    int left;
    int right;
    int top;
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

static inline void view_rect_reset(view_rect_t *rect)
{
    rect->x = 0;
    rect->y = 0;
    rect->w.uw = 0;
    rect->h.uh = 0;
}

static inline int view_rect_valid(view_rect_t *rect)
{
    if (rect->w.uw > 0 && rect->h.uh > 0)
        return 1;
    return 0;
}

static inline void view_rect_copy(view_rect_t *rect0, view_rect_t *rect1)
{
    *rect0 = *rect1;
}

#endif /* _XBOOK_DRIVERS_VIEW_MISC_H */