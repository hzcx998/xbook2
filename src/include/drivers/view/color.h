#ifndef _XBOOK_DRIVERS_VIEW_COLOR_H
#define _XBOOK_DRIVERS_VIEW_COLOR_H

#include <stdint.h>

typedef unsigned int view_color_t;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} view_argb_t;

#define VIEW_ARGB_SUB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define VIEW_ARGB(a, r, g, b)     VIEW_ARGB_SUB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define VIEW_RGB(r, g, b)         VIEW_ARGB(255, r, g, b)

/* 常用颜色 */
#define VIEW_RED        VIEW_RGB(255, 0, 0)
#define VIEW_GREEN      VIEW_RGB(0, 255, 0)
#define VIEW_BLUE       VIEW_RGB(0, 0, 255)
#define VIEW_WHITE      VIEW_RGB(255, 255, 255)
#define VIEW_BLACK      VIEW_RGB(0, 0, 0)
#define VIEW_GRAY       VIEW_RGB(195, 195, 195)
#define VIEW_LEAD       VIEW_RGB(127, 127, 127)
#define VIEW_YELLOW     VIEW_RGB(255, 255, 0)
#define VIEW_NONE       VIEW_ARGB(0, 0, 0, 0)

#endif /* _XBOOK_DRIVERS_VIEW_COLOR_H */