#include <sys/syscall.h>
#include <glayer.h>
#include <gshape.h>
#include <gfont.h>
#include <gbitmap.h>
#include <string.h>

/* 图层id都大于0，因此0表示没有图层 */
static int _g_layer_table[G_LAYER_NR] = {0,};
static int _g_layer_desktop = -1;

static int g_alloc_layer_solt()
{
    int i;
    for (i = 0; i < G_LAYER_NR; i++) {
        if (!_g_layer_table[i])
            return i;
    }
    return -1;
}

static int g_free_layer_solt(int id)
{
    int i;
    for (i = 0; i < G_LAYER_NR; i++) {
        if (_g_layer_table[i] == id) {
            _g_layer_table[i] = 0;
            return i;
        }
    }
    return -1;
}

int g_new_layer(int x, int y, uint32_t width, uint32_t height)
{
    int solt = g_alloc_layer_solt();
    if (solt < 0)
        return -1;
    int id = syscall4(int, SYS_LAYERNEW, x, y, width, height);
    if (id < 0) {
        return -1;
    }
    _g_layer_table[solt] = id;
    return id;
}

int g_del_layer(int layer)
{
    if (g_set_layer_z(layer, -1) < 0)
        return -1;
    int retval = syscall1(int, SYS_LAYERDEL, layer);
    if (retval < 0)
        return -1;
    return g_free_layer_solt(layer);
}

int g_del_layer_all()
{
    int i;
    for (i = 0; i < G_LAYER_NR; i++) {
        if (_g_layer_table[i] > 0) {
            if (g_del_layer(_g_layer_table[i]) < 0)
                return -1;
        }
    }
    return 0;
}

int g_move_layer(int layer, int x, int y)
{
    return syscall3(int, SYS_LAYERMOVE, layer, x, y);
}

int g_set_layer_z(int layer, int z)
{
    return syscall2(int, SYS_LAYERZ, layer, z);
}

int g_set_layer_flags(int layer, uint32_t flags)
{
    return syscall2(int, SYS_LAYERSETFLG, layer, flags);
}

int g_refresh_layer(int layer, int left, int top, int right, int bottom)
{
    g_region_t p;
    p.left = left;
    p.top = top;
    p.right = right;
    p.bottom = bottom;
    return syscall2(int, SYS_LAYERREFRESH, layer, &p);
}

int g_paint_layer(int layer, int x, int y, g_bitmap_t *bmp)
{
    if (layer < 0)
        return -1;

    g_rect_t rect;
    rect.x = x;
    rect.y = y;
    rect.width = bmp->width;
    rect.height = bmp->height;
    g_region_t regn;
    g_region_init(&regn);
    return g_sync_layer_bitmap(
        layer,
        &rect,
        bmp->buffer,
        &regn);
}

int g_paint_layer_ex(int layer, int x, int y, g_bitmap_t *bmp)
{
    if (layer < 0)
        return -1;

    g_rect_t rect;
    rect.x = x;
    rect.y = y;
    rect.width = bmp->width;
    rect.height = bmp->height;
    g_region_t regn;
    g_region_init(&regn);
    return g_sync_layer_bitmap_ex(
        layer,
        &rect,
        bmp->buffer,
        &regn);
}

int g_get_layer_wintop()
{
    return syscall0(int, SYS_LAYERGETWINTOP);
}

int g_set_layer_wintop(int top)
{
    return syscall1(int, SYS_LAYERSETWINTOP, top);
}

int g_get_layer_desktop()
{
    if (_g_layer_desktop == -1)
        _g_layer_desktop = syscall0(int, SYS_LAYERGETDESKTOP);
    return _g_layer_desktop;
}

int g_set_layer_desktop(int id)
{
    return syscall1(int, SYS_LAYERSETDESKTOP, id);
}

int g_get_layer_focus()
{
    return syscall0(int, SYS_LAYERGETFOCUS);
}

int g_set_layer_focus(int ly)
{
    return syscall1(int, SYS_LAYERSETFOCUS, ly);
}

int g_focus_layer(int ly)
{
    return syscall1(int, SYS_LAYERFOCUS, ly);
}

int g_resize_layer(int ly, int x, int y, uint32_t width, uint32_t height)
{
    g_rect_t rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    return syscall2(int, SYS_LAYERRESIZE, ly, &rect);
}

int g_set_layer_region(int layer, int type, int left, int top, int right, int bottom)
{
    g_region_t rg;
    rg.left = left;
    rg.top = top;
    rg.right = right;
    rg.bottom = bottom;
    return syscall3(int , SYS_LAYERSETREGION, layer, type, &rg);
}

int g_focus_layer_win_top()
{
    return syscall0(int, SYS_LAYERFOCUSWINTOP);
}


int g_sync_layer_bitmap(
    int layer,
    g_rect_t *rect,
    g_color_t *bitmap,
    g_region_t *region)
{
    syscall4(int, SYS_LAYERSYNCBMP, layer, rect, bitmap, region);
    return 0;
}

int g_sync_layer_bitmap_ex(
    int layer,
    g_rect_t *rect,
    g_color_t *bitmap,
    g_region_t *region)
{
    syscall4(int, SYS_LAYERSYNCBMPEX, layer, rect, bitmap, region);
    return 0;
}

int g_copy_layer_bitmap(
    int layer,
    g_rect_t *rect,
    g_color_t *bitmap,
    g_region_t *region)
{
    syscall4(int, SYS_LAYERCOPYBMP, layer, rect, bitmap, region);
    return 0;
}

int g_get_icon_path(int layer, char *path, uint32_t len)
{
    return syscall3(int, SYS_GGETICONPATH, layer, path, len);
}

int g_set_icon_path(int layer, char *path)
{
    uint32_t len = strlen(path);
    return syscall3(int, SYS_GSETICONPATH, layer, path, len);
}
