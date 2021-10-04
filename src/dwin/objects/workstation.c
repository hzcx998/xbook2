#include <dwin/workstation.h>

dwin_workstation_t dwin_workstations[DWIN_WORKSTATION_NR];

dwin_workstation_t *dwin_current_workstation;

void dwin_workstation_init(uint32_t width, uint32_t height)
{
    int i;
    for (i = 0; i < DWIN_WORKSTATION_NR; i++)
    {
        dwin_workstations[i].depth = i;
        dwin_workstations[i].width = width;
        dwin_workstations[i].height = height;
        dwin_workstations[i].topz = -1;

        list_init(&dwin_workstations[i].show_list_head);
        list_init(&dwin_workstations[i].global_list_head);
        dwin_workstation_init_flush(&dwin_workstations[i]);
    }
    /* default select 0 */
    dwin_current_workstation = &dwin_workstations[0];
}

dwin_workstation_t *dwin_workstation_switch(int idx)
{
    if (idx < 0 || idx >= DWIN_WORKSTATION_NR)
    {
        return dwin_current_workstation;
    }
    dwin_workstation_t *old = dwin_current_workstation;
    dwin_current_workstation = &dwin_workstations[idx];
    return old;
}

/**
 * check workstation has layer
 * success return 1, failed return 0
 */
int dwin_workstation_has_layer(dwin_workstation_t *station, dwin_layer_t *layer)
{
    dwin_critical_t crit;
    dwin_enter_critical(crit);
    int found = list_find(&layer->global_list, &station->global_list_head);
    dwin_leave_critical(crit);
    return found;
}

int dwin_workstation_is_layer_shown(dwin_workstation_t *station, dwin_layer_t *layer)
{
    dwin_critical_t crit;
    dwin_enter_critical(crit);
    int found = list_find(&layer->list, &station->show_list_head);
    dwin_leave_critical(crit);
    return found;
}

int dwin_workstation_add_layer(dwin_workstation_t *station, dwin_layer_t *layer)
{
    /* check layer */
    if (station == NULL || layer == NULL)
    {
        return -1;
    }

    if (dwin_workstation_has_layer(station, layer) == 1)
    {
        dwin_log("layer %d had on workstation %d\n", layer->id, station->depth);
        return -1;
    }

    dwin_critical_t crit;
    dwin_enter_critical(crit);
    /* add to global layer list */
    list_add(&layer->global_list, &station->global_list_head);
    layer->workstation = station;
    dwin_leave_critical(crit);

    return 0;
}

int dwin_workstation_del_layer(dwin_workstation_t *station, dwin_layer_t *layer)
{
    /* check layer */
    if (station == NULL || layer == NULL)
    {
        return -1;
    }

    if (!dwin_workstation_has_layer(station, layer))
    {
        dwin_log("layer %d not on work station %d\n", layer->id, station->depth);
        return -1;
    }

    dwin_critical_t crit;
    dwin_enter_critical(crit);
    /* del from layer list */
    list_del(&layer->global_list);
    list_del(&layer->list);
    layer->workstation = NULL;
    dwin_leave_critical(crit);

    return 0;
}

dwin_workstation_t *dwin_workstation_get_by_depth(int depth)
{
    if (depth < 0 || depth >= DWIN_WORKSTATION_NR)
    {
        return NULL;
    }
    return &dwin_workstations[depth];
}
