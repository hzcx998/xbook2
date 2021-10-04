#ifndef _DWIN_LAYER_H
#define _DWIN_LAYER_H

#include <dwin/dwin_config.h>

/* max number of layers */
#define DWIN_LAYER_NR   128

#define DWIN_LAYER_BPP  4

#define DWIN_LAYER_WIDTH_MAX   1920
#define DWIN_LAYER_HEIGHT_MAX  1080

#define DWIN_LAYER_BUF_SZ(w, h) ((w) * (h) * DWIN_LAYER_BPP)

struct dwin_layer
{
    list_t list;
    list_t global_list;
    uint8_t *buffer;
    uint32_t id;
    uint32_t width;
    uint32_t height;
    int x;
    int y;
    int z;
    void *workstation;
};
typedef struct dwin_layer dwin_layer_t;

void dwin_layer_init(void);
dwin_layer_t *dwin_layer_create(uint32_t width, uint32_t height);
int dwin_layer_destroy(dwin_layer_t *layer);
int dwin_layer_delete(dwin_layer_t *layer);

void dwin_layer_flush(dwin_layer_t *layer, int left, int top, int right, int buttom);
void dwin_layer_zorder(dwin_layer_t *layer, int z);

int dwin_layer_resize(dwin_layer_t *layer, int x, int y, uint32_t width, uint32_t height);

int dwin_layer_alloc_id(void);
int dwin_layer_free_id(int id);
void dwin_layer_id_init(void);

#endif   /* _DWIN_LAYER_H */
