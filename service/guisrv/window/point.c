#include <window/point.h>
#include <layer/draw.h>

int gui_window_put_point(gui_window_t *window, int x, int y, GUI_COLOR color)
{
    if (!window)
        return -1;
    layer_put_point(window->layer, window->x_off + x, window->y_off + y, color);
    return 0;
}

int gui_window_get_point(gui_window_t *window, int x, int y, GUI_COLOR *color)
{
     if (!window)
        return -1;
    layer_get_point(window->layer, window->x_off + x, window->y_off + y, color);
    return 0;
}
