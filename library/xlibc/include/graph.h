
#ifndef _XLIBC_GRAPH_H
#define _XLIBC_GRAPH_H

#include <stdint.h>

#define GC_NO_ALPHA   255

#define _GC_ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) 
#define GC_ARGB(a, r, g, b) _GC_ARGB((a) & 0xff, (r)  & 0xff, (g) & 0xff, (b) & 0xff)
#define GC_RGB(r, g, b) GC_ARGB(GC_NO_ALPHA, r, g, b)

/* 常用颜色 */
#define GC_RED        GC_RGB(255, 0, 0)
#define GC_GREEN      GC_RGB(0, 255, 0)
#define GC_BLUE       GC_RGB(0, 0, 255)
#define GC_WHITE      GC_RGB(255, 255, 255)
#define GC_BLACK      GC_RGB(0, 0, 0)
#define GC_GRAY       GC_RGB(195, 195, 195)
#define GC_LEAD       GC_RGB(127, 127, 127)
#define GC_YELLOW     GC_RGB(255, 255, 0)
#define GC_NONE       GC_ARGB(0, 0, 0, 0)

int g_layer_new(int x, int y, uint32_t width, uint32_t height);
int g_layer_del(int layer);
int g_layer_move(int layer, int x, int y);
int g_layer_z(int layer, int z);

int g_layer_outp(int layer, int x, int y, uint32_t color);
int g_layer_inp(int layer, int x, int y, uint32_t *color);
int g_layer_line(int layer, int x0, int y0, int x1, int y1, uint32_t color);
int g_layer_rect(int layer, int x, int y, int width, int height, uint32_t color);
int g_layer_rect_fill(int layer, int x, int y, int width, int height, uint32_t color);
int g_layer_pixmap(int layer, int x, int y, int width, int height, uint32_t *pixels, int bps);
int g_layer_refresh(int layer, int left, int top, int right, int bottom);
int g_gui_info(unsigned int *width, unsigned int *height, unsigned int *bpp);

#endif  /* _XLIBC_GRAPH_H */
