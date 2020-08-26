#include <sys/syscall.h>
#include <glayer.h>
#include <gshape.h>
#include <gfont.h>

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

int g_layer_outp(int layer, int x, int y, uint32_t color)
{
    g_point_t p;
    p.x = x;
    p.y = y;
    return syscall3(int, SYS_LAYEROUTP, layer, &p, color);
}

int g_layer_inp(int layer, int x, int y, uint32_t *color)
{
    g_point_t p;
    p.x = x;
    p.y = y;
    return syscall3(int, SYS_LAYERINP, layer, &p, color);
}

int g_layer_line(int layer, int x0, int y0, int x1, int y1, uint32_t color)
{
    g_line_t p;
    p.x0 = x0;
    p.y0 = y0;
    p.x1 = x1;
    p.y1 = y1;
    return syscall3(int, SYS_LAYERLINE, layer, &p, color);
}

int g_layer_rect(int layer, int x, int y, int width, int height, uint32_t color)
{
    g_rect_t p;
    p.x = x;
    p.y = y;
    p.width = width;
    p.height = height;
    return syscall3(int, SYS_LAYERRECT, layer, &p, color);
}

int g_layer_rect_fill(int layer, int x, int y, int width, int height, uint32_t color)
{
    g_rect_t p;
    p.x = x;
    p.y = y;
    p.width = width;
    p.height = height;
    return syscall3(int, SYS_LAYERRECTFILL, layer, &p, color);
}

int g_layer_pixmap(int layer, int x, int y, int width, int height, uint32_t *pixels, int bps)
{
    g_pixmap_t p;
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
    g_region_t p;
    p.left = left;
    p.top = top;
    p.right = right;
    p.bottom = bottom;
    return syscall2(int, SYS_LAYERREFRESH, layer, &p);
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


static void g_draw_word_bit(
    int layer,
    int x,
    int y,
    uint32_t color,
    uint8_t *data)
{
	unsigned int i;
	uint8_t d /* data */;
	for (i = 0; i < 16; i++) {
		d = data[i];
		if ((d & 0x80) != 0)
            g_layer_outp(layer, x + 0, y + i, color);
		if ((d & 0x40) != 0)
            g_layer_outp(layer, x + 1, y + i, color);
		if ((d & 0x20) != 0)
             g_layer_outp(layer, x + 2, y + i, color);
		if ((d & 0x10) != 0)
            g_layer_outp(layer, x + 3, y + i, color);
		if ((d & 0x08) != 0)
            g_layer_outp(layer, x + 4, y + i, color);
		if ((d & 0x04) != 0)
            g_layer_outp(layer, x + 5, y + i, color);
		if ((d & 0x02) != 0)
            g_layer_outp(layer, x + 6, y + i, color);
		if ((d & 0x01) != 0)
            g_layer_outp(layer, x + 7, y + i, color);
	}	
}

void g_draw_word_ex(
    int layer,
    int x,
    int y,
    char word,
    uint32_t color,
    g_font_t *font)
{
    if (!font)
        return;
    if (font->addr)
        g_draw_word_bit(layer, x, y, color, font->addr + word * font->char_height);
}

void g_draw_text_ex(
    int layer,
    int x,
    int y,
    char *text,
    uint32_t color,
    g_font_t *font)
{
    if (!font)
        return;

    while (*text) {
        g_draw_word_ex(layer, x, y, *text, color, font);
		x += font->char_width;
        text++;
	}
}

void g_layer_word(
    int layer,
    int x,
    int y,
    char ch,
    uint32_t color)
{
    g_draw_word_ex(layer, x, y, ch, color, g_current_font);
}

void g_layer_text(
    int layer,
    int x,
    int y,
    char *text,
    uint32_t color)
{
    g_draw_text_ex(layer, x, y, text, color, g_current_font);
}
