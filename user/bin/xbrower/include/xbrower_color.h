#ifndef _XGUI_COLOR_H
#define _XGUI_COLOR_H

#include <stdint.h>

typedef unsigned int xbrower_color_t;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} xbrower_argb_t;

#define XGUI_ARGB_SUB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define XGUI_ARGB(a, r, g, b)     XGUI_ARGB_SUB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define XGUI_RGB(r, g, b)         XGUI_ARGB(255, r, g, b)

/* 常用颜色 */
#define XGUI_RED        XGUI_RGB(255, 0, 0)
#define XGUI_GREEN      XGUI_RGB(0, 255, 0)
#define XGUI_BLUE       XGUI_RGB(0, 0, 255)
#define XGUI_WHITE      XGUI_RGB(255, 255, 255)
#define XGUI_BLACK      XGUI_RGB(0, 0, 0)
#define XGUI_GRAY       XGUI_RGB(195, 195, 195)
#define XGUI_LEAD       XGUI_RGB(127, 127, 127)
#define XGUI_YELLOW     XGUI_RGB(255, 255, 0)
#define XGUI_NONE       XGUI_ARGB(0, 0, 0, 0)

#endif /* _XGUI_COLOR_H */