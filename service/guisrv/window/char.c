#include <window/char.h>
#include <layer/draw.h>

int gui_window_draw_word(
    gui_window_t *window,
    int x,
    int y,
    char word,
    GUI_COLOR color
) {
    if (!window)
        return -1;
    layer_draw_word(window->layer, window->x_off + x, window->y_off + y, word, color);
    return 0;
}

int gui_window_draw_text(
    gui_window_t *window,
    int x,
    int y,
    char *text,
    GUI_COLOR color
) {
    if (!window)
        return -1;
    layer_draw_text(window->layer, window->x_off + x, window->y_off + y, text, color);
    return 0;
}

int gui_window_draw_word_ex(
    gui_window_t *window,
    int x,
    int y,
    char word,
    GUI_COLOR color,
    gui_font_t *font
) {
    if (!window)
        return -1;
    layer_draw_word_ex(window->layer, window->x_off + x,
        window->y_off + y, word, color, font);
    return 0;
}

int gui_window_draw_text_ex(
    gui_window_t *window,
    int x,
    int y,
    char *text,
    GUI_COLOR color,
    gui_font_t *font
) {
    if (!window)
        return -1;
    layer_draw_text_ex(window->layer, window->x_off + x,
        window->y_off + y, text, color, font);
    return 0;
}

