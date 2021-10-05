/**
 * Direct Window (DWIN)
 */

#ifndef _DWIN_BUFFER_H
#define _DWIN_BUFFER_H

#include <dwin/dwin_config.h>

struct dwin_buffer
{
    int width;
    int height;
    uint32_t *bits;
};
typedef struct dwin_buffer dwin_buffer_t;

struct dwin_rect
{
    int x;
    int y;
    int w;
    int h;
};
typedef struct dwin_rect dwin_rect_t;

static inline void dwin_buffer_init(dwin_buffer_t *buf, int width, int height, uint32_t *bits)
{
    buf->width = width;
    buf->height = height;
    buf->bits = bits;
}

#define dwin_buffer_getpixel(buf, x, y) ((buf)->bits[(y) * (buf)->width + (x)])
#define dwin_buffer_putpixel(buf, x, y, color) (buf)->bits[(y) * (buf)->width + (x)] = (color)

#endif   /* _DWIN_BUFFER_H */
