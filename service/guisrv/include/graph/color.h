#ifndef __GRAPH_COLOR_H__
#define __GRAPH_COLOR_H__

#include <stdint.h>

#define SCREEN_COLOR uint32_t
#define GUI_COLOR   uint32_t

#define COLOR_NO_ALPHA   255

#define __ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define COLOR_ARGB(a, r, g, b) __ARGB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define COLOR_RGB(r, g, b) COLOR_ARGB(COLOR_NO_ALPHA, r, g, b)

/* 常用颜色 */
#define COLOR_RED        COLOR_RGB(255, 0, 0)
#define COLOR_GREEN      COLOR_RGB(0, 255, 0)
#define COLOR_BLUE       COLOR_RGB(0, 0, 255)
#define COLOR_WHITE      COLOR_RGB(255, 255, 255)
#define COLOR_BLACK      COLOR_RGB(0, 0, 0)
#define COLOR_GRAY       COLOR_RGB(195, 195, 195)
#define COLOR_LEAD       COLOR_RGB(127, 127, 127)
#define COLOR_YELLOW     COLOR_RGB(255, 255, 0)
#define COLOR_NONE       COLOR_ARGB(0, 0, 0, 0)

#endif  /* __GRAPH_COLOR_H__ */
