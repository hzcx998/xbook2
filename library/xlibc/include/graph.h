
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


/* GUI message */
enum {
    GM_NONE = 0,
    GM_QUIT,
    GM_KEY_DOWN,
    GM_KEY_UP,
    GM_MOUSE_MOTION,
    GM_MOUSE_LBTN_DOWN,
    GM_MOUSE_LBTN_UP,
    GM_MOUSE_LBTN_DBLCLK,
    GM_MOUSE_RBTN_DOWN,
    GM_MOUSE_RBTN_UP,
    GM_MOUSE_RBTN_DBLCLK,
    GM_MOUSE_MBTN_DOWN,
    GM_MOUSE_MBTN_UP,
    GM_MOUSE_MBTN_DBLCLK,
    GM_MOUSE_WHEEL,
    GM_COUNTER,
    GM_TIMER,
    GM_NR
};

typedef struct {
    uint32_t id;        /* 消息id */
    int target;         /* 目标句柄 */
    uint32_t data0;     
    uint32_t data1;     
    uint32_t data2;     
    uint32_t data3;
} g_msg_t;

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

int g_layer_get_wintop();
int g_layer_set_wintop(int top);

int g_quit(void);
int g_init(void);
int g_set_routine(int (*routine)(g_msg_t *));
int g_get_msg(g_msg_t *msg);
int g_dispatch_msg(g_msg_t *msg);
int g_try_get_msg(g_msg_t *msg);

int g_layer_get_focus();
int g_layer_set_focus(int ly);

#endif  /* _XLIBC_GRAPH_H */
