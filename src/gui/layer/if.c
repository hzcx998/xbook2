#include <gui/layer.h>
#include <gui/draw.h>
#include <gui/shape.h>
#include <gui/screen.h>
#include <xbook/mutexlock.h>
#include <xbook/task.h>
#include <string.h>

DEFINE_MUTEX_LOCK(layer_refresh_lock);

int sys_new_layer(int x, int y, uint32_t width, uint32_t height)
{
    if (!width || !height)
        return -1;
    layer_t *l = create_layer(width, height);
    if (l == NULL) {
        return -1;
    }
    //layer_draw_rect_fill(l, 0, 0, l->width, l->height, COLOR_WHITE);
    layer_set_xy(l, x, y);
    l->extension = current_task;
    return l->id;
}

int sys_layer_z(int id, int z)
{
    if (id < 0)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    layer_set_z(l, z);
    return 0;
}

int sys_layer_set_flags(int id, uint32_t flags)
{
    if (id < 0)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    layer_set_flags(l, flags);
    return 0;
}

int sys_layer_move(int id, int x, int y)
{
    if (id < 0)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    layer_set_xy(l, x, y);
    return 0;
}

int sys_del_layer(int id)
{
    if (id < 0)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    l->extension = NULL;
    destroy_layer(l);
    
    return 0;
}

int sys_layer_outp(int id, gui_point_t *p, uint32_t color)
{
    if (id < 0 || !p)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    return layer_put_point(l, p->x, p->y, color);
}

int sys_layer_inp(int id, gui_point_t *p, uint32_t *color)
{
    if (id < 0 || !p || !color)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    return layer_get_point(l, p->x, p->y, color);
}

int sys_layer_line(int id, gui_line_t *p, uint32_t color)
{
    if (id < 0 || !p)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    layer_draw_line(l, p->x0, p->y0, p->x1, p->y1, color);
    return 0;
}

int sys_layer_rect(int id, gui_rect_t *p, uint32_t color)
{
    if (id < 0 || !p)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    layer_draw_rect(l, p->x, p->y, p->width, p->height, color);
    return 0;
}

int sys_layer_rect_fill(int id, gui_rect_t *p, uint32_t color)
{
    if (id < 0 || !p)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    layer_draw_rect_fill(l, p->x, p->y, p->width, p->height, color);
    return 0;
}

int sys_layer_refresh(int id, gui_region_t *p)
{
    if (id < 0 || !p)
        return -1; 
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    mutex_lock(&layer_refresh_lock);

    layer_refresh(l, p->left, p->top, p->right, p->bottom);
    mutex_unlock(&layer_refresh_lock);
    return 0;
}

int sys_layer_set_region(int id, int type, gui_region_t *rg)
{
    if (id < 0 || !rg)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    if (type == LAYER_REGION_DRAG) {
        l->drag_rg = *rg;
    } else if (type == LAYER_REGION_RESIZE) {
        l->resize_rg = *rg;
    } else if (type == LAYER_REGION_RESIZEMIN) {
        l->resizemin_rg = *rg;
    }
    return 0;
}

int sys_layer_set_focus(int ly)
{
    layer_t *layer = layer_find_by_id(ly);
    if (layer == NULL)
        return -1;
    
    layer_set_focus(layer);
    return 0;
}

int sys_layer_get_focus()
{
    layer_t *layer = layer_get_focus();
    if (layer)
        return layer->id;
    else 
        return -1;
}

int sys_layer_set_win_top(int id)
{
    layer_t *layer = layer_find_by_id(id);
    if (layer == NULL)
        return -1;
    
    layer_set_win_top(layer);
    return 0;
}

int sys_layer_get_win_top()
{
    layer_t *layer = layer_get_win_top();
    if (layer) {
        return layer->z;
    } else 
        return -1;
}

int sys_layer_set_desktop(int id)
{
    layer_t *layer = layer_find_by_id(id);
    if (layer == NULL)
        return -1;
    
    layer_set_desktop(layer);
    return 0;
}

int sys_layer_get_desktop()
{
    layer_t *layer = layer_get_win_top();
    if (layer)
        return layer->id;
    else 
        return -1;
}

int sys_layer_resize(int id, gui_rect_t *rect)
{
    if (id < 0 || !rect)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    return layer_try_resize(l, rect);
}

int sys_layer_focus(int id)
{
    if (id < 0)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    return layer_try_focus(l);
}

int sys_layer_focus_win_top()
{
    return layer_focus_win_top();
}

int sys_layer_sync_bitmap(int lyid, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region)
{
    if (lyid < 0)
        return -1;
    layer_t *l = layer_find_by_id(lyid);
    if (l == NULL) {
        return -1;
    }
    //printk("b ");
    layer_sync_bitmap(l, rect, bitmap, region);
    
    mutex_lock(&layer_refresh_lock);
    layer_refresh_rect(l, rect->x, rect->y, rect->width, rect->height);
    mutex_unlock(&layer_refresh_lock);
 
    return 0;
}

/**
 * 扩展的时候不刷新，需要异步刷新
 */
int sys_layer_sync_bitmap_ex(int lyid, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region)
{
    if (lyid < 0)
        return -1;
    layer_t *l = layer_find_by_id(lyid);
    if (l == NULL) {
        return -1;
    }
    layer_sync_bitmap(l, rect, bitmap, region);
    return 0;
}

/**
 * 扩展的时候不刷新，需要异步刷新
 */
int sys_layer_copy_bitmap(int lyid, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region)
{
    if (lyid < 0)
        return -1;
    layer_t *l = layer_find_by_id(lyid);
    if (l == NULL) {
        return -1;
    }
    layer_copy_bitmap(l, rect, bitmap, region);
    return 0;
}

int sys_gui_get_icon(int lyid, char *path, uint32_t len)
{
    if (lyid < 0)
        return -1;
    layer_t *l = layer_find_by_id(lyid);
    if (l == NULL) {
        return -1;
    }
    if (l->ext_buf0 && path) {  /* 扩展缓冲区0 */
        memcpy(path, l->ext_buf0, min(len, MAX_PATH));
        /* 释放缓冲区 */
        kfree(l->ext_buf0);
        l->ext_buf0 = NULL;
        return 0;
    }
    return -1;
}

int sys_gui_set_icon(int lyid, char *path, uint32_t len)
{
    if (lyid < 0)
        return -1;
    layer_t *l = layer_find_by_id(lyid);
    if (l == NULL) {
        return -1;
    }
    if (l->ext_buf0 || !path || !len)
        return -1;
    l->ext_buf0 = kmalloc(MAX_PATH);
    if (l->ext_buf0 == NULL)
        return -1;
    memset(l->ext_buf0, 0, MAX_PATH);
    memcpy(l->ext_buf0, path, min(len, MAX_PATH));
    return 0;
}
