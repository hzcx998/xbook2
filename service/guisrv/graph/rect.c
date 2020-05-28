#include <graph/line.h>
#include <graph/rect.h>
#include <drivers/screen.h>

void graph_fill_rect(int x, int y, int width, int height, GUI_COLOR color)
{
    screen.output_rect_fill(x, y, x + width - 1, y + height - 1, screen.gui_to_screen_color(color));
}

void graph_draw_rect(int x, int y, int width, int height, GUI_COLOR color)
{
    /* left */
    screen.output_vline(x, y, y + height - 1, screen.gui_to_screen_color(color));
    /* right */
    screen.output_vline(x + width - 1, y, y + height - 1, screen.gui_to_screen_color(color));
    /* top */
    screen.output_hline(x, x + width - 1, y, screen.gui_to_screen_color(color));
    /* bottom */
    screen.output_hline(x, x + width - 1, y + height - 1, screen.gui_to_screen_color(color));
}
