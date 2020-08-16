#include <gui/screen.h>
#include <gui/draw.h>

void layer_draw_rect_fill(layer_t *layer, int x, int y, int width, int height, GUI_COLOR color)
{
    int i, j;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            layer_put_point(layer, x + i, y + j, color);
        }
    }
}

void layer_draw_rect(layer_t *layer, int x, int y, int width, int height, GUI_COLOR color)
{
    /* left */
    layer_put_vline(layer, x, y, y + height - 1, color);
    /* right */
    layer_put_vline(layer, x + width - 1, y, y + height - 1, color);
    /* top */
    layer_put_hline(layer, x, x + width - 1, y, color);
    /* bottom */
    layer_put_hline(layer, x, x + width - 1, y + height - 1, color);
}
