#include "xgui_view.h"
#include "xgui_hal.h"
#include "xgui_misc.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static LIST_HEAD(view_show_list_head);
static LIST_HEAD(view_global_list_head);
static uint16_t *view_id_map;
/* 顶层图层的Z轴 */
int top_view_z = -1;    

xgui_view_t *xgui_view_new(int x, int y, int width, int height)
{
    xgui_view_t *view = malloc(sizeof(xgui_view_t));
    if (view == NULL) {
        printf("malloc for view failed!\n");
        return NULL;
    }
    view->section = xgui_section_new(width, height);
    if (!view->section) {
        printf("new section failed!\n");
        free(view);
        return NULL;
    }
    view->x = x;
    view->y = y;
    view->z = -1;
    view->width = width;
    view->height = height;
    /* 添加到全局链表上，以备显示时加入显示链表 */
    //list_add(&view->list, &view_global_list_head);
    return view;
}

int xgui_view_put(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (xgui_section_put(view->section) < 0)
        return -1;
    list_del_init(&view->list);
    free(view);
    return 0;
}

static int xgui_view_count_z(xgui_view_t *view)
{
    if (!view)
        return -1;
    int z = 0;
    xgui_view_t *tmp;
    list_for_each_owner (tmp, &view_show_list_head, list) {
        if (tmp == view) {
            return z;
        }
        z++;
    }
    return z;
}

static void xgui_view_refresh_map(int left, int top, int right, int buttom, int z0)
{
    int view_left, xgui_view_top, view_right, view_buttom;
    int screen_x, screen_y;
    int view_x, view_y;

    int screen_w = xgui_screen_get_width();
    int screen_h = xgui_screen_get_height();
    
    /* 修复需要刷新的区域 */
    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > screen_w)
        right = screen_w;
	if (buttom > screen_h)
        buttom = screen_h;
    
    xgui_view_t *view;
    xgui_color_t *colors;
    int z = 0;
    /* 刷新高度为[z0-top]区间的图层 */
    list_for_each_owner (view, &view_show_list_head, list) {
        if (z >= z0) {
            /* 获取刷新范围 */
            view_left = left - view->x;
            xgui_view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (xgui_view_top < 0)
                xgui_view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            colors = (xgui_color_t *)view->section->addr;
            for(view_y = xgui_view_top; view_y < view_buttom; view_y++){
                screen_y = view->y + view_y;
                if (screen_y < 0)
                    continue;
                if (screen_y >= screen_h)
                    break;
                for(view_x = view_left; view_x < view_right; view_x++){
                    screen_x = view->x + view_x;
                    if (screen_x < 0)
                        continue;
                    if (screen_x >= screen_w)
                        break;
                       /* 不是全透明的，就把视图标识写入到映射表中 */
                    if ((colors[view_y * view->width + view_x] >> 24) & 0xff) {
                        view_id_map[(screen_y * screen_w + screen_x)] = z;
                    }
                }
            }
        }
        z++;
    }
}

/**
 * view_set_z - 设置图层的Z轴深度
 * @view: 图层
 * @z: z轴
 * 
 * Z轴轴序：
 *      可显示图层的轴序从0开始排序，一直递增。
 *      设置图层Z轴时，如果小于0，就把图层隐藏。
 *      如果图层没有存在于链表中，就是插入一个新图层。
 *      如果图层已经在链表中，那么就是调整一个图层的位置。
 *      如果设置图层小于0，为负，那么就是要隐藏图层
 * 
 * 调整Z序时需要刷新图层：
 *      当调整到更低的图层时，如果是隐藏图层，那么就刷新最底层到原图层Z的下一层。
 *      不是隐藏图层的话，刷新当前图层上一层到原来图层。
 *      当调整到更高的图层时，就只刷新新图层的高度那一层。
 * @return: 成功返回0，失败返回-1
 */
void xgui_view_set_z(xgui_view_t *view, int z)
{
    xgui_view_t *tmp = NULL;
    xgui_view_t *old_view = NULL;
    int old_z = view->z;
    if (list_find(&view->list, &view_show_list_head)) {
        if (z >= 0) {
            if (z > top_view_z) {
                z = top_view_z;
            }
            if (z == top_view_z) {
                list_del_init(&view->list);
                list_for_each_owner (tmp, &view_show_list_head, list) {
                    if (tmp->z > view->z) {
                        tmp->z--;
                    }
                }
                view->z = z;
                list_add_tail(&view->list, &view_show_list_head);
                /* 刷新新图层[z, z] */
                xgui_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
                xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
            } else {    /* 不是最高图层，那么就和其它图层交换 */
                if (z > view->z) { /* 如果新高度比原来的高度高 */
                    /* 先从链表中移除 */
                    list_del_init(&view->list);
                    /* 把位于旧图层高度和新图层高度之间（不包括旧图层，但包括新图层高度）的图层下降1层 */
                    list_for_each_owner (tmp, &view_show_list_head, list) {
                        if (tmp->z > view->z && tmp->z <= z) {
                            if (tmp->z == z) {
                                old_view = tmp;
                            }
                            tmp->z--;
                        }
                    }
                    assert(old_view);
                    view->z = z;
                    list_add_after(&view->list, &old_view->list);
                    /* 刷新新图层[z, z] */
                    xgui_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
                    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
                } else if (z < view->z) { /* 如果新高度比原来的高度低 */
                    list_del_init(&view->list);
                    /* 把位于旧图层高度和新图层高度之间（不包括旧图层，但包括新图层高度）的图层上升1层 */
                    list_for_each_owner (tmp, &view_show_list_head, list) {
                        if (tmp->z < view->z && tmp->z >= z) {
                            if (tmp->z == z) {  /* 记录原来为与新图层这个位置的图层 */
                                old_view = tmp;
                            }
                            tmp->z++; /* 等上一步判断图层高度后，再增加图层高度 */
                        }
                    }
                    assert(old_view);  
                    view->z = z;
                    list_add_before(&view->list, &old_view->list);
                    /* 刷新新图层[z + 1, old z] */
                    xgui_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z + 1);
                    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z + 1, old_z);
                }
            }
        } else { /* 小于0就是要隐藏起来的图层 */
            list_del_init(&view->list);
            if (top_view_z > old_z) {  /* 旧图层必须在顶图层下面 */
                /* 把位于当前图层后面的图层的高度都向下降1 */
                list_for_each_owner (tmp, &view_show_list_head, list) {
                    if (tmp->z > view->z) {
                        tmp->z--;
                    }
                }   
            }
            /* 由于隐藏了一个图层，那么，图层顶层的高度就需要减1 */
            top_view_z--;
            /* 刷新图层, [0, view->z - 1] */
            xgui_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, 0);
            xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, 0, old_z - 1);
            view->z = -1;  /* 隐藏图层后，高度变为-1 */
        }
    } else {    /* 插入新图层 */
        if (z >= 0) {
            if (z > top_view_z) {
                top_view_z++;
                z = top_view_z;
            } else {
                top_view_z++;
            }
            /* 如果新高度就是最高的图层，就直接插入到图层队列末尾 */
            if (z == top_view_z) {
                view->z = z;
                list_add_tail(&view->list, &view_show_list_head);
                /* 刷新新图层[z, z] */
                xgui_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
                xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
            } else {
                /* 查找和当前图层一样高度的图层 */
                list_for_each_owner(tmp, &view_show_list_head, list) {
                    if (tmp->z == z) {
                        old_view = tmp;
                        break;
                    }
                }
                tmp = NULL;
                assert(old_view);
                /* 把后面的图层的高度都+1 */
                list_for_each_owner(tmp, &view_show_list_head, list) {
                    if (tmp->z >= old_view->z) { 
                        tmp->z++;
                    }
                }
                view->z = z;
                /* 插入到旧图层前面 */
                list_add_before(&view->list, &old_view->list);
                /* 刷新新图层[z, z] */
                xgui_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
                xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
            }
        }
        /* 小于0就是要隐藏起来的图层，但是由于不在图层链表中，就不处理 */
    }
}

int xgui_view_show(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &view_global_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add_tail(&view->list, &view_show_list_head);
    // view->z = xgui_view_count_z(view);
    xgui_view_refresh_map(view->x, view->y, view->x + view->width, 
        view->y + view->height, view->z);
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, view->z, view->z);
    return 0;
}

int xgui_view_hide(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &view_show_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add(&view->list, &view_global_list_head);
    view->z = -1;
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, 0, xgui_view_get_top()->z);
    return 0;
}

int xgui_view_move_to_top(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &view_show_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add_tail(&view->list, &view_show_list_head);
    view->z = xgui_view_count_z(view);
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, view->z, xgui_view_get_top()->z);
    return 0;
}

int xgui_view_move_to_bottom(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &view_show_list_head)) {
        return -1;
    }
    list_del_init(&view->list);
    list_add(&view->list, &view_show_list_head);
    view->z = xgui_view_count_z(view);
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, 0, xgui_view_get_top()->z);
    return 0;
}

int xgui_view_move_under_top(xgui_view_t *view)
{
    if (!view)
        return -1;
    if (!list_find(&view->list, &view_show_list_head) || list_length(&view_show_list_head) < 2) {
        return -1;
    }
    list_del_init(&view->list);
    xgui_view_t *top_view = xgui_view_get_top();
    list_add_before(&view->list, &top_view->list);
    view->z = xgui_view_count_z(view);
    xgui_view_refresh_by_z(view->x, view->y, view->x + view->width, 
        view->y + view->height, view->z, xgui_view_get_top()->z);
    return 0;
}

int xgui_view_slide(xgui_view_t *view, int x, int y)
{
    if (!view)
        return -1;
    int old_x = view->x;
    int old_y = view->y;
    view->x = x;
    view->y = y;
    if (view->z >= 0) {
        int x0, y0, x1, y1;
        x0 = min(old_x, x);
        y0 = min(old_y, y);
        x1 = max(old_x + view->width, x + view->width);
        y1 = max(old_y + view->height, y + view->height);
        xgui_view_refresh_map(x0, y0, x1, y1, 0);
        xgui_view_refresh_by_z(x0, y0, x1, y1, 0, view->z);
    }
    return 0;
}

xgui_view_t *xgui_view_get_top()
{
    xgui_view_t *view = list_last_owner_or_null(&view_show_list_head, xgui_view_t, list);
    return view;
}

xgui_view_t *xgui_view_get_bottom()
{
    xgui_view_t *view = list_first_owner_or_null(&view_show_list_head, xgui_view_t, list);
    return view;
}

int xgui_view_refresh_one(xgui_view_t *view)
{
    if (!view)
        return -1;
    xgui_bitmap_t bitmap;
    bitmap.region.x = 0;
    bitmap.region.y = 0;
    bitmap.region.w = view->width;
    bitmap.region.h = view->height;
    bitmap.width = view->width;
    bitmap.height = view->height;
    bitmap.colors = (xgui_color_t *)view->section->addr;
    xgui_screen_write_bitmap(view->x, view->y, &bitmap);
    return 0;
}

void xgui_view_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1)
{
    int view_left, xgui_view_top, view_right, view_buttom;

    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > xgui_screen_get_width() - 1)
        right = xgui_screen_get_width() - 1;
	if (buttom > xgui_screen_get_height() - 1)
        buttom = xgui_screen_get_height() - 1;
    
    int vx, vy;
    int sx, sy;
    
    int z = 0;
    xgui_color_t color;
    xgui_color_t *buf;
    xgui_view_t *view;
    list_for_each_owner (view, &view_show_list_head, list) {
        if (z >= z0 && z <= z1) {
            view_left = left - view->x;
            xgui_view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (xgui_view_top < 0)
                xgui_view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            for (vy = xgui_view_top; vy < view_buttom; vy++) {
                sy = view->y + vy;
                for (vx = view_left; vx < view_right; vx++) {
                    sx = view->x + vx;
                    if (view_id_map[sy * xgui_screen_get_width() + sx] == z) {
                        buf = (xgui_color_t *)view->section->addr;
                        color = buf[vy * view->width + vx];
                        xgui_screen_write_pixel(sx, sy, color);
                    }
                }
            }
        }
        z++;
    }
}

void xgui_view_refresh(xgui_view_t *view, int left, int top, int right, int buttom)
{
    if (view->z >= 0) {
        xgui_view_refresh_map(view->x + left, view->y + top, view->x + right,
            view->y + buttom, view->z);
        xgui_view_refresh_by_z(view->x + left, view->y + top, view->x + right,
            view->y + buttom, view->z, view->z);
    }
}

#define view_self_refresh(view) xgui_view_refresh((view), 0, 0, view->width, view->height)

/**
 * button: 桌面
 * 一般窗口
 * 系统预留窗口
 * top: 鼠标
*/

int xgui_view_init()
{
    size_t id_map_size = xgui_screen_get_width() * xgui_screen_get_width() * sizeof(uint16_t);
    view_id_map = malloc(id_map_size);
    if (!view_id_map) {
        printf("malloc for view id map failed!\n");
        return -1;
    }
    memset(view_id_map, 0, id_map_size);

    xgui_view_t *view0 = xgui_view_new(0, 0, 800, 600);
    xgui_section_fill_rect(view0->section, XCOLOR_RED);
    xgui_view_set_z(view0, 0);
    view_self_refresh(view0);

    xgui_view_t *view = xgui_view_new(100, 100, 400, 300);
    xgui_section_fill_rect(view->section, XCOLOR_BLUE);
    xgui_view_set_z(view, 1);
    
    view_self_refresh(view);

    view = xgui_view_new(300, 300, 400, 300);
    xgui_section_fill_rect(view->section, XCOLOR_GREEN);
    xgui_view_set_z(view, 2);
    
    view_self_refresh(view);

    view = xgui_view_new(400, 300, 32, 32);
    xgui_section_fill_rect(view->section, XCOLOR_WHITE);
    xgui_view_set_z(view, 3);
    
    view_self_refresh(view);

    return 0;
}