#include <drivers/screen.h>
#include <graph/point.h>

int graph_put_point(int x, int y, GUI_COLOR color)
{
    return screen.output_pixel(x, y, screen.gui_to_screen_color(color));
}

int graph_get_point(int x, int y, GUI_COLOR *color)
{
    screen.input_pixel(x, y, color);
    *color = screen.screen_to_gui_color(*color);
    return 0;
}
