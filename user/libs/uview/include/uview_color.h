#ifndef _LIB_UVIEW_COLOR_H
#define _LIB_UVIEW_COLOR_H

#include <stdint.h>

typedef unsigned int uview_color_t;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} uview_argb_t;

#define __UVIEW_ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define UVIEW_ARGB(a, r, g, b)     __UVIEW_ARGB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define UVIEW_RGB(r, g, b)         UVIEW_ARGB(255, r, g, b)

/* 常用颜色 */
#define UVIEW_RED        UVIEW_RGB(255, 0, 0)
#define UVIEW_GREEN      UVIEW_RGB(0, 255, 0)
#define UVIEW_BLUE       UVIEW_RGB(0, 0, 255)
#define UVIEW_WHITE      UVIEW_RGB(255, 255, 255)
#define UVIEW_BLACK      UVIEW_RGB(0, 0, 0)
#define UVIEW_GRAY       UVIEW_RGB(195, 195, 195)
#define UVIEW_LEAD       UVIEW_RGB(127, 127, 127)
#define UVIEW_YELLOW     UVIEW_RGB(255, 255, 0)
#define UVIEW_NONE_COLOR UVIEW_ARGB(0, 0, 0, 0)

#endif  /* _LIB_UVIEW_COLOR_H */