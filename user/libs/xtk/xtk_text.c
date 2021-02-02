#include "xtk_text.h"
#include <assert.h>

dotfont_library_t __xtk_dotflib;

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
    dotfont_t *stdfnt =  dotfont_find(&__xtk_dotflib, family);
    if (!stdfnt)
        return -1;
    char *p = text;
    int _x = x, _y = y;
    while (*p) {
        switch (*p)
        {
        case '\n':
        case '\b':
        case '\t':
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

static void xtk_char_to_surface(dotfont_t *font, char ch,
        xtk_surface_t *surface, int x, int y, uint32_t color)
{
    uint8_t *addr = dotfont_get_addr(font, ch);
    assert(addr);
    int i, j;
	uint8_t d /* data */;
	for (i = 0; i < 16; i++) {
		d = addr[i];
        for (j = (8 - 1); j >= 0; j--) {
            if ((d & (1 << j))) {
                xtk_surface_putpixel(surface, x + (8 - 1) - j, y + i, color);
            }
        }
    }
}

int xtk_text_to_surface(char *text, uint32_t color, char *family,
        xtk_surface_t *surface, int x, int y)
{
    dotfont_t *stdfnt =  dotfont_find(&__xtk_dotflib, family);
    if (!stdfnt)
        return -1;
    char *p = text;
    int _x = x, _y = y;
    while (*p) {
        switch (*p)
        {
        case '\n':
        case '\b':
        case '\t':
            break;
        default:
            xtk_char_to_surface(stdfnt, *p, surface, _x, _y, color);
            _x += dotfont_get_char_width(stdfnt);
            break;
        }
        p++;
    }
    return 0;
}

int xtk_text_init()
{
    dotfont_init(&__xtk_dotflib);
    return 0;
}
