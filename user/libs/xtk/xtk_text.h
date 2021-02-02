#ifndef _LIB_XTK_TEXT_H
#define _LIB_XTK_TEXT_H

#include <uview.h>
#include <dotfont.h>
#include "xtk_surface.h"

int xtk_text_to_bitmap(char *text, uint32_t color, char *family,
        uview_bitmap_t *bmp, int x, int y);

int xtk_text_to_surface(char *text, uint32_t color, char *family,
        xtk_surface_t *surface, int x, int y);

int xtk_text_init();

#endif /* _LIB_XTK_TEXT_H */