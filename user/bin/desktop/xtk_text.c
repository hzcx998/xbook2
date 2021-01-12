#include "xtk_text.h"
#include <assert.h>

static dotfont_library_t __dotflib;

static void xtk_char_to_bitmap(dotfont_t *font, char ch,
        uview_bitmap_t *bmp, int x, int y, uint32_t color)
{
    uint8_t *addr = dotfont_get_addr(font, ch);
    assert(addr);
    int i, j;
	uint8_t d /* data */;
	for (i = 0; i < 16; i++) {
		d = addr[i];
        for (j = (8 - 1); j >= 0; j--) {
            if ((d & (1 << j))) {
                uview_bitmap_putpixel(bmp, x + (8 - 1) - j, y + i, color);
            }
        }
    }
}

int xtk_text_to_bitmap(char *text, uint32_t color, char *family,
        uview_bitmap_t *bmp, int x, int y)
{
    dotfont_t *stdfnt =  dotfont_find(&__dotflib, family);
    if (!stdfnt)
        return -1;
    char *p = text;
    int _x = x, _y = y;
    while (*p) {
        switch (*p)
        {
        case '\b':
            _x -= dotfont_get_char_width(stdfnt);
            xtk_char_to_bitmap(stdfnt, ' ', bmp, _x, _y, color);
            break;
        case '\n':
            _x = x;
            _y += dotfont_get_char_height(stdfnt);
            break;
        default:
            xtk_char_to_bitmap(stdfnt, *p, bmp, _x, _y, color);
            _x += dotfont_get_char_width(stdfnt);
            break;
        }
        p++;
    }
    return 0;
}

int xtk_dotfont_text(int uview, int x, int y, uint32_t w, uint32_t h,
        char *text, uint32_t color, char *family)
{
    if (uview < 0)
        return -1;
    uview_bitmap_t *bmp = uview_bitmap_create(w, h);
    assert(bmp);
    xtk_text_to_bitmap(text, color, family, bmp, 0, 0);
    uview_bitblt_update(uview, x, y, bmp);
    uview_bitmap_destroy(bmp);
    return 0;
}

int xtk_text_init()
{
    dotfont_init(&__dotflib);
    return 0;
}
