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

#endif  /* _GUI_SHAPE_H */
