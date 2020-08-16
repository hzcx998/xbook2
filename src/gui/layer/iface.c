#include <gui/layer.h>
#include <gui/draw.h>
#include <gui/shape.h>
#include <gui/screen.h>
#include <xbook/mutexlock.h>

DEFINE_MUTEX_LOCK(layer_mutex); /* 图层管理互斥 */

int sys_new_layer(int x, int y, uint32_t width, uint32_t height)
{
    if (!width || !height)
        return -1;
    mutex_lock(&layer_mutex);
    layer_t *l = create_layer(width, height);
    if (l == NULL) {
        mutex_unlock(&layer_mutex);
        return -1;
    }
    layer_draw_rect_fill(l, 0, 0, l->width, l->height, COLOR_WHITE);
    layer_set_xy(l, x, y);
    mutex_unlock(&layer_mutex);
    return l->id;
}

int sys_layer_z(int id, int z)
{
    if (id < 0)
        return -1;
    mutex_lock(&layer_mutex);
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        mutex_unlock(&layer_mutex);
        return -1;
    }
    layer_set_z(l, z);
    mutex_unlock(&layer_mutex);
    return 0;
}

int sys_layer_move(int id, int x, int y)
{
    if (id < 0)
        return -1;
    mutex_lock(&layer_mutex);
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        mutex_unlock(&layer_mutex);
        return -1;
    }
    layer_set_xy(l, x, y);
    mutex_unlock(&layer_mutex);
    return 0;
}

int sys_del_layer(int id)
{
    if (id < 0)
        return -1;
    mutex_lock(&layer_mutex);
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }

    destroy_layer(l);
    mutex_unlock(&layer_mutex);
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

int sys_layer_pixmap(int id, gui_pixmap_t *p)
{
     if (id < 0 || !p)
        return -1;
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    layer_draw_pixmap(l, p->rect.x, p->rect.y, p->rect.width, p->rect.height, p->pixles, p->bps);
    return 0;
}

int sys_layer_refresh(int id, gui_region_t *p)
{
     if (id < 0 || !p)
        return -1;
    mutex_lock(&layer_mutex);
    layer_t *l = layer_find_by_id(id);
    if (l == NULL) {
        return -1;
    }
    layer_refresh(l, p->left, p->top, p->right, p->bottom);
    mutex_unlock(&layer_mutex);
    return 0;
}

int sys_gui_info(uint32_t *width, uint32_t *height, uint32_t *bpp)
{
    if (width)
        *width = gui_screen.width;
    if (height)
        *height = gui_screen.height;
    if (bpp)
        *bpp = gui_screen.bpp;
    return 0;
}
