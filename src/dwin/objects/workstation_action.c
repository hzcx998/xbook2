#include <dwin/workstation.h>
#include <dwin/hal.h>

/**
 * idle layer is lowest layer
 */
void dwin_workstation_idle_layer_init(dwin_workstation_t *station)
{
    dwin_layer_t *layer;
    layer = dwin_layer_create(station->width, station->height);
    dwin_assert(layer != NULL);
    dwin_layer_change_priority(layer, DWIN_LAYER_PRIO_DESKTOP);

    dwin_layer_draw_rect(layer, 0, 0, layer->width, layer->height, 0xffffffff);

    dwin_workstation_add_layer(station, layer);

    dwin_layer_move(layer, 0, 0);
    dwin_layer_zorder(layer, 0);
    
    station->idle_layer = layer;
}

void dwin_workstation_mouse_layer_init(dwin_workstation_t *station)
{
    dwin_layer_t *layer;
    layer = dwin_layer_create(DWIN_WORKSTATION_MOUSE_W, DWIN_WORKSTATION_MOUSE_H);
    dwin_assert(layer != NULL);
    dwin_layer_change_priority(layer, DWIN_LAYER_PRIO_TOPEST);
    
    dwin_layer_draw_rect(layer, 0, 0, layer->width, layer->height, 0xff808080);

    dwin_workstation_add_layer(station, layer);

    dwin_layer_move(layer, station->width / 2, station->height / 2);
    dwin_layer_zorder(layer, 10);
    
    dwin_hal->mouse->parent.x = layer->x;
    dwin_hal->mouse->parent.y = layer->y;
    station->mouse_layer = layer;
}
