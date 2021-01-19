#include "xbrower_view.h"
#include "xbrower_hal.h"
#include "xbrower_misc.h"
#include "xbrower_render.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static LIST_HEAD(view_show_list_head);
static LIST_HEAD(view_global_list_head);
static uint16_t *view_id_map;
static int view_top_z = -1;    
static int view_next_id = 0;    

xbrower_view_t *xbrower_view_create(int x, int y, int width, int height)
{
    xbrower_view_t *view = malloc(sizeof(xbrower_view_t));
    if (view == NULL) {
        printf("malloc for view failed!\n");
        return NULL;
    }
    view->section = xbrower_section_create(width, height);
    if (!view->section) {
        printf("new section failed!\n");
        free(view);
        return NULL;
    }
    view->x = x;
    view->y = y;
    view->id = view_next_id++;
    view->z = -1;
    view->width = width;
    view->height = height;
    list_init(&view->list);
    list_add(&view->global_list, &view_global_list_head);
    return view;
}

xbrower_view_t *xbrower_view_find_by_id(int id)
{
    xbrower_view_t *view;
    list_for_each_owner (view, &view_global_list_head, global_list) {
        if (view->id == id)
            return view;
    }
    return NULL;
}

int xbrower_view_destroy(xbrower_view_t *view)
{
    if (!view)
        return -1;
    if (xbrower_section_destroy(view->section) < 0) {
        return -1;
    }
    if (list_find(&view->list, &view_show_list_head))
        list_del_init(&view->list);
    list_del_init(&view->global_list);
    free(view);
    return 0;
}

static void xbrower_view_refresh_map(int left, int top, int right, int buttom, int z0)
{
    int view_left, xbrower_view_top, view_right, view_buttom;
    int screen_x, screen_y;
    int view_x, view_y;

    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > xbrower_screen.width)
        right = xbrower_screen.width;
	if (buttom > xbrower_screen.height)
        buttom = xbrower_screen.height;
    
    xbrower_view_t *view;
    xbrower_color_t *colors;

    /* 刷新高度为[z0-top]区间的视图 */
    list_for_each_owner (view, &view_show_list_head, list) {
        if (view->z >= z0) {
            view_left = left - view->x;
            xbrower_view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (xbrower_view_top < 0)
                xbrower_view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            colors = (xbrower_color_t *)view->section->addr;
            for(view_y = xbrower_view_top; view_y < view_buttom; view_y++){
                screen_y = view->y + view_y;
                if (screen_y < 0)
                    continue;
                if (screen_y >= xbrower_screen.height)
                    break;
                for(view_x = view_left; view_x < view_right; view_x++){
                    screen_x = view->x + view_x;
                    if (screen_x < 0)
                        continue;
                    if (screen_x >= xbrower_screen.width)
                        break;
                       /* 不是全透明的，就把视图标识写入到映射表中 */
                    if ((colors[view_y * view->width + view_x] >> 24) & 0xff) {
                        view_id_map[(screen_y * xbrower_screen.width + screen_x)] = view->z;
                    }
                }
            }
        }
    }
}

static void __xbrower_view_adjust_by_z(xbrower_view_t *view, int z)
{
    xbrower_view_t *tmp;
    xbrower_view_t *old_view = NULL;
    int old_z = view->z;
    if (z > view_top_z) {
        z = view_top_z;
    }
    /* 先从链表中移除 */
    list_del_init(&view->list);
    if (z == view_top_z) {
        /* 其它视图降低高度 */
        list_for_each_owner (tmp, &view_show_list_head, list) {
            if (tmp->z > view->z) {
                tmp->z--;
            }
        }
        view->z = z;
        list_add_tail(&view->list, &view_show_list_head);
        /* 刷新新视图[z, z] */
        xbrower_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
        xbrower_refresh_view_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
    } else {    /* 不是最高视图，那么就和其它视图交换 */
        if (z > view->z) { /* 如果新高度比原来的高度高 */
            /* 把位于旧视图高度和新视图高度之间（不包括旧视图，但包括新视图高度）的视图下降1层 */
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
            /* 刷新新视图[z, z] */
            xbrower_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
            xbrower_refresh_view_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
        } else if (z < view->z) { /* 如果新高度比原来的高度低 */
            /* 把位于旧视图高度和新视图高度之间（不包括旧视图，但包括新视图高度）的视图上升1层 */
            list_for_each_owner (tmp, &view_show_list_head, list) {
                if (tmp->z < view->z && tmp->z >= z) {
                    if (tmp->z == z) {  /* 记录原来为与新视图这个位置的视图 */
                        old_view = tmp;
                    }
                    tmp->z++; /* 等上一步判断视图高度后，再增加视图高度 */
                }
            }
            assert(old_view);  
            view->z = z;
            list_add_before(&view->list, &old_view->list);
            /* 刷新新视图[z + 1, old z] */
            xbrower_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z + 1);
            xbrower_refresh_view_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z + 1, old_z);
        }
    }
}

static void __xbrower_view_hiden_by_z(xbrower_view_t *view, int z)
{
    int old_z = view->z;
    list_del_init(&view->list);
    if (view_top_z > old_z) {  /* 旧视图必须在顶视图下面 */
        /* 把位于当前视图后面的视图的高度都向下降1 */
        xbrower_view_t *tmp;
        list_for_each_owner (tmp, &view_show_list_head, list) {
            if (tmp->z > view->z) {
                tmp->z--;
            }
        }   
    }
    /* 由于隐藏了一个视图，那么，视图顶层的高度就需要减1 */
    view_top_z--;
    view->z = -1;  /* 隐藏视图后，高度变为-1 */
    /* 刷新视图, [0, view->z - 1] */
    xbrower_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, 0);
    xbrower_refresh_view_by_z(view->x, view->y, view->x + view->width, view->y + view->height, 0, old_z - 1);
    
}

static void __xbrower_view_show_by_z(xbrower_view_t *view, int z)
{
    xbrower_view_t *tmp;
    xbrower_view_t *old_view = NULL;
    if (z > view_top_z) {
        view_top_z++;
        z = view_top_z;
    } else {
        view_top_z++;
    }
    /* 如果新高度就是最高的视图，就直接插入到视图队列末尾 */
    if (z == view_top_z) {
        view->z = z;
        list_add_tail(&view->list, &view_show_list_head);
        /* 刷新新视图[z, z] */
        xbrower_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
        xbrower_refresh_view_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
    } else {
        /* 查找和当前视图一样高度的视图 */
        list_for_each_owner(tmp, &view_show_list_head, list) {
            if (tmp->z == z) {
                old_view = tmp;
                break;
            }
        }
        tmp = NULL;
        assert(old_view);
        /* 把后面的视图的高度都+1 */
        list_for_each_owner(tmp, &view_show_list_head, list) {
            if (tmp->z >= old_view->z) { 
                tmp->z++;
            }
        }
        view->z = z;
        /* 插入到旧视图前面 */
        list_add_before(&view->list, &old_view->list);
        /* 刷新新视图[z, z] */
        xbrower_view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
        xbrower_refresh_view_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
    }
}

/**
 * view_set_z - 设置视图的Z轴深度
 * @view: 视图
 * @z: z轴
 * 
 * Z轴轴序：
 *      可显示视图的轴序从0开始排序，一直递增。
 *      设置视图Z轴时，如果小于0，就把视图隐藏。
 *      如果视图没有存在于链表中，就是插入一个新视图。
 *      如果视图已经在链表中，那么就是调整一个视图的位置。
 *      如果设置视图小于0，为负，那么就是要隐藏视图
 * 
 * 调整Z序时需要刷新视图：
 *      当调整到更低的视图时，如果是隐藏视图，那么就刷新最底层到原视图Z的下一层。
 *      不是隐藏视图的话，刷新当前视图上一层到原来视图。
 *      当调整到更高的视图时，就只刷新新视图的高度那一层。
 * @return: 成功返回0，失败返回-1
 */
void xbrower_view_set_z(xbrower_view_t *view, int z)
{
    if (list_find(&view->list, &view_show_list_head)) {
        if (z >= 0) {
            __xbrower_view_adjust_by_z(view, z);
        } else { /* 小于0就是要隐藏起来的视图 */
            __xbrower_view_hiden_by_z(view, z);
        }
    } else {    /* 插入新视图 */
        if (z >= 0) {
            __xbrower_view_show_by_z(view, z);
        }
    }
}

int xbrower_view_move_to_top(xbrower_view_t *view)
{
    if (!view)
        return -1;
    xbrower_view_set_z(view, view_top_z);
    return 0;
}

int xbrower_view_move_to_bottom(xbrower_view_t *view)
{
    if (!view)
        return -1;
    xbrower_view_set_z(view, 0);
    return 0;
}

int xbrower_view_move_under_top(xbrower_view_t *view)
{
    if (!view)
        return -1;
    xbrower_view_set_z(view, view_top_z - 1);
    return 0;
}

int xbrower_view_move_upper_top(xbrower_view_t *view)
{
    if (!view)
        return -1;
    xbrower_view_set_z(view, view_top_z + 1);
    return 0;
}

int xbrower_view_hide(xbrower_view_t *view)
{
    if (!view)
        return -1;
    xbrower_view_set_z(view, -1);
    return 0;
}

int xbrower_view_show(xbrower_view_t *view)
{
    if (!view)
        return -1;
    xbrower_view_move_to_top(view);
    return 0;
}


int xbrower_view_set_xy(xbrower_view_t *view, int x, int y)
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
        xbrower_view_refresh_map(x0, y0, x1, y1, 0);
        xbrower_refresh_view_by_z(x0, y0, x1, y1, 0, view->z);
    }
    return 0;
}

xbrower_view_t *xbrower_view_get_top()
{
    xbrower_view_t *view = list_last_owner_or_null(&view_show_list_head, xbrower_view_t, list);
    return view;
}

xbrower_view_t *xbrower_view_get_bottom()
{
    xbrower_view_t *view = list_first_owner_or_null(&view_show_list_head, xbrower_view_t, list);
    return view;
}

void xbrower_refresh_view_by_z(int left, int top, int right, int buttom, int z0, int z1)
{
    int view_left, xbrower_view_top, view_right, view_buttom;

    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > xbrower_screen.width)
        right = xbrower_screen.width;
	if (buttom > xbrower_screen.height)
        buttom = xbrower_screen.height;
    
    int vx, vy;
    int sx, sy;
    
    xbrower_color_t color;
    xbrower_color_t *buf;
    xbrower_view_t *view;
    list_for_each_owner (view, &view_show_list_head, list) {
        if (view->z >= z0 && view->z <= z1) {
            view_left = left - view->x;
            xbrower_view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (xbrower_view_top < 0)
                xbrower_view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            for (vy = xbrower_view_top; vy < view_buttom; vy++) {
                sy = view->y + vy;
                for (vx = view_left; vx < view_right; vx++) {
                    sx = view->x + vx;
                    if (view_id_map[sy * xbrower_screen.width + sx] == view->z) {
                        buf = (xbrower_color_t *)view->section->addr;
                        color = buf[vy * view->width + vx];
                        xbrower_screen_write_pixel(sx, sy, color);
                    }
                }
            }
        }
    }
}

void xbrower_view_refresh(xbrower_view_t *view, int left, int top, int right, int buttom)
{
    if (view->z >= 0) {
        xbrower_view_refresh_map(view->x + left, view->y + top, view->x + right,
            view->y + buttom, view->z);
        xbrower_refresh_view_by_z(view->x + left, view->y + top, view->x + right,
            view->y + buttom, view->z, view->z);
    }
}

/**
 * button: 桌面
 * 一般窗口
 * 系统预留窗口
 * top: 鼠标
*/
int xbrower_view_init()
{
    size_t id_map_size = xbrower_screen.width * xbrower_screen.height * sizeof(uint16_t);
    view_id_map = malloc(id_map_size);
    if (!view_id_map) {
        printf("malloc for view id map failed!\n");
        return -1;
    }
    memset(view_id_map, 0, id_map_size);
    
    return 0;
}

void xbrower_view_exit()
{
    xbrower_view_t *view, *next;
    list_for_each_owner_safe (view, next, &view_global_list_head, global_list) {
        xbrower_view_destroy(view);
    }
    free(view_id_map);
}