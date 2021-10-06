#ifndef _DWIN_LAYER_H
#define _DWIN_LAYER_H

#include <dwin/dwin_config.h>
#include <dwin/buffer.h>
#include <dwin/objects.h>
#include <dwin/message.h>

/* max number of layers */
#define DWIN_LAYER_NR   128

#define DWIN_LAYER_BPP  4

#define DWIN_LAYER_WIDTH_MAX   1920
#define DWIN_LAYER_HEIGHT_MAX  1080

#define DWIN_LAYER_BUF_SZ(w, h) ((w) * (h) * DWIN_LAYER_BPP)

/* default message count */
#define DWIN_LAYER_MSG_CNT  64

#define DWIN_LAYER_ID_UNKNOWN  0

enum dwin_layer_priority
{
    DWIN_LAYER_PRIO_DESKTOP = 0,
    DWIN_LAYER_PRIO_WINDOW,
    DWIN_LAYER_PRIO_PANEL,
    DWIN_LAYER_PRIO_WINDOW_HIGH,
    DWIN_LAYER_PRIO_PANEL_HIGH,
    DWIN_LAYER_PRIO_TOPEST,
    DWIN_LAYER_PRIO_NR,
};
typedef enum dwin_layer_priority dwin_layer_priority_t; 

#define DWIN_LAYER_FLAG_NOMSG   1   /* layer no message */

struct dwin_layer
{
    list_t list;
    list_t global_list;
    uint8_t *buffer;
    uint32_t id;
    uint32_t flags;
    int width;
    int height;
    int x;
    int y;
    int z;
    enum dwin_layer_priority priority;
    void *workstation;
    struct dwin_msgpool *msgpool;
};
typedef struct dwin_layer dwin_layer_t;

int dwin_layer_alloc_id(void);
int dwin_layer_free_id(int id);
void dwin_layer_id_init(void);

void dwin_layer_init(void);
dwin_layer_t *dwin_layer_create(uint32_t width, uint32_t height, int flags);
int dwin_layer_destroy(dwin_layer_t *layer);
int dwin_layer_delete(dwin_layer_t *layer);

void dwin_layer_flush(dwin_layer_t *layer, int left, int top, int right, int buttom);
void dwin_layer_zorder(dwin_layer_t *layer, int z);
int dwin_layer_move(dwin_layer_t *layer, int x, int y);
int dwin_layer_resize(dwin_layer_t *layer, int x, int y, uint32_t width, uint32_t height);
int dwin_layer_change_priority(dwin_layer_t *layer, dwin_layer_priority_t priority);

int dwin_layer_recv_message(dwin_layer_t *layer, void *msg, int flags);
int dwin_layer_send_message(dwin_layer_t *layer, void *msg, int flags);

int dwin_layer_add_flags(dwin_layer_t *layer, uint32_t flags);
int dwin_layer_del_flags(dwin_layer_t *layer, uint32_t flags);

/* draw */
int dwin_layer_draw_rect(dwin_layer_t *layer, int x, int y, uint32_t w, uint32_t h, uint32_t color);
#define dwin_layer_putpixel(layer, x, y, color) ((uint32_t *)(layer)->buffer)[(y) * (layer)->width + (x)] = (color)
void dwin_layer_bitblt(dwin_layer_t *layer, int x, int y,
        struct dwin_buffer *buf, dwin_rect_t *rect);

#endif   /* _DWIN_LAYER_H */
