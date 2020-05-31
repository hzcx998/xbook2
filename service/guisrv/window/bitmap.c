#include <window/bitmap.h>
#include <layer/draw.h>

int gui_window_draw_bitmap(gui_window_t *window, int x, int y, int width, int height, GUI_COLOR *buffer)
{
    if (!window)
        return -1;
    layer_draw_bitmap(window->layer, window->x_off + x, window->y_off + y, 
        width, height, buffer);
    return 0;
}
