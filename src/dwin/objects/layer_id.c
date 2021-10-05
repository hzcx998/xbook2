#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/layer.h>

#define DWIN_LAYER_ID_OFF 1

/* layer id manage */
static uint8_t layer_id_map[DWIN_LAYER_NR / 8];
static bitmap_t dwin_layer_id;

int dwin_layer_alloc_id(void)
{
    long id = bitmap_scan(&dwin_layer_id, 1);
    if (id == -1)
    {
        return -1;
    }
    bitmap_set(&dwin_layer_id, id, 1);
    return id + DWIN_LAYER_ID_OFF;
}

int dwin_layer_free_id(int id)
{
    id -= DWIN_LAYER_ID_OFF;
    
    if (id < 0 || id >= dwin_layer_id.byte_length * 8)
    {
        return -1;
    }

    bitmap_set(&dwin_layer_id, id, 0);
    return 0;
}

void dwin_layer_id_init(void)
{
    memset(layer_id_map, 0, sizeof(layer_id_map));
    dwin_layer_id.bits = layer_id_map;
    dwin_layer_id.byte_length = DWIN_LAYER_NR / 8;
    bitmap_init(&dwin_layer_id);
}
