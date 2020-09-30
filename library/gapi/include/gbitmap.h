#ifndef _GAPI_PIXMAP_H
#define _GAPI_PIXMAP_H

#include <stdint.h>
#include <sys/list.h>
#include "gcolor.h"
#include "gfont.h"

typedef struct {
    list_t list;
    unsigned int width;
    unsigned int height;
    g_color_t *buffer;
} g_bitmap_t;

/**
 * g_new_bitmap - create a new bitmap 
 * @width: bitmap width (pixel)
 * @width: bitmap height (pixel)
 * 
 * @return: none NULL is success, it's the bitmap struct , NULL is failed.
 */
g_bitmap_t *g_new_bitmap(unsigned int width, unsigned int height);

/**
 * g_del_bitmap - destroy a bitmap
 * @bitmap: bitmap addr, create by 'g_new_bitmap'
 * 
 * @return: 0 is success, -1 is failed.
 */
int g_del_bitmap(g_bitmap_t *bitmap);

/**
 * g_putpixel - put one pixel to a bitmap
 * @bmp: bitmap struct
 * @x: pixel x postion in bmp
 * @y: pixel y postion in bmp
 * @color: pixel color val, a g_color_t type
 */
void g_putpixel(g_bitmap_t *bmp, int x, int y, g_color_t color);

/**
 * g_getpixel - get one pixel from a bitmap
 * @bmp: bitmap struct
 * @x: pixel x postion in bmp
 * @y: pixel y postion in bmp
 * @color: pixel color val addr, save color data
 * 
 * @return: 0 is success, -1 is failed.
 */
int g_getpixel(g_bitmap_t *bmp, int x, int y, g_color_t *color);


/**
 * _g_putpixel - put one pixel to a bitmap without bound check
 * @bmp: bitmap struct
 * @x: pixel x postion in bmp
 * @y: pixel y postion in bmp
 * @color: pixel color val, a g_color_t type
 * 
 * this maybe unsafe, you must make sure x or y would not out of bitmap bound
 */
static inline void _g_putpixel(g_bitmap_t *bmp, int x, int y, g_color_t color)
{
    bmp->buffer[y * bmp->width + x] = color;
}

/**
 * _g_getpixel - get one pixel from a bitmap without bound check
 * @bmp: bitmap struct
 * @x: pixel x postion in bmp
 * @y: pixel y postion in bmp
 * @color: pixel color val addr, save color data
 * 
 * this maybe unsafe, you must make sure x or y would not out of bitmap bound
 * 
 * @return: 0 is success, -1 is failed.
 */
static inline void _g_getpixel(g_bitmap_t *bmp, int x, int y, g_color_t *color)
{
    *color = bmp->buffer[y * bmp->width + x];
}

/**
 * g_vline - draw a vertical line
 * @bmp: bitmap struct
 * @x: line x postion in bmp
 * @y1: line y1 postion in bmp
 * @y2: line y2 postion in bmp
 * @color: line color
 * 
 * draw a line on (x, y1) to (x, y2)
 */
void g_vline(g_bitmap_t *bmp, int x, int y1, int y2, g_color_t color);

/**
 * g_hline - draw a horizontal line
 * @bmp: bitmap struct
 * @x1: line x1 postion in bmp
 * @y: line y postion in bmp
 * @y2: line x2 postion in bmp
 * @color: line color
 * 
 * draw a line on (x1, y) to (x2, y)
 */
void g_hline(g_bitmap_t *bmp, int x1, int y, int x2, g_color_t color);

/**
 * g_line - draw a line
 * @bmp: bitmap struct
 * @x1: line x1 postion in bmp
 * @y1: line y1 postion in bmp
 * @x2: line x2 postion in bmp
 * @y2: line y2 postion in bmp
 * @color: line color
 * 
 * draw a line on (x1, y1) to (x2, y2)
 */
void g_line(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color);

/**
 * g_rect_ex - draw a rect extension
 * @bmp: bitmap struct
 * @x1: x1 postion in bmp
 * @y1: y1 postion in bmp
 * @x2: x2 postion in bmp
 * @y2: y2 postion in bmp
 * @color: rect color
 * 
 * draw rect from (x1, y1) to (x2, y2)
 */
void g_rect_ex(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color);

/**
 * g_rect_ex - draw a rect filled extension
 * @bmp: bitmap struct
 * @x1: x1 postion in bmp
 * @y1: y1 postion in bmp
 * @x2: x2 postion in bmp
 * @y2: y2 postion in bmp
 * @color: rect color
 * 
 * draw rect from (x1, y1) to (x2, y2)
 */
void g_rectfill_ex(g_bitmap_t *bmp, int x1, int y1, int x2, int y2, g_color_t color);

/**
 * g_rect - draw a rect
 * @bmp: bitmap struct
 * @x: x postion in bmp
 * @y: y postion in bmp
 * @width: rect width
 * @height: rect height
 * @color: rect color
 * 
 * draw rect at (x, y)
 */
void g_rect(g_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, g_color_t color);

/**
 * g_rectfill- draw a rect filled
 * @bmp: bitmap struct
 * @x: x postion in bmp
 * @y: y postion in bmp
 * @width: rect width
 * @height: rect height
 * @color: rect color
 * 
 * draw rect at (x, y)
 */
void g_rectfill(g_bitmap_t *bmp, int x, int y, uint32_t width, uint32_t height, g_color_t color);


/**
 * g_char - draw a char
 * @bmp: bitmap struct
 * @x: x postion in bmp
 * @y: y postion in bmp
 * @ch: ascii character
 * @color: char color
 * 
 * draw char at (x, y)
 */
void g_char(
    g_bitmap_t *bmp, 
    int x,
    int y,
    char ch,
    g_color_t color);

/**
 * g_char_ex - draw a char with font
 * @bmp: bitmap struct
 * @x: x postion in bmp
 * @y: y postion in bmp
 * @ch: ascii character
 * @color: char color
 * @font: char font
 * 
 * draw char at (x, y)
 */
void g_char_ex(
    g_bitmap_t *bmp, 
    int x,
    int y,
    char ch,
    g_color_t color,
    g_font_t *font);

/**
 * g_bitmap_clear - clear the bitmap
 * @bmp: bitmap struct
 */
void g_bitmap_clear(g_bitmap_t *bmp);

/**
 * g_text - draw a text
 * @bmp: bitmap struct
 * @x: x postion in bmp
 * @y: y postion in bmp
 * @text: ascii string
 * @color: char color
 * 
 * draw text at (x, y)
 */
void g_text(
    g_bitmap_t *bmp, 
    int x,
    int y,
    char *text,
    g_color_t color);

/**
 * g_bitmap_sync - sync bitmap to layer
 * @bmp: bitmap struct
 * @layer: the layer handle
 * @x: x postion in bmp
 * @y: y postion in bmp
 * 
 * bmp must sync to layer, then you can see the bmp on the screen
 */
int g_bitmap_sync(
    g_bitmap_t *bmp,
    int layer,
    int x,
    int y);
    
#endif /* _GAPI_PIXMAP_H */