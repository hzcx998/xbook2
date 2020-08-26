#ifndef _GAPI_SHAPE_H
#define _GAPI_SHAPE_H

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
        

#endif /* _GAPI_SHAPE_H */