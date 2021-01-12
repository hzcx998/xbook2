#ifndef _LIB_XTK_H
#define _LIB_XTK_H

#include <stdint.h>

/* X tool kit
window
spirit
canvas
dialog
*/

typedef unsigned int xtk_color_t;

typedef struct {
    int w;
    int h;
    int channels;
    unsigned char *buf; 
} xtk_image_t;

xtk_image_t *xtk_image_load(char *filename);
int xtk_image_destroy(xtk_image_t *img);
int xtk_image_resize(xtk_image_t *img, int w, int h);
xtk_image_t *xtk_image_load2(char *filename, int w, int h);


typedef enum {
    XTK_ALIGN_LEFT = 0,
    XTK_ALIGN_RIGHT,
    XTK_ALIGN_TOP,
    XTK_ALIGN_BOTTOM,
    XTK_ALIGN_CENTER,
} xtk_align_t;

typedef struct {
    /* border */
    xtk_color_t border_color;
    /* background */
    xtk_color_t background_color;
    xtk_align_t background_align;
    /* font */
    // style
    xtk_color_t color;
    xtk_align_t align;
} xtk_style_t;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    xtk_image_t *background_image;
    char *text;
    xtk_style_t style;
} xtk_spirit_t;

#include "xtk_text.h"

void xtk_test(int fd, uview_bitmap_t *wbmp);

#endif /* _LIB_XTK_H */