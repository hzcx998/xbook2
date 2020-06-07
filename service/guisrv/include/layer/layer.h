#ifndef __GUISRV_LAYER_H__
#define __GUISRV_LAYER_H__

#include <stdint.h>
#include <sys/list.h>
#include "color.h"

typedef struct _layer {
    int x, y, z;                /* 图层的位置(x, y, z) */
    int width, height;          /* 图层的宽高 */
    GUI_COLOR *buffer;          /* 图层缓冲区 */
    list_t list;                /* 在显示图层中的一个节点 */
    list_t global_list;         /* 在全局图层链表中的一个节点 */
    list_t widget_list_head;    /* 子控件链表 */
    void *extension;            /* 图层拓展 */
} layer_t;

extern layer_t *layer_topest;


int guisrv_init_layer();

layer_t *create_layer(int width, int height);
int destroy_layer(layer_t *layer);
void layer_set_xy(layer_t *layer, int x, int y);
void layer_set_z(layer_t *layer, int z);
void layer_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1);
layer_t *layer_get_by_z(int z);
void layer_refresh(layer_t *layer, int left, int top, int right, int buttom);

#endif  /* __GUISRV_WINDOW_LAYER_H__ */
