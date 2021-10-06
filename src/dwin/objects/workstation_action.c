#include <dwin/workstation.h>
#include <dwin/hal.h>

/**
 * idle layer is lowest layer
 */
void dwin_workstation_idle_layer_init(dwin_workstation_t *station)
{
    dwin_layer_t *layer;
    layer = dwin_layer_create(station->width, station->height, 0);
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
    layer = dwin_layer_create(DWIN_WORKSTATION_MOUSE_W, DWIN_WORKSTATION_MOUSE_H, DWIN_LAYER_FLAG_NOMSG);
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

dwin_layer_t *dwin_workstation_get_lowest_layer(dwin_workstation_t *station)
{
    dwin_layer_t *layer;

    dwin_critical_t crit;
    dwin_enter_critical(crit);

    int lv;
    for (lv = DWIN_LAYER_PRIO_DESKTOP; lv <= DWIN_LAYER_PRIO_TOPEST; lv++)
    {
        list_for_each_owner (layer, &station->priority_list_head[lv], list)
        {
            dwin_leave_critical(crit);
            return layer;
        }
    }
    
    dwin_leave_critical(crit);
    return NULL;
}

dwin_layer_t *dwin_workstation_get_topest_layer(dwin_workstation_t *station)
{
    dwin_layer_t *layer;

    dwin_critical_t crit;
    dwin_enter_critical(crit);

    int lv;
    for (lv = DWIN_LAYER_PRIO_TOPEST; lv >= DWIN_LAYER_PRIO_DESKTOP; lv--)
    {
        list_for_each_owner_reverse (layer, &station->priority_list_head[lv], list)
        {    
            dwin_leave_critical(crit);
            return layer;
        }
    }
    
    dwin_leave_critical(crit);
    return NULL;
}

dwin_layer_t *dwin_workstation_get_lowest_layer_priority(dwin_workstation_t *station, int priority)
{
    if (priority >= DWIN_LAYER_PRIO_DESKTOP && priority <= DWIN_LAYER_PRIO_TOPEST)
    {
        dwin_layer_t *layer;
        
        dwin_critical_t crit;
        dwin_enter_critical(crit);

        list_for_each_owner (layer, &station->priority_list_head[priority], list)
        {        
            dwin_leave_critical(crit);
            return layer;
        }
        
        dwin_leave_critical(crit);
    }
    return NULL;
}

dwin_layer_t *dwin_workstation_get_topest_layer_priority(dwin_workstation_t *station, int priority)
{
    if (priority >= DWIN_LAYER_PRIO_DESKTOP && priority <= DWIN_LAYER_PRIO_TOPEST)
    {
        dwin_layer_t *layer;

        dwin_critical_t crit;
        dwin_enter_critical(crit);

        list_for_each_owner_reverse (layer, &station->priority_list_head[priority], list)
        {        
            dwin_leave_critical(crit);
            return layer;
        }

        dwin_leave_critical(crit);
    }
    return NULL;
}

void dwin_workstation_switch_hover_layer(dwin_workstation_t *station, dwin_layer_t *layer)
{
    if (station->hover_layer != layer)
    {
        if (layer)  /* enter new layer */
        {
            dwin_log(DWIN_TAG "Enter layer %d\n", layer->id);
        }
        if (station->hover_layer)  /* leave old layer */
        {
            dwin_log(DWIN_TAG "Leave layer %d\n", station->hover_layer->id);
        }

        station->hover_layer = layer;
    }
}

void dwin_workstation_switch_focus_layer(dwin_workstation_t *station, dwin_layer_t *layer)
{
    if (station->focus_layer != layer)
    {
        if (layer)  /* new layer get focus  */
        {
            dwin_log(DWIN_TAG "Get focus on layer %d\n", layer->id);
        }
        if (station->focus_layer)  /* old layer lose focus */
        {
            dwin_log(DWIN_TAG "Lose focus on layer %d\n", station->focus_layer->id);
        }

        station->focus_layer = layer;
    }
}
