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

#endif /* _GAPI_SHAPE_H */