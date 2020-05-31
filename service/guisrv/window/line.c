#include <window/line.h>
#include <layer/draw.h>

int gui_window_draw_line(gui_window_t *window, int x0, int y0, int x1, int y1, GUI_COLOR color)
{
    if (!window)
        return -1;
    layer_draw_line(window->layer, window->x_off + x0, window->y_off + y0, 
        window->x_off + x1, window->y_off + y1, color);
    return 0;
}
