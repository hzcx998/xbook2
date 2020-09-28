#include <gbitmap.h>
#include <glayer.h>
#include <malloc.h>

LIST_HEAD(__g_bitmap_list_head);

g_bitmap_t *g_new_bitmap(unsigned int width, unsigned int height)
{
    g_bitmap_t *bitmap = malloc(sizeof(g_bitmap_t));
    if (bitmap == NULL) {
        return NULL;
    }
    bitmap->buffer = malloc(width * height * sizeof(g_color_t));
    if (bitmap->buffer == NULL) {
        free(bitmap);
        return NULL;
    }
    int i;
    for (i = 0; i < width * height; i++) {
        bitmap->buffer[i] = 0;  // 清0
    }
    bitmap->width = width;
    bitmap->height = height;
    list_add(&bitmap->list, &__g_bitmap_list_head);
    return bitmap;
}

int g_del_bitmap(g_bitmap_t *bitmap)
{
    if (!bitmap)
        return -1;
    if (bitmap->buffer)
        free(bitmap->buffer);
    list_del(&bitmap->list);
    free(bitmap);
    return 0;
}

int g_del_bitmap_all()
{
    g_bitmap_t *bmp, *next;
    list_for_each_owner_safe (bmp, next, &__g_bitmap_list_head, list) {
        if (g_del_bitmap(bmp) < 0)
            return -1;
    }
    return 0;
}

void g_putpixel(g_bitmap_t *bmp, int x, int y, g_color_t color)
{
    if (!bmp)
        return;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return;
    bmp->buffer[y * bmp->width + x] = color;
}

int g_getpixel(g_bitmap_t *bmp, int x, int y, g_color_t *color)
{
    if (!bmp)
        return -1;
    if (x < 0 || y < 0 || x >= bmp->width || y >= bmp->height)
        return -1;
    *color = bmp->buffer[y * bmp->width + x];
    return 0;
}

void g_vline(g_bitmap_t *bmp, int x, int y1, int y2, g_color_t color)
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
        *(bmp->buffer + offset) = color;
    }
}

void g_hline(g_bitmap_t *bmp, int x1, int y, int x2, g_color_t color)
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
        *(bmp->buffer + offset + i) = color;

}

void g_line(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color)
{
    if (!bmp)
        return;
    if (x1 == x2) { /* 垂直的线 */
        if (y1 < y2) 
            g_vline(bmp, x1, y1, y2, color);
        else 
            g_vline(bmp, x1, y2, y1, color);
        return;
    } else if (y1 == y2) {  /* 水平的直线 */
        if (x1 < x2) 
            g_hline(bmp, x1, x2, y1, color);
        else 
            g_hline(bmp, x2, x1, y1, color);
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
        g_putpixel(bmp, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

void g_rect_ex(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color)
{
    if (!bmp)
        return;
    /* left */
    g_vline(bmp, x1, y1, y2, color);
    /* right */
    g_vline(bmp, x2, y1, y2, color);
    /* top */
    g_hline(bmp, x1, y1, x2, color);
    /* bottom */
    g_hline(bmp, x1, x2, y2, color);
}

void g_rectfill_ex(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color)
{
    if (!bmp)
        return;

    #if 1
    
    int i;
    for (i = 0; i <= y2 - y1; i++) {
        g_hline(bmp, x1, y1 + i, x2, color);
    }
    #else
    
    int i, j;
    for (j = 0; j <= y2 - y1; j++) {
        for (i = 0; i <= x2 - x1; i++) {
            g_putpixel(bmp, x1 + i, y1 + j, color);
        }
    }
    #endif
}

void g_rect(g_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, g_color_t color)
{
    if (!bmp)
        return;
    g_rect_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}

void g_rectfill(g_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, g_color_t color)
{
    if (!bmp)
        return;
    g_rectfill_ex(bmp, x, y, x + width - 1, y + height - 1, color);
}

void g_bitmap_clear(g_bitmap_t *bmp)
{
    if (!bmp)
        return;
    int i, j;
    for (j = 0; j < bmp->height; j++) {
        for (i = 0; i < bmp->width; i++) {
            bmp->buffer[j * bmp->width + i] = 0;
        }
    }
}

void g_char_ex(
    g_bitmap_t *bmp, 
    int x,
    int y,
    char ch,
    g_color_t color,
    g_font_t *font)
{
    if (!bmp)
        return;

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
            g_putpixel(bmp, x + 0, y + i, color);
		if ((d & 0x40) != 0)
            g_putpixel(bmp, x + 1, y + i, color);
		if ((d & 0x20) != 0)
             g_putpixel(bmp, x + 2, y + i, color);
		if ((d & 0x10) != 0)
            g_putpixel(bmp, x + 3, y + i, color);
		if ((d & 0x08) != 0)
            g_putpixel(bmp, x + 4, y + i, color);
		if ((d & 0x04) != 0)
            g_putpixel(bmp, x + 5, y + i, color);
		if ((d & 0x02) != 0)
            g_putpixel(bmp, x + 6, y + i, color);
		if ((d & 0x01) != 0)
            g_putpixel(bmp, x + 7, y + i, color);
	}
}

void g_char(
    g_bitmap_t *bmp, 
    int x,
    int y,
    char ch,
    g_color_t color)
{
    if (!bmp)
        return;
    
    g_char_ex(bmp, x, y, ch, color, g_current_font);
}

void g_text(
    g_bitmap_t *bmp, 
    int x,
    int y,
    char *text,
    g_color_t color)
{
    if (!bmp)
        return;
    while (*text) {
        g_char(bmp, x, y, *text, color);
        x += 8;
        text++;
    }
}

int g_bitmap_sync(
    g_bitmap_t *bmp,
    int layer,
    int x,
    int y)
{
    g_rect_t rect = {x, y, bmp->width, bmp->height};
    g_layer_sync_bitmap(layer, &rect, bmp->buffer, NULL);
    return 0;
}
