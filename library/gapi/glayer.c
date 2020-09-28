#include <sys/syscall.h>
#include <glayer.h>
#include <gshape.h>
#include <gfont.h>
#include <gbitmap.h>
#include <string.h>

/* 图层id都大于0，因此0表示没有图层 */
static int _g_layer_table[G_LAYER_NR] = {0,};
static int _g_layer_desktop = -1;

static int g_layer_alloc_solt()
{
    int i;
    for (i = 0; i < G_LAYER_NR; i++) {
        if (!_g_layer_table[i])
            return i;
    }
    return -1;
}

static int g_layer_free_solt(int id)
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

int g_layer_new(int x, int y, uint32_t width, uint32_t height)
{
    int solt = g_layer_alloc_solt();
    if (solt < 0)
        return -1;
    int id = syscall4(int, SYS_LAYERNEW, x, y, width, height);
    if (id < 0) {
        return -1;
    }
    _g_layer_table[solt] = id;
    return id;
}

int g_layer_del(int layer)
{
    if (g_layer_z(layer, -1) < 0)
        return -1;
    int retval = syscall1(int, SYS_LAYERDEL, layer);
    if (retval < 0)
        return -1;
    return g_layer_free_solt(layer);
}

int g_layer_del_all()
{
    int i;
    for (i = 0; i < G_LAYER_NR; i++) {
        if (_g_layer_table[i] > 0) {
            if (g_layer_del(_g_layer_table[i]) < 0)
                return -1;
        }
    }
    return 0;
}

int g_layer_move(int layer, int x, int y)
{
    return syscall3(int, SYS_LAYERMOVE, layer, x, y);
}

int g_layer_z(int layer, int z)
{
    return syscall2(int, SYS_LAYERZ, layer, z);
}

int g_layer_set_flags(int layer, uint32_t flags)
{
    return syscall2(int, SYS_LAYERSETFLG, layer, flags);
}

int g_layer_refresh(int layer, int left, int top, int right, int bottom)
{
    g_region_t p;
    p.left = left;
    p.top = top;
    p.right = right;
    p.bottom = bottom;
    return syscall2(int, SYS_LAYERREFRESH, layer, &p);
}

int g_layer_paint(int layer, int x, int y, g_bitmap_t *bmp)
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
    return g_layer_sync_bitmap(
        layer,
        &rect,
        bmp->buffer,
        &regn);
}

int g_layer_paint_ex(int layer, int x, int y, g_bitmap_t *bmp)
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
    return g_layer_sync_bitmap_ex(
        layer,
        &rect,
        bmp->buffer,
        &regn);
}

int g_layer_get_wintop()
{
    return syscall0(int, SYS_LAYERGETWINTOP);
}

int g_layer_set_wintop(int top)
{
    return syscall1(int, SYS_LAYERSETWINTOP, top);
}

int g_layer_get_desktop()
{
    if (_g_layer_desktop == -1)
        _g_layer_desktop = syscall0(int, SYS_LAYERGETDESKTOP);
    return _g_layer_desktop;
}

int g_layer_set_desktop(int id)
{
    return syscall1(int, SYS_LAYERSETDESKTOP, id);
}

int g_layer_get_focus()
{
    return syscall0(int, SYS_LAYERGETFOCUS);
}

int g_layer_set_focus(int ly)
{
    return syscall1(int, SYS_LAYERSETFOCUS, ly);
}

int g_layer_focus(int ly)
{
    return syscall1(int, SYS_LAYERFOCUS, ly);
}

int g_layer_resize(int ly, int x, int y, uint32_t width, uint32_t height)
{
    g_rect_t rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    return syscall2(int, SYS_LAYERRESIZE, ly, &rect);
}

int g_layer_set_region(int layer, int type, int left, int top, int right, int bottom)
{
    g_region_t rg;
    rg.left = left;
    rg.top = top;
    rg.right = right;
    rg.bottom = bottom;
    return syscall3(int , SYS_LAYERSETREGION, layer, type, &rg);
}

int g_layer_focus_win_top()
{
    return syscall0(int, SYS_LAYERFOCUSWINTOP);
}


int g_layer_sync_bitmap(
    int layer,
    g_rect_t *rect,
    g_color_t *bitmap,
    g_region_t *region)
{
    syscall4(int, SYS_LAYERSYNCBMP, layer, rect, bitmap, region);
    return 0;
}

int g_layer_sync_bitmap_ex(
    int layer,
    g_rect_t *rect,
    g_color_t *bitmap,
    g_region_t *region)
{
    syscall4(int, SYS_LAYERSYNCBMPEX, layer, rect, bitmap, region);
    return 0;
}

int g_layer_copy_bitmap(
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
