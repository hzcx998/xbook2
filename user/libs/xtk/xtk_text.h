#ifndef _LIB_XTK_TEXT_H
#define _LIB_XTK_TEXT_H

#include <uview.h>
#include <dotfont.h>

int xtk_text_to_bitmap(char *text, uint32_t color, char *family,
        uview_bitmap_t *bmp, int x, int y);
int xtk_text_init();

#endif /* _LIB_XTK_TEXT_H */