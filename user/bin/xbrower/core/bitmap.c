#include <xbrower_bitmap.h>
#include <stdlib.h>

xbrower_bitmap_t *xbrower_bitmap_create(unsigned int width, unsigned int height)
{
    xbrower_bitmap_t *bitmap = malloc(sizeof(xbrower_bitmap_t));
    if (bitmap == NULL) {
        return NULL;
    }
    bitmap->bits = malloc(width * height * sizeof(xbrower_color_t));
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

int xbrower_bitmap_destroy(xbrower_bitmap_t *bitmap)
{
    if (!bitmap)
        return -1;
    if (bitmap->bits)
        free(bitmap->bits);
    free(bitmap);
    return 0;
}

void xbrower_bitmap_putpixel(xbrower_bitmap_t *bmp, int x, int y, xbrower_color_t color)
{
    if (!bmp)
        return;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return;
    bmp->bits[y * bmp->width + x] = color;
}

int xbrower_bitmap_getpixel(xbrower_bitmap_t *bmp, int x, int y, xbrower_color_t *color)
{
    if (!bmp)
        return -1;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return -1;
    *color = bmp->bits[y * bmp->width + x];
    return 0;
}

void xbrower_bitmap_vline(xbrower_bitmap_t *bmp, int x, int y1, int y2, xbrower_color_t color)
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

void xbrower_bitmap_hline(xbrower_bitmap_t *bmp, int x1, int y, int x2, xbrower_color_t color)
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

void xbrower_bitmap_line(xbrower_bitmap_t *bmp, int x1, int y1, int x2, int y2, xbrower_color_t color)
{
    if (!bmp)
        return;
    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            xbrower_bitmap_vline(bmp, x1, y1, y2, color);
        else 
            xbrower_bitmap_vline(bmp, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            xbrower_bitmap_hline(bmp, x1, x2, y1, color);
        else 
            xbrower_bitmap_hline(bmp, x2, x1, y1, color);
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
        xbrower_bitmap_putpixel(bmp, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void xbrower_bitmap_rect_ex(xbrower_bitmap_t *bmp, int x1, int y1, int x2, int y2, xbrower_color_t color)
{
    if (!bmp)
        return;
    /* left */
    xbrower_bitmap_vline(bmp, x1, y1, y2, color);
    /* right */
    xbrower_bitmap_vline(bmp, x2, y1, y2, color);
    /* top */
    xbrower_bitmap_hline(bmp, x1, y1, x2, color);
    /* bottom */
    xbrower_bitmap_hline(bmp, x1, y2, x2, color);
}

void xbrower_bitmap_rectfill_ex(xbrower_bitmap_t *bmp, int x1, int y1, int x2, int y2, xbrower_color_t color)
{
    if (!bmp)
        return;

    int i;
    for (i = 0; i <= y2 - y1; i++) {
        xbrower_bitmap_hline(bmp, x1, y1 + i, x2, color);
    }
}

void xbrower_bitmap_rect(xbrower_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, xbrower_color_t color)
{
    if (!bmp)
        return;
    xbrower_bitmap_rect_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}

void xbrower_bitmap_rectfill(xbrower_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, xbrower_color_t color)
{
    if (!bmp)
        return;
    xbrower_bitmap_rectfill_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}
