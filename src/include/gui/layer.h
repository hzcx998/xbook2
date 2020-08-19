#ifndef _GUI_LAYER_H
#define _GUI_LAYER_H

#include <stdint.h>
#include <list.h>
#include "color.h"
#include "shape.h"
#include "message.h"

typedef struct _layer {
    int id;                /* 图层id标识 */
    int x, y, z;                /* 图层的位置(x, y, z) */
    int width, height;          /* 图层的宽高 */
    GUI_COLOR *buffer;          /* 图层缓冲区 */
    list_t list;                /* 在显示图层中的一个节点 */
    list_t global_list;         /* 在全局图层链表中的一个节点 */
    list_t widget_list_head;    /* 子控件链表 */
    void *extension;            /* 图层拓展 */
} layer_t;

extern layer_t *layer_topest;

int gui_init_layer();

layer_t *create_layer(int width, int height);
int destroy_layer(layer_t *layer);
void layer_set_xy(layer_t *layer, int x, int y);
void layer_set_z(layer_t *layer, int z);
void layer_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1);
layer_t *layer_get_by_z(int z);
void layer_refresh(layer_t *layer, int left, int top, int right, int buttom);
void layer_refresh_all();

#define layer_flush(layer) layer_refresh((layer), 0, 0, layer->width, layer->height)

layer_t *layer_find_by_id(int id);

int layer_get_win_top();
int layer_set_win_top(int top);

int sys_new_layer(int x, int y, uint32_t width, uint32_t height);
int sys_layer_z(int id, int z);
int sys_layer_move(int id, int x, int y);
int sys_del_layer(int id);

int sys_layer_outp(int id, gui_point_t *p, uint32_t color);
int sys_layer_inp(int id, gui_point_t *p, uint32_t *color);
int sys_layer_line(int id, gui_line_t *p, uint32_t color);
int sys_layer_rect(int id, gui_rect_t *p, uint32_t color);
int sys_layer_rect_fill(int id, gui_rect_t *p, uint32_t color);
int sys_layer_pixmap(int id, gui_pixmap_t *p);
int sys_layer_refresh(int id, gui_region_t *p);

int sys_gui_info(uint32_t *width, uint32_t *height, uint32_t *bpp);

#define sys_layer_get_win_top layer_get_win_top
#define sys_layer_set_win_top layer_set_win_top

int gui_dispatch_key_msg(g_msg_t *msg);
int gui_dispatch_mouse_msg(g_msg_t *msg);

int sys_layer_set_focus(int ly);
int sys_layer_get_focus();

#endif  /* _GUI_LAYER_H */
