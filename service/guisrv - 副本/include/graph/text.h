#ifndef __GRAPH_TEXT_H__
#define __GRAPH_TEXT_H__

#include "color.h"
#include <font/font.h>

void draw_word_ex(
    int x,
    int y,
    char word,
    GUI_COLOR color,
    gui_font_t *font
);

void draw_text_ex(
    int x,
    int y,
    char *text,
    GUI_COLOR color,
    gui_font_t *font
);

void draw_word(
    int x,
    int y,
    char ch,
    GUI_COLOR color
);

void draw_text(
    int x,
    int y,
    char *text,
    GUI_COLOR color
);

#endif  /* __GRAPH_TEXT_H__ */
