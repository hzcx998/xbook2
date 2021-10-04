#ifndef _DWIN_WORKSTATION_H
#define _DWIN_WORKSTATION_H

#include <dwin/dwin_config.h>
#include <dwin/layer.h>

/**
 * 0: basic workstation: desktop, window
 * 1: login workstation: login/logout
 * 2: reserved work station
 * 3: error mode workstation: error notition
 */
#define DWIN_WORKSTATION_NR 4

typedef uint16_t dwin_layer_id_map_t;

struct dwin_workstation
{
    list_t global_list_head;
    list_t show_list_head;
    int depth;          /* depth of per workstation */
    uint32_t width;     /* workstation width size */
    uint32_t height;    /* workstation height size */

    int topz;   /* topest Z */

    dwin_layer_id_map_t *id_map;   /* layer id map */

    /* flush layer function */
    void (*flush_bits) (dwin_layer_t *, int, int, int, int);    
    
    void (*flush_map) (struct dwin_workstation *, int, int, int, int, int);  
    void (*flush_z) (struct dwin_workstation *, int, int, int, int, int, int);  
};
typedef struct dwin_workstation dwin_workstation_t;

extern dwin_workstation_t *dwin_current_workstation;

void dwin_workstation_init(uint32_t width, uint32_t height);
void dwin_workstation_init_flush(dwin_workstation_t *workstation);

dwin_workstation_t *dwin_workstation_switch(int idx);
int dwin_workstation_has_layer(dwin_workstation_t *station, dwin_layer_t *layer);
int dwin_workstation_is_layer_shown(dwin_workstation_t *station, dwin_layer_t *layer);

int dwin_workstation_add_layer(dwin_workstation_t *station, dwin_layer_t *layer);
int dwin_workstation_del_layer(dwin_workstation_t *station, dwin_layer_t *layer);

dwin_workstation_t *dwin_workstation_get_by_depth(int depth);

void dwin_workstation_zorder_layer(dwin_workstation_t *station, dwin_layer_t *layer, int z);
int dwin_workstation_move_layer(dwin_workstation_t *station, dwin_layer_t *layer, int x, int y);

#endif   /* _DWIN_WORKSTATION_H */
