#include <uview.h>
#include <stdlib.h>

uview_bitmap_t *uview_bitmap_create(unsigned int width, unsigned int height)
{
    uview_bitmap_t *bitmap = malloc(sizeof(uview_bitmap_t));
    if (bitmap == NULL) {
        return NULL;
    }
    bitmap->bits = malloc(width * height * sizeof(uview_color_t));
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

int uview_bitmap_destroy(uview_bitmap_t *bitmap)
{
    if (!bitmap)
        return -1;
    if (bitmap->bits)
        free(bitmap->bits);
    free(bitmap);
    return 0;
}

void uview_bitmap_putpixel(uview_bitmap_t *bmp, int x, int y, uview_color_t color)
{
    if (!bmp)
        return;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return;
    bmp->bits[y * bmp->width + x] = color;
}

int uview_bitmap_getpixel(uview_bitmap_t *bmp, int x, int y, uview_color_t *color)
{
    if (!bmp)
        return -1;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return -1;
    *color = bmp->bits[y * bmp->width + x];
    return 0;
}

void uview_bitmap_vline(uview_bitmap_t *bmp, int x, int y1, int y2, uview_color_t color)
{
    if (!bmp)
        return;

    int offset = 0;
    int i = 0;

    if (x < 0)
        return;
    if (x > (bmp->width - 1))
        return;
    if (y1 > (bmp->height - 1))
        return;
    if (y2 < 0)
        return;

    for (i = 0; i <= y2 - y1; i++)
    {
        offset = (bmp->width * (y1 + i) + x);
        if (offset >= (bmp->width * bmp->height - 1))
            return;
        *(bmp->bits + offset) = color;
    }
}

void uview_bitmap_hline(uview_bitmap_t *bmp, int x1, int x2, int y, uview_color_t color)
{
    if (!bmp)
        return;

    int offset = 0;
    int i = 0;
    
    if (x1 > (bmp->width - 1))
        return;
    if (x2 < 0)
        return;
    if (y > (bmp->height - 1))
        return;
    if (y < 0)
        return;

    offset = ((bmp->width) * y + x1);
    if (offset >= (bmp->width * bmp->height - 1))
        return;
    for (i = 0; i <= x2 - x1; i++ )
        *(bmp->bits + offset + i) = color;

}

void uview_bitmap_line(uview_bitmap_t *bmp, int x1, int y1, int x2, int y2, uview_color_t color)
{
    if (!bmp)
        return;
    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            uview_bitmap_vline(bmp, x1, y1, y2, color);
        else 
            uview_bitmap_vline(bmp, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            uview_bitmap_hline(bmp, x1, x2, y1, color);
        else 
            uview_bitmap_hline(bmp, x2, x1, y1, color);
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
        uview_bitmap_putpixel(bmp, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void uview_bitmap_rect_ex(uview_bitmap_t *bmp, int x1, int y1, int x2, int y2, uview_color_t color)
{
    if (!bmp)
        return;
    /* left */
    uview_bitmap_vline(bmp, x1, y1, y2, color);
    /* right */
    uview_bitmap_vline(bmp, x2, y1, y2, color);
    /* top */
    uview_bitmap_hline(bmp, x1, x2, y1, color);
    /* bottom */
    uview_bitmap_hline(bmp, x1, x2, y2, color);
}

void uview_bitmap_rectfill_ex(uview_bitmap_t *bmp, int x1, int y1, int x2, int y2, uview_color_t color)
{
    if (!bmp)
        return;

    int i;
    for (i = 0; i <= y2 - y1; i++) {
        uview_bitmap_hline(bmp, x1, x2, y1 + i, color);
    }
}

void uview_bitmap_rect(uview_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, uview_color_t color)
{
    if (!bmp)
        return;
    uview_bitmap_rect_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}

void uview_bitmap_rectfill(uview_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, uview_color_t color)
{
    if (!bmp)
        return;
    uview_bitmap_rectfill_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}
