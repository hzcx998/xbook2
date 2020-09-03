#include <gui/screen.h>
#include <gui/draw.h>

int layer_put_point(layer_t *layer, int x, int y, GUI_COLOR color)
{
    if (x < 0 || y < 0 || x >= layer->width || y >= layer->height)
        return -1;
    layer->buffer[y * layer->width + x] = color;
    return 0;
}

int layer_get_point(layer_t *layer, int x, int y, GUI_COLOR *color)
{
    if (x < 0 || y < 0 || x >= layer->width || y >= layer->height)
        return -1;
    *color = layer->buffer[y * layer->width + x];
    return 0;
}
