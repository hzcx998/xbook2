#include <drivers/view/bitmap.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <xbook/memalloc.h>

view_bitmap_t *view_bitmap_create(unsigned int width, unsigned int height)
{
    view_bitmap_t *bitmap = mem_alloc(sizeof(view_bitmap_t));
    if (bitmap == NULL) {
        return NULL;
    }
    bitmap->bits = mem_alloc(width * height * sizeof(view_color_t));
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

int view_bitmap_destroy(view_bitmap_t *bitmap)
{
    if (!bitmap)
        return -1;
    if (bitmap->bits)
        free(bitmap->bits);
    free(bitmap);
    return 0;
}

void view_bitmap_putpixel(view_bitmap_t *bmp, int x, int y, view_color_t color)
{
    if (!bmp)
        return;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return;
    bmp->bits[y * bmp->width + x] = color;
}

int view_bitmap_getpixel(view_bitmap_t *bmp, int x, int y, view_color_t *color)
{
    if (!bmp)
        return -1;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return -1;
    *color = bmp->bits[y * bmp->width + x];
    return 0;
}

void view_bitmap_vline(view_bitmap_t *bmp, int x, int y1, int y2, view_color_t color)
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

void view_bitmap_hline(view_bitmap_t *bmp, int x1, int y, int x2, view_color_t color)
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

void view_bitmap_line(view_bitmap_t *bmp, int x1, int y1, int x2, int y2, view_color_t color)
{
    if (!bmp)
        return;
    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            view_bitmap_vline(bmp, x1, y1, y2, color);
        else 
            view_bitmap_vline(bmp, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            view_bitmap_hline(bmp, x1, x2, y1, color);
        else 
            view_bitmap_hline(bmp, x2, x1, y1, color);
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
        view_bitmap_putpixel(bmp, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void view_bitmap_rect_ex(view_bitmap_t *bmp, int x1, int y1, int x2, int y2, view_color_t color)
{
    if (!bmp)
        return;
    /* left */
    view_bitmap_vline(bmp, x1, y1, y2, color);
    /* right */
    view_bitmap_vline(bmp, x2, y1, y2, color);
    /* top */
    view_bitmap_hline(bmp, x1, y1, x2, color);
    /* bottom */
    view_bitmap_hline(bmp, x1, y2, x2, color);
}

void view_bitmap_rectfill_ex(view_bitmap_t *bmp, int x1, int y1, int x2, int y2, view_color_t color)
{
    if (!bmp)
        return;

    int i;
    for (i = 0; i <= y2 - y1; i++) {
        view_bitmap_hline(bmp, x1, y1 + i, x2, color);
    }
}

void view_bitmap_rect(view_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, view_color_t color)
{
    if (!bmp)
        return;
    view_bitmap_rect_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}

void view_bitmap_rectfill(view_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, view_color_t color)
{
    if (!bmp)
        return;
    view_bitmap_rectfill_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}

void view_bitmap_clear(view_bitmap_t *bmp)
{
    memset(bmp->bits, 0, bmp->width * bmp->height * sizeof(view_color_t));
}

void view_bitmap_blit(view_bitmap_t *src, view_rect_t *srcrect, view_bitmap_t *dst, view_rect_t *dstrect)
{
    if (!src || !dst)
        return;
    // 处理src内部矩形
    view_rect_t _srcrect;
    if (!srcrect) {
        _srcrect.x = 0;
        _srcrect.y = 0;
        _srcrect.w.sw = src->width;
        _srcrect.h.sh = src->height;
        srcrect = &_srcrect;
    } else {
        srcrect->w.sw = min(srcrect->w.sw, src->width);
        srcrect->h.sh = min(srcrect->h.sh, src->height);
    }
    // 处理dst内部矩形
    view_rect_t _dstrect;
    if (!dstrect) {
        _dstrect.x = 0;
        _dstrect.y = 0;
        _dstrect.w.sw = dst->width;
        _dstrect.h.sh = dst->height;
        dstrect = &_dstrect;
    } else {
        dstrect->w.sw = min(dstrect->w.sw, dst->width);
        dstrect->h.sh = min(dstrect->h.sh, dst->height);
    }

    uint32_t color = 0;
    int src_x = srcrect->x; 
    int src_y = srcrect->y;
    int src_w = srcrect->w.sw; 
    int src_h = srcrect->h.sh;
    int dst_x = dstrect->x;
    int dst_y = dstrect->y;
    int dst_w = dstrect->w.sw;
    int dst_h = dstrect->h.sh;
    while (src_h > 0 && dst_h > 0) {
        for (src_x = srcrect->x, dst_x = dstrect->x, src_w = srcrect->w.sw, dst_w = dstrect->w.sw;
            src_w > 0 && dst_w > 0;
            src_x++, dst_x++, src_w--, dst_w--)
        {
            if (!view_bitmap_getpixel(src, src_x, src_y, &color)) {
                if (((color >> 24) & 0xff))
                    view_bitmap_putpixel(dst, dst_x, dst_y, color);
            }   
        }
        src_y++;
        dst_y++;
        src_h--;
        dst_h--;
    }
}
