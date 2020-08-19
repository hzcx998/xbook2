#include <sys/syscall.h>
#include <graph.h>

typedef struct {
    int x, y;
} gui_point_t;

typedef struct {
    int x, y;
    unsigned int width, height;
} gui_rect_t;

typedef struct {
    int x0, y0;
    int x1, y1;
} gui_line_t;

typedef struct {
    gui_rect_t rect;
    unsigned int *pixles;
    int bps;
} gui_pixmap_t;

typedef struct {
    int left, top;
    int right, bottom;
} gui_region_t;

int g_layer_new(int x, int y, uint32_t width, uint32_t height)
{
    return syscall4(int, SYS_LAYERNEW, x, y, width, height);
}

int g_layer_del(int layer)
{
    return syscall1(int, SYS_LAYERDEL, layer);
}

int g_layer_move(int layer, int x, int y)
{
    return syscall3(int, SYS_LAYERMOVE, layer, x, y);
}

int g_layer_z(int layer, int z)
{
    return syscall2(int, SYS_LAYERZ, layer, z);
}

int g_layer_outp(int layer, int x, int y, uint32_t color)
{
    gui_point_t p;
    p.x = x;
    p.y = y;
    return syscall3(int, SYS_LAYEROUTP, layer, &p, color);
}

int g_layer_inp(int layer, int x, int y, uint32_t *color)
{
    gui_point_t p;
    p.x = x;
    p.y = y;
    return syscall3(int, SYS_LAYERINP, layer, &p, color);
}

int g_layer_line(int layer, int x0, int y0, int x1, int y1, uint32_t color)
{
    gui_line_t p;
    p.x0 = x0;
    p.y0 = y0;
    p.x1 = x1;
    p.y1 = y1;
    return syscall3(int, SYS_LAYERLINE, layer, &p, color);
}

int g_layer_rect(int layer, int x, int y, int width, int height, uint32_t color)
{
    gui_rect_t p;
    p.x = x;
    p.y = y;
    p.width = width;
    p.height = height;
    return syscall3(int, SYS_LAYERRECT, layer, &p, color);
}

int g_layer_rect_fill(int layer, int x, int y, int width, int height, uint32_t color)
{
    gui_rect_t p;
    p.x = x;
    p.y = y;
    p.width = width;
    p.height = height;
    return syscall3(int, SYS_LAYERRECTFILL, layer, &p, color);
}

int g_layer_pixmap(int layer, int x, int y, int width, int height, uint32_t *pixels, int bps)
{
    gui_pixmap_t p;
    p.rect.x = x;
    p.rect.y = y;
    p.rect.width = width;
    p.rect.height = height;
    p.pixles = pixels;
    p.bps = bps;
    return syscall2(int, SYS_LAYERPIXMAP, layer, &p);
}

int g_layer_refresh(int layer, int left, int top, int right, int bottom)
{
    gui_region_t p;
    p.left = left;
    p.top = top;
    p.right = right;
    p.bottom = bottom;
    return syscall2(int, SYS_LAYERREFRESH, layer, &p);
}

int g_gui_info(unsigned int *width, unsigned int *height, unsigned int *bpp)
{
    return syscall3(int, SYS_GUIINFO, width, height, bpp);
}

int g_layer_get_wintop()
{
    return syscall0(int, SYS_LAYERGETWINTOP);
}

int g_layer_set_wintop(int top)
{
    return syscall1(int, SYS_LAYERSETWINTOP, top);
}

int g_layer_get_focus()
{
    return syscall0(int, SYS_LAYERGETFOCUS);
}

int g_layer_set_focus(int ly)
{
    return syscall1(int, SYS_LAYERSETFOCUS, ly);
}
int g_init(void)
{
    int val = syscall0(int, SYS_GINIT);
    return val;
}

int g_quit(void)
{
    int val = syscall0(int, SYS_GQUIT);
    
    return val;
}

int g_set_routine(int (*routine)(g_msg_t *))
{

    return 0;
}

int g_get_msg(g_msg_t *msg)
{
    int val = syscall1(int, SYS_GGETMSG, msg);
    
    return val;
}

int g_try_get_msg(g_msg_t *msg)
{
    int val = syscall1(int, SYS_GTRYGETMSG, msg);
    return val;
}

int g_dispatch_msg(g_msg_t *msg)
{
    return 0;
}
