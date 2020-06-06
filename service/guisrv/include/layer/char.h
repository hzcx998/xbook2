#ifndef __GUISRV_LAYER_CHAR_H__
#define __GUISRV_LAYER_CHAR_H__

#include "color.h"
#include "layer.h"
#include <font/font.h>

void layer_draw_word_ex(
    layer_t *layer,
    int x,
    int y,
    char word,
    GUI_COLOR color,
    gui_font_t *font
);

void layer_draw_text_ex(
    layer_t *layer,
    int x,
    int y,
    char *text,
    GUI_COLOR color,
    gui_font_t *font
);

void layer_draw_word(
    layer_t *layer,
    int x,
    int y,
    char ch,
    GUI_COLOR color
);

void layer_draw_text(
    layer_t *layer,
    int x,
    int y,
    char *text,
    GUI_COLOR color
);

#endif  /* __GUISRV_LAYER_CHAR_H__ */
