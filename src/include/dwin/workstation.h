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

#define DWIN_WORKSTATION_MOUSE_W 32
#define DWIN_WORKSTATION_MOUSE_H 32

typedef uint16_t dwin_layer_id_map_t;

struct dwin_workstation
{
    list_t global_list_head;
    list_t priority_list_head[DWIN_LAYER_PRIO_NR];  /* priority layer */
    int priority_topz[DWIN_LAYER_PRIO_NR];   /* topest Z */

    dwin_layer_t *idle_layer;
    dwin_layer_t *mouse_layer;

    dwin_layer_t *hover_layer;  /* hover layer */
    dwin_layer_t *focus_layer;  /* focus layer */
    
    int depth;          /* depth of per workstation */
    uint32_t width;     /* workstation width size */
    uint32_t height;    /* workstation height size */

    dwin_layer_id_map_t *id_map;   /* layer id map */

    /* flush layer function */
    void (*flush_bits) (dwin_layer_t *, int, int, int, int);
    void (*flush_map) (struct dwin_workstation *, int, int, int, int, int, int);  
    void (*flush_z) (struct dwin_workstation *, int, int, int, int, int, int, int);  
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

void dwin_workstation_idle_layer_init(dwin_workstation_t *station);
void dwin_workstation_mouse_layer_init(dwin_workstation_t *station);

dwin_layer_t *dwin_workstation_get_lowest_layer(dwin_workstation_t *station);
dwin_layer_t *dwin_workstation_get_topest_layer(dwin_workstation_t *station);

void dwin_workstation_switch_hover_layer(dwin_workstation_t *station, dwin_layer_t *layer);
void dwin_workstation_switch_focus_layer(dwin_workstation_t *station, dwin_layer_t *layer);

#endif   /* _DWIN_WORKSTATION_H */
