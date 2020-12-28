#include "xgui_section.h"
#include "xgui_view.h"
#include "xgui_misc.h"
#include <xgui_bitmap.h>
#include <xgui_dotfont.h>
#include <math.h>

int xgui_vrender_putpixel(xgui_view_t *view, int x, int y, xgui_color_t color)
{
    if (!view)
        return -1;
    if (!view->section)
        return -1;
    if (x < 0 || x >= view->width || y < 0 || y >= view->height) {
        return -1;
    }
    xgui_color_t *buf = (xgui_color_t *)view->section->addr;
    buf[y * view->width + x] = color;
    return 0;
}

int xgui_vrender_getpixel(xgui_view_t *view, int x, int y, xgui_color_t *color)
{
    if (!view)
        return -1;
    if (!view->section)
        return -1;
    if (x < 0 || x >= view->width || y < 0 || y >= view->height) {
        return -1;
    }
    xgui_color_t *buf = (xgui_color_t *)view->section->addr;
    *color = buf[y * view->width + x];
    return 0;
}

void xgui_vrender_clear(xgui_view_t *view)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xgui_color_t *buf = (xgui_color_t *)view->section->addr;
    int i, j;
    for (j = 0; j < view->height; j++) {
        for (i = 0; i < view->width; i++) {
            buf[j * view->width + i] = 0;
        }
    }
}

void xgui_vrender_vline(xgui_view_t *view, int x, int y1, int y2, xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xgui_color_t *buf = (xgui_color_t *)view->section->addr;
    int offset = 0;
    int i = 0;

    if (x > (view->width - 1))
        return;
    if (y1 > (view->height - 1))
        return;
    if (y2 > (view->height - 1))
        return;

    for (i = 0; i <= y2 - y1; i++)
    {
        offset = (view->width * (y1 + i) + x);
        if (offset >= (view->width * view->height - 1))
            return;
        *(buf + offset) = color;
    }
}

void xgui_vrender_hline(xgui_view_t *view, int x1, int y, int x2, xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xgui_color_t *buf = (xgui_color_t *)view->section->addr;

    int offset = 0;
    int i = 0;
    
    if (x1 > (view->width - 1))
        return;
    if (x2 > (view->width - 1))
        return;
    if (y > (view->height - 1))
        return;

    offset = ((view->width) * y + x1);
    if (offset >= (view->width * view->height - 1))
        return;
    for (i = 0; i <= x2 - x1; i++ )
        *(buf + offset + i) = color;

}

void xgui_vrender_line(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            xgui_vrender_vline(view, x1, y1, y2, color);
        else 
            xgui_vrender_vline(view, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            xgui_vrender_hline(view, x1, x2, y1, color);
        else 
            xgui_vrender_hline(view, x2, x1, y1, color);
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
        xgui_vrender_putpixel(view, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void xgui_vrender_rect_ex(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    /* left */
    xgui_vrender_vline(view, x1, y1, y2, color);
    /* right */
    xgui_vrender_vline(view, x2, y1, y2, color);
    /* top */
    xgui_vrender_hline(view, x1, y1, x2, color);
    /* bottom */
    xgui_vrender_hline(view, x1, y2, x2, color);
}

void xgui_vrender_rectfill_ex(xgui_view_t *view, int x1, int y1, int x2, int y2, xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;

    int i;
    for (i = 0; i <= y2 - y1; i++) {
        xgui_vrender_hline(view, x1, y1 + i, x2, color);
    }
}

void xgui_vrender_rect(xgui_view_t *view, int x, int y, uint32_t width, uint32_t height, xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xgui_vrender_rect_ex(view, x, y, x + width - 1, y + height - 1, color);
}

void xgui_vrender_rectfill(xgui_view_t *view, int x, int y, uint32_t width, uint32_t height, xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xgui_vrender_rectfill_ex(view, x, y, x + width - 1, y + height - 1, color);
}

void xgui_vrender_bitblt(xgui_view_t *view, int x, int y, 
        xgui_bitmap_t *bmp, int bx, int by, int width, int height)
{
    if (!view)
        return;
    if (!view->section)
        return;
    int w = min(width, bmp->width - bx);
    int h = min(height, bmp->height - by);
    if (w <= 0 || h <= 0)
        return;
    xgui_color_t color;
    int i, j;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            xgui_bitmap_getpixel(bmp, i + bx, j + by, &color);
            if (((color >> 24) & 0xff))
                xgui_vrender_putpixel(view, x + i, y + j, color);
        }
    }
}

void xgui_vrender_char(
        xgui_view_t *view, 
        int x,
        int y,
        char ch,
        xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
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
            xgui_vrender_putpixel(view, x + 0, y + i, color);
		if ((d & 0x40) != 0)
            xgui_vrender_putpixel(view, x + 1, y + i, color);
		if ((d & 0x20) != 0)
             xgui_vrender_putpixel(view, x + 2, y + i, color);
		if ((d & 0x10) != 0)
            xgui_vrender_putpixel(view, x + 3, y + i, color);
		if ((d & 0x08) != 0)
            xgui_vrender_putpixel(view, x + 4, y + i, color);
		if ((d & 0x04) != 0)
            xgui_vrender_putpixel(view, x + 5, y + i, color);
		if ((d & 0x02) != 0)
            xgui_vrender_putpixel(view, x + 6, y + i, color);
		if ((d & 0x01) != 0)
            xgui_vrender_putpixel(view, x + 7, y + i, color);
	}
}

void xgui_vrender_text(
    xgui_view_t *view, 
    int x,
    int y,
    char *text,
    xgui_color_t color)
{
    if (!view)
        return;
    if (!view->section)
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
            xgui_vrender_char(view, cur_x, cur_y, *text, color);
            cur_x += 8;
            break;
        }
        text++;
    }
}
