#ifndef _GUI_LAYER_H
#define _GUI_LAYER_H

#include <stdint.h>
#include <list.h>
#include "color.h"
#include "shape.h"
#include "message.h"
#include <xbook/mutexlock.h>

/* 图层区域 */
enum {
    LAYER_REGION_DRAG = 1,      /* drag occur region */
    LAYER_REGION_RESIZE,        /* resize occur region */
    LAYER_REGION_RESIZEMIN,     /* resize minim region */
};

/* 图层标志 */
enum {
    LAYER_WINDOW    = (1 << 0),   /* 窗口标志 */
    LAYER_FLOAT     = (1 << 1),   /* 浮动块标志 */
    LAYER_FIXED     = (1 << 2),   /* 固定位置标志 */
    LAYER_EXT_BUF   = (1 << 3),   /* 扩展内存，使用缓冲区专用内存 */
    LAYER_EXT_BUF2  = (1 << 4),   /* 扩展内存，使用缓冲区专用内存，最大内存 */
};

typedef struct _layer {
    int id;                /* 图层id标识 */
    int x, y, z;                /* 图层的位置(x, y, z) */
    int width, height;          /* 图层的宽高 */
    uint32_t flags;              /* 图层标志 */
    GUI_COLOR *buffer;          /* 图层缓冲区 */
    list_t list;                /* 在显示图层中的一个节点 */
    list_t global_list;         /* 在全局图层链表中的一个节点 */
    list_t widget_list_head;    /* 子控件链表 */
    void *extension;            /* 图层拓展 */
    void *ext_buf0;             /* 拓展缓冲区 */
    gui_region_t drag_rg;       /* 拖拽区域 */
    gui_region_t resize_rg;     /* 调整大小区域 */
    gui_region_t resizemin_rg;  /* 最小的调整大小区域 */
    spinlock_t mutex;          /* 图层操作时的互斥 */
} layer_t;

int gui_init_layer();

layer_t *create_layer(int width, int height);
int destroy_layer(layer_t *layer);
void layer_clear(layer_t *layer);
void layer_set_xy(layer_t *layer, int x, int y);
void layer_set_z(layer_t *layer, int z);
void layer_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1);

void layer_refresh(layer_t *layer, int left, int top, int right, int buttom);
void layer_refresh_all();
void layer_refresh_rect(layer_t *layer, int x, int y, uint32_t width, uint32_t height);

void layer_refresh_under(layer_t *layer, int left, int top, int right, int buttom);
void layer_refresh_under_rect(layer_t *layer, int x, int y, uint32_t width, uint32_t height);

#define layer_flush(layer) layer_refresh((layer), 0, 0, layer->width, layer->height)

layer_t *layer_find_by_id(int id);
layer_t *layer_find_by_id_without_lock(int id);
layer_t *layer_find_by_z(int z);
layer_t *layer_find_by_extension(void *extension);

layer_t *layer_get_win_top();
int layer_set_win_top(layer_t *layer);

layer_t *layer_get_desktop();
int layer_set_desktop(layer_t *layer);

void layer_set_focus(layer_t *layer);
layer_t *layer_get_focus();

int layer_set_flags(layer_t *layer, uint32_t flags);

int layer_reset_size(layer_t *layer, int x, int y, uint32_t width, uint32_t height);

int layer_try_focus(layer_t *layer);
int layer_try_resize(layer_t *layer, gui_rect_t *prect);
int layer_focus_win_top();

void layer_sync_bitmap(layer_t *layer, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region);

static inline void layer_mutex_lock(layer_t *layer)
{
    spin_lock(&layer->mutex);
}

static inline void layer_mutex_unlock(layer_t *layer)
{
    spin_unlock(&layer->mutex);
}

int sys_new_layer(int x, int y, uint32_t width, uint32_t height);
int sys_layer_z(int id, int z);
int sys_layer_move(int id, int x, int y);
int sys_del_layer(int id);

int sys_layer_outp(int id, gui_point_t *p, uint32_t color);
int sys_layer_inp(int id, gui_point_t *p, uint32_t *color);
int sys_layer_line(int id, gui_line_t *p, uint32_t color);
int sys_layer_rect(int id, gui_rect_t *p, uint32_t color);
int sys_layer_rect_fill(int id, gui_rect_t *p, uint32_t color);
int sys_layer_refresh(int id, gui_region_t *p);

int gui_dispatch_key_msg(g_msg_t *msg);
int gui_dispatch_mouse_msg(g_msg_t *msg);
int gui_dispatch_target_msg(g_msg_t *msg);

int sys_layer_set_focus(int ly);
int sys_layer_get_focus();
int sys_layer_set_win_top(int id);
int sys_layer_get_win_top();
int sys_layer_set_region(int id, int type, gui_region_t *rg);

int sys_layer_set_flags(int id, uint32_t flags);

int sys_layer_resize(int id, gui_rect_t *rect);
int sys_layer_focus(int id);
int sys_layer_focus_win_top();

int sys_layer_set_desktop(int id);
int sys_layer_get_desktop();
int sys_layer_sync_bitmap(int lyid, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region);
int sys_layer_sync_bitmap_ex(int lyid, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region);
int sys_gui_get_icon(int lyid, char *path, uint32_t len);
int sys_gui_set_icon(int lyid, char *path, uint32_t len);

#endif  /* _GUI_LAYER_H */
