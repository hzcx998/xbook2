#ifndef __GUISRV_WINDOW_CHAR_H__
#define __GUISRV_WINDOW_CHAR_H__

#include "window.h"
#include <font/font.h>

int gui_window_draw_word(
    gui_window_t *window,
    int x,
    int y,
    char word,
    GUI_COLOR color
);

int gui_window_draw_text(
    gui_window_t *window,
    int x,
    int y,
    char *text,
    GUI_COLOR color
);

int gui_window_draw_word_ex(
    gui_window_t *window,
    int x,
    int y,
    char word,
    GUI_COLOR color,
    gui_font_t *font
);

int gui_window_draw_text_ex(
    gui_window_t *window,
    int x,
    int y,
    char *text,
    GUI_COLOR color,
    gui_font_t *font
);

#endif  /* __GUISRV_WINDOW_CHAR_H__ */
