#ifndef _LIB_XTK_COLOR_H
#define _LIB_XTK_COLOR_H

#include <stdint.h>

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} xtk_argb_t;

#define __XTK_ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define XTK_ARGB(a, r, g, b)     __XTK_ARGB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define XTK_RGB(r, g, b)         XTK_ARGB(255, r, g, b)

/* 常用颜色 */
#define XTK_RED        XTK_RGB(255, 0, 0)
#define XTK_GREEN      XTK_RGB(0, 255, 0)
#define XTK_BLUE       XTK_RGB(0, 0, 255)
#define XTK_WHITE      XTK_RGB(255, 255, 255)
#define XTK_BLACK      XTK_RGB(0, 0, 0)
#define XTK_GRAY       XTK_RGB(195, 195, 195)
#define XTK_LEAD       XTK_RGB(127, 127, 127)
#define XTK_YELLOW     XTK_RGB(255, 255, 0)
#define XTK_NONE_COLOR XTK_ARGB(0, 0, 0, 0)

#endif /* _LIB_XTK_COLOR_H */