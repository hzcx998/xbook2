#ifndef _XGUI_COLOR_H
#define _XGUI_COLOR_H

#include <stdint.h>

typedef unsigned int xgui_color_t;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} xgui_argb_t;

#define XGUI_ARGB_SUB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define XCOLOR_ARGB(a, r, g, b)     XGUI_ARGB_SUB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define XCOLOR_RGB(r, g, b)         XCOLOR_ARGB(255, r, g, b)

/* 常用颜色 */
#define XCOLOR_RED        XCOLOR_RGB(255, 0, 0)
#define XCOLOR_GREEN      XCOLOR_RGB(0, 255, 0)
#define XCOLOR_BLUE       XCOLOR_RGB(0, 0, 255)
#define XCOLOR_WHITE      XCOLOR_RGB(255, 255, 255)
#define XCOLOR_BLACK      XCOLOR_RGB(0, 0, 0)
#define XCOLOR_GRAY       XCOLOR_RGB(195, 195, 195)
#define XCOLOR_LEAD       XCOLOR_RGB(127, 127, 127)
#define XCOLOR_YELLOW     XCOLOR_RGB(255, 255, 0)
#define XCOLOR_NONE       XCOLOR_ARGB(0, 0, 0, 0)

#endif /* _XGUI_COLOR_H */