#ifndef _XGUI_MISC_H
#define _XGUI_MISC_H

#include "xgui_color.h"

typedef struct {
    int x;
    int y;
    unsigned int w;
    unsigned int h;
} xgui_region_t;

typedef struct {
    xgui_region_t region;   // 位图中某个区域
    unsigned int width;     // 位图的宽度
    unsigned int height;    // 位图的高度
    xgui_color_t *colors;   // 位图颜色
} xgui_bitmap_t;

#endif /* _XGUI_MISC_H */