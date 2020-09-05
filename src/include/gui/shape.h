#ifndef _GUI_SHAPE_H
#define _GUI_SHAPE_H

typedef struct {
    int x, y;
} gui_point_t;

typedef struct {
    int x, y;
    unsigned int width, height;
} gui_rect_t;

typedef struct {
    int x, y;
    int width, height;
} gui_rect_ex_t;

typedef struct {
    int x0, y0;
    int x1, y1;
} gui_line_t;

typedef struct {
    gui_rect_t rect;
    unsigned int *pixles;
    int bps;
} gui_pixmap_t;

typedef struct {
    int left, top;
    int right, bottom;
} gui_region_t;

static inline void gui_region_init(gui_region_t *rg)
{
    rg->left = -1;
    rg->top = -1;
    rg->right = -1;
    rg->bottom = -1;
}

/**
 * check region whether valid
 */
static inline int gui_region_valid(gui_region_t *rg)
{
    if (rg->left != -1 && rg->right != -1 &&
        rg->top != -1 && rg->bottom != -1)
        return 1;
    return 0;
}

#define gui_region_in(rg, x, y) \
        ((rg)->left <= (x) && (x) < (rg)->right && \
        (rg)->top <= (y) && (y) < (rg)->bottom)

static inline void gui_rect_init(gui_rect_t *rect)
{
    rect->x = 0;
    rect->y = 0;
    rect->width = 0;
    rect->height = 0;
}

/**
 * check rect whether valid
 */
static inline int gui_rect_valid(gui_rect_t *rect)
{
    if (rect->width > 0 && rect->height > 0)
        return 1;
    return 0;
} 

static inline void gui_rect_copy(gui_rect_t *rect0, gui_rect_t *rect1)
{
    *rect0 = *rect1;
} 
#endif  /* _GUI_SHAPE_H */
