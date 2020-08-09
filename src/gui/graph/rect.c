#include <gui/screen.h>
#include <gui/rect.h>

/* 封装screen */
void gui_draw_rect_fill(int x, int y, int width, int height, GUI_COLOR color)
{
    gui_screen.output_rect_fill(x, y, x + width - 1, y + height - 1, color);
}

/* 封装screen */
void gui_draw_rect(int x, int y, int width, int height, GUI_COLOR color)
{
    /* left */
    gui_screen.output_vline(x, y, y + height - 1, color);
    /* right */
    gui_screen.output_vline(x + width - 1, y, y + height - 1, color);
    /* top */
    gui_screen.output_hline(x, x + width - 1, y, color);
    /* bottom */
    gui_screen.output_hline(x, x + width - 1, y + height - 1, color);
}
