#include <window/rect.h>
#include <layer/draw.h>

int gui_window_draw_rect(gui_window_t *window, int x, int y, int width, int height, GUI_COLOR color)
{
    if (!window)
        return -1;
    layer_draw_rect(window->layer, window->x_off + x, window->y_off + y, width, height, color);
    return 0;
}

int gui_window_draw_rect_fill(gui_window_t *window, int x, int y, int width, int height, GUI_COLOR color)
{
     if (!window)
        return -1;
    layer_draw_rect_fill(window->layer, window->x_off + x, window->y_off + y, width, height, color);
    return 0;
}
