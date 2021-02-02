#include "xtk_surface.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

xtk_surface_t *xtk_surface_create(uint32_t width, uint32_t height)
{
    xtk_surface_t *surface = malloc(sizeof(xtk_surface_t));
    if (surface == NULL) {
        return NULL;
    }
    surface->pixels = (uint32_t *) malloc(width * height * sizeof(uint32_t));
    if (!surface->pixels) {
        free(surface);
        return NULL;
    }
    memset(surface->pixels, 0, width * height * sizeof(uint32_t));
    surface->w = width;
    surface->h = height;
    return surface;
}

int xtk_surface_destroy(xtk_surface_t *surface)
{
    if (!surface)
        return -1;
    if (!surface->pixels)
        return -1;
    free(surface->pixels);
    surface->pixels = NULL;
    free(surface);
    return 0;
}

void xtk_surface_clear(xtk_surface_t *surface)
{
    memset(surface->pixels, 0, surface->w * surface->h * sizeof(uint32_t));
}

void xtk_surface_putpixel(xtk_surface_t *surface, int x, int y, uint32_t color)
{
    if (!surface)
        return;
    if (x < 0 || y < 0 || x >= surface->w || y >= surface->h)
        return;
    surface->pixels[y * surface->w + x] = color;
}

int xtk_surface_getpixel(xtk_surface_t *surface, int x, int y, uint32_t *color)
{
    if (!surface)
        return -1;
    if (x < 0 || y < 0 || x >= surface->w || y >= surface->h)
        return -1;
    *color = surface->pixels[y * surface->w + x];
    return 0;
}

void xtk_surface_vline(xtk_surface_t *surface, int x, int y1, int y2, uint32_t color)
{
    if (!surface)
        return;
    if (!surface->pixels)
        return;

    int offset = 0;
    int i = 0;

    if (x < 0)
        return;
    if (x > (surface->w - 1))
        return;
    if (y1 > (surface->h - 1))
        return;
    if (y2 < 0)
        return;

    for (i = 0; i <= y2 - y1; i++)
    {
        offset = (surface->w * (y1 + i) + x);
        if (offset >= (surface->w * surface->h - 1))
            return;
        *(surface->pixels + offset) = color;
    }
}

void xtk_surface_hline(xtk_surface_t *surface, int x1, int x2, int y, uint32_t color)
{
    if (!surface)
        return;
    if (!surface->pixels)
        return;

    if (y > (surface->h - 1))
        return;
    if (y < 0)
        return;

    if (x1 < 0)
        x1 = 0;
    if (x2 > surface->w - 1)
        x2 = surface->w - 1;

    uint32_t *pixels = surface->pixels + (y * surface->w + x1);
    int i;
    for (i = 0; i <= x2 - x1; i++ )
        *(pixels + i) = color;
}

void xtk_surface_line(xtk_surface_t *surface, int x1, int y1, int x2, int y2, uint32_t color)
{
    if (!surface)
        return;
    if (!surface->pixels)
        return;

    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            xtk_surface_vline(surface, x1, y1, y2, color);
        else 
            xtk_surface_vline(surface, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            xtk_surface_hline(surface, x1, x2, y1, color);
        else 
            xtk_surface_hline(surface, x2, x1, y1, color);
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
        xtk_surface_putpixel(surface, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void xtk_surface_rect_ex(xtk_surface_t *surface, int x1, int y1, int x2, int y2, uint32_t color)
{
    if (!surface)
        return;
    if (!surface->pixels)
        return;

    /* left */
    xtk_surface_vline(surface, x1, y1, y2, color);
    /* right */
    xtk_surface_vline(surface, x2, y1, y2, color);
    /* top */
    xtk_surface_hline(surface, x1, x2, y1, color);
    /* bottom */
    xtk_surface_hline(surface, x1, x2, y2, color);
}

void xtk_surface_rectfill_ex(xtk_surface_t *surface, int x1, int y1, int x2, int y2, uint32_t color)
{
    if (!surface)
        return;
    if (!surface->pixels)
        return;
    
    int i;
    for (i = 0; i <= y2 - y1; i++) {
        xtk_surface_hline(surface, x1, x2, y1 + i, color);
    }
}

void xtk_surface_rect(xtk_surface_t *surface, int x, int y, uint32_t width, uint32_t height, uint32_t color)
{
    if (!surface)
        return;
    xtk_surface_rect_ex(surface, x, y, x + width - 1, y + height - 1, color);
}

void xtk_surface_rectfill(xtk_surface_t *surface, int x, int y, uint32_t width, uint32_t height, uint32_t color)
{
    if (!surface)
        return;
    xtk_surface_rectfill_ex(surface, x, y, x + width - 1, y + height - 1, color);
}

void xtk_surface_blit(xtk_surface_t *src, xtk_rect_t *srcrect, xtk_surface_t *dst, xtk_rect_t *dstrect)
{
    if (!src || !dst)
        return;
    // 处理src内部矩形
    xtk_rect_t _srcrect;
    if (!srcrect) {
        _srcrect.x = 0;
        _srcrect.y = 0;
        _srcrect.w = src->w;
        _srcrect.h = src->h;
        srcrect = &_srcrect;
    } else {
        srcrect->w = min(srcrect->w, src->w);
        srcrect->h = min(srcrect->h, src->h);
    }
    // 处理dst内部矩形
    xtk_rect_t _dstrect;
    if (!dstrect) {
        _dstrect.x = 0;
        _dstrect.y = 0;
        _dstrect.w = dst->w;
        _dstrect.h = dst->h;
        dstrect = &_dstrect;
    } else {
        dstrect->w = min(dstrect->w, dst->w);
        dstrect->h = min(dstrect->h, dst->h);
    }

    uint32_t color = 0;

    int src_x;
    int src_w;
    int src_y = srcrect->y;
    int src_h = srcrect->h;
    int dst_x;
    int dst_w;
    int dst_y = dstrect->y;
    int dst_h = dstrect->h;
    while (src_h > 0 && dst_h > 0) {
        for (src_x = srcrect->x, dst_x = dstrect->x, src_w = srcrect->w, dst_w = dstrect->w;
            src_w > 0 && dst_w > 0;
            src_x++, dst_x++, src_w--, dst_w--)
        {
            color = xtk_surface_getpixel_fast(src, src_x, src_y);
            if (((color >> 24) & 0xff))
                xtk_surface_putpixel_fast(dst, dst_x, dst_y, color);   
        }
        src_y++;
        dst_y++;
        src_h--;
        dst_h--;
    }
}
