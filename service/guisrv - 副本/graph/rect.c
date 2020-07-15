#include <drivers/screen.h>
#include <graph/rect.h>

/* 封装screen */
void draw_rect_fill(int x, int y, int width, int height, GUI_COLOR color)
{
    drv_screen.output_rect_fill(x, y, x + width - 1, y + height - 1, color);
}

/* 封装screen */
void draw_rect(int x, int y, int width, int height, GUI_COLOR color)
{
    /* left */
    drv_screen.output_vline(x, y, y + height - 1, color);
    /* right */
    drv_screen.output_vline(x + width - 1, y, y + height - 1, color);
    /* top */
    drv_screen.output_hline(x, x + width - 1, y, color);
    /* bottom */
    drv_screen.output_hline(x, x + width - 1, y + height - 1, color);
}
