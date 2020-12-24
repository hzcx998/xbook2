#include "xgui_bitmap.h"
#include <xgui_dotfont.h>
#include <stdlib.h>

xgui_bitmap_t *xgui_bitmap_create(unsigned int width, unsigned int height)
{
    xgui_bitmap_t *bitmap = malloc(sizeof(xgui_bitmap_t));
    if (bitmap == NULL) {
        return NULL;
    }
    bitmap->bits = malloc(width * height * sizeof(xgui_color_t));
    if (bitmap->bits == NULL) {
        free(bitmap);
        return NULL;
    }
    int i;
    for (i = 0; i < width * height; i++) {
        bitmap->bits[i] = 0;  // 清0
    }
    bitmap->width = width;
    bitmap->height = height;
    return bitmap;
}

int xgui_bitmap_destroy(xgui_bitmap_t *bitmap)
{
    if (!bitmap)
        return -1;
    if (bitmap->bits)
        free(bitmap->bits);
    free(bitmap);
    return 0;
}

void xgui_bitmap_putpixel(xgui_bitmap_t *bmp, int x, int y, xgui_color_t color)
{
    if (!bmp)
        return;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return;
    bmp->bits[y * bmp->width + x] = color;
}

int xgui_bitmap_getpixel(xgui_bitmap_t *bmp, int x, int y, xgui_color_t *color)
{
    if (!bmp)
        return -1;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return -1;
    *color = bmp->bits[y * bmp->width + x];
    return 0;
}

void xgui_bitmap_vline(xgui_bitmap_t *bmp, int x, int y1, int y2, xgui_color_t color)
{
    if (!bmp)
        return;

    int offset = 0;
    int i = 0;

    if (x > (bmp->width - 1))
        return;
    if (y1 > (bmp->height - 1))
        return;
    if (y2 > (bmp->height - 1))
        return;

    for (i = 0; i <= y2 - y1; i++)
    {
        offset = (bmp->width * (y1 + i) + x);
        if (offset >= (bmp->width * bmp->height - 1))
            return;
        *(bmp->bits + offset) = color;
    }
}

void xgui_bitmap_hline(xgui_bitmap_t *bmp, int x1, int y, int x2, xgui_color_t color)
{
    if (!bmp)
        return;

    int offset = 0;
    int i = 0;
    
    if (x1 > (bmp->width - 1))
        return;
    if (x2 > (bmp->width - 1))
        return;
    if (y > (bmp->height - 1))
        return;

    offset = ((bmp->width) * y + x1);
    if (offset >= (bmp->width * bmp->height - 1))
        return;
    for (i = 0; i <= x2 - x1; i++ )
        *(bmp->bits + offset + i) = color;

}

void xgui_bitmap_line(xgui_bitmap_t *bmp, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    if (!bmp)
        return;
    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            xgui_bitmap_vline(bmp, x1, y1, y2, color);
        else 
            xgui_bitmap_vline(bmp, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            xgui_bitmap_hline(bmp, x1, x2, y1, color);
        else 
            xgui_bitmap_hline(bmp, x2, x1, y1, color);
        return;
    }
    int i, x, y, len, dx, dy;
	dx = x2 - x1;
	dy = y2 - y1;
	
	x = x1 << 10;
	y = y1 << 10;
	
	if(dx < 0){
		dx = -dx;
	}
	if(dy < 0){
		dy = -dy;
	}
	if(dx >= dy ){
		len = dx + 1;
		if(x1 > x2){
			dx = -1024;
		} else {
			dx = 1024;
		}
		if(y1 <= y2){
			dy = ((y2 - y1 + 1) << 10)/len;
		} else {
			dy = ((y2 - y1 - 1) << 10)/len;
		}
	}else{
		len = dy + 1;
		if(y1 > y2){
			dy = -1024;
		} else {
			dy = 1024;
		}
		if(x1 <= x2){
			dx = ((x2 - x1 + 1) << 10)/len;
		} else {
			dx = ((x2 - x1 - 1) << 10)/len;
		}	
	}
	for(i = 0; i < len; i++){
        xgui_bitmap_putpixel(bmp, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void xgui_bitmap_rect_ex(xgui_bitmap_t *bmp, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    if (!bmp)
        return;
    /* left */
    xgui_bitmap_vline(bmp, x1, y1, y2, color);
    /* right */
    xgui_bitmap_vline(bmp, x2, y1, y2, color);
    /* top */
    xgui_bitmap_hline(bmp, x1, y1, x2, color);
    /* bottom */
    xgui_bitmap_hline(bmp, x1, y2, x2, color);
}

void xgui_bitmap_rectfill_ex(xgui_bitmap_t *bmp, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    if (!bmp)
        return;

    int i;
    for (i = 0; i <= y2 - y1; i++) {
        xgui_bitmap_hline(bmp, x1, y1 + i, x2, color);
    }
}

void xgui_bitmap_rect(xgui_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, xgui_color_t color)
{
    if (!bmp)
        return;
    xgui_bitmap_rect_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}

void xgui_bitmap_rectfill(xgui_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, xgui_color_t color)
{
    if (!bmp)
        return;
    xgui_bitmap_rectfill_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}

void xgui_bitmap_char(
        xgui_bitmap_t *bmp, 
        int x,
        int y,
        char ch,
        xgui_color_t color)
{
    if (!bmp)
        return;
    xgui_dotfont_t *font = xgui_dotfont_current();
    if (!font)
        return;
    if (!font->addr)
        return;
    uint8_t *data = font->addr + ch * font->char_height;
    unsigned int i;
	uint8_t d /* data */;
	for (i = 0; i < 16; i++) {
		d = data[i];
		if ((d & 0x80) != 0)
            xgui_bitmap_putpixel(bmp, x + 0, y + i, color);
		if ((d & 0x40) != 0)
            xgui_bitmap_putpixel(bmp, x + 1, y + i, color);
		if ((d & 0x20) != 0)
             xgui_bitmap_putpixel(bmp, x + 2, y + i, color);
		if ((d & 0x10) != 0)
            xgui_bitmap_putpixel(bmp, x + 3, y + i, color);
		if ((d & 0x08) != 0)
            xgui_bitmap_putpixel(bmp, x + 4, y + i, color);
		if ((d & 0x04) != 0)
            xgui_bitmap_putpixel(bmp, x + 5, y + i, color);
		if ((d & 0x02) != 0)
            xgui_bitmap_putpixel(bmp, x + 6, y + i, color);
		if ((d & 0x01) != 0)
            xgui_bitmap_putpixel(bmp, x + 7, y + i, color);
	}
}

void xgui_bitmap_text(
    xgui_bitmap_t *bmp, 
    int x,
    int y,
    char *text,
    xgui_color_t color)
{
    if (!bmp)
        return;
    int cur_x = x;
    int cur_y = y;
    xgui_dotfont_t *font = xgui_dotfont_current();
    while (*text) {
        switch (*text) {
        case '\n':
            cur_x = x;
            cur_y += font->char_height;
            break;
        case '\b':
            cur_x -= font->char_width;
            if (cur_x < x)
                cur_x = x;
            break;
        default:
            xgui_bitmap_char(bmp, cur_x, cur_y, *text, color);
            cur_x += 8;
            break;
        }
        text++;
    }
}

void xgui_bitmap_clear(xgui_bitmap_t *bmp)
{
    if (!bmp)
        return;
    int i, j;
    for (j = 0; j < bmp->height; j++) {
        for (i = 0; i < bmp->width; i++) {
            bmp->bits[j * bmp->width + i] = 0;
        }
    }
}