#include "xbrower_section.h"
#include "xbrower_view.h"
#include "xbrower_misc.h"
#include <xbrower_bitmap.h>
#include <math.h>

int xbrower_render_putpixel(xbrower_view_t *view, int x, int y, xbrower_color_t color)
{
    if (!view)
        return -1;
    if (!view->section)
        return -1;
    if (x < 0 || x >= view->width || y < 0 || y >= view->height) {
        return -1;
    }
    xbrower_color_t *buf = (xbrower_color_t *)view->section->addr;
    buf[y * view->width + x] = color;
    return 0;
}

int xbrower_render_getpixel(xbrower_view_t *view, int x, int y, xbrower_color_t *color)
{
    if (!view)
        return -1;
    if (!view->section)
        return -1;
    if (x < 0 || x >= view->width || y < 0 || y >= view->height) {
        return -1;
    }
    xbrower_color_t *buf = (xbrower_color_t *)view->section->addr;
    *color = buf[y * view->width + x];
    return 0;
}

void xbrower_render_clear(xbrower_view_t *view)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xbrower_color_t *buf = (xbrower_color_t *)view->section->addr;
    int i, j;
    for (j = 0; j < view->height; j++) {
        for (i = 0; i < view->width; i++) {
            buf[j * view->width + i] = 0;
        }
    }
}

void xbrower_render_vline(xbrower_view_t *view, int x, int y1, int y2, xbrower_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xbrower_color_t *buf = (xbrower_color_t *)view->section->addr;
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

void xbrower_render_hline(xbrower_view_t *view, int x1, int y, int x2, xbrower_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xbrower_color_t *buf = (xbrower_color_t *)view->section->addr;

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

void xbrower_render_line(xbrower_view_t *view, int x1, int y1, int x2, int y2, xbrower_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            xbrower_render_vline(view, x1, y1, y2, color);
        else 
            xbrower_render_vline(view, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            xbrower_render_hline(view, x1, x2, y1, color);
        else 
            xbrower_render_hline(view, x2, x1, y1, color);
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
        xbrower_render_putpixel(view, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void xbrower_render_rect_ex(xbrower_view_t *view, int x1, int y1, int x2, int y2, xbrower_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    /* left */
    xbrower_render_vline(view, x1, y1, y2, color);
    /* right */
    xbrower_render_vline(view, x2, y1, y2, color);
    /* top */
    xbrower_render_hline(view, x1, y1, x2, color);
    /* bottom */
    xbrower_render_hline(view, x1, y2, x2, color);
}

void xbrower_render_rectfill_ex(xbrower_view_t *view, int x1, int y1, int x2, int y2, xbrower_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;

    int i;
    for (i = 0; i <= y2 - y1; i++) {
        xbrower_render_hline(view, x1, y1 + i, x2, color);
    }
}

void xbrower_render_rect(xbrower_view_t *view, int x, int y, uint32_t width, uint32_t height, xbrower_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xbrower_render_rect_ex(view, x, y, x + width - 1, y + height - 1, color);
}

void xbrower_render_rectfill(xbrower_view_t *view, int x, int y, uint32_t width, uint32_t height, xbrower_color_t color)
{
    if (!view)
        return;
    if (!view->section)
        return;
    xbrower_render_rectfill_ex(view, x, y, x + width - 1, y + height - 1, color);
}

void xbrower_render_bitblt(xbrower_view_t *view, int x, int y, 
        xbrower_bitmap_t *bmp, int bx, int by, int width, int height)
{
    if (!view)
        return;
    if (!view->section)
        return;
    int w = min(width, bmp->width - bx);
    int h = min(height, bmp->height - by);
    if (w <= 0 || h <= 0)
        return;
    xbrower_color_t color;
    int i, j;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            xbrower_bitmap_getpixel(bmp, i + bx, j + by, &color);
            if (((color >> 24) & 0xff))
                xbrower_render_putpixel(view, x + i, y + j, color);
        }
    }
}
