#include "drivers/view/view.h"
#include "drivers/view/hal.h"
#include "drivers/view/misc.h"
#include "drivers/view/render.h"
#include "drivers/view/msg.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/ioctl.h>

static LIST_HEAD(view_show_list_head);
static LIST_HEAD(view_global_list_head);
static uint16_t *view_id_map;
static int view_top_z = -1;    
static int view_next_id = 0;    
int view_last_x, view_last_y; // 上一个关闭的视图的位置

view_t *view_create(int x, int y, int width, int height)
{
    view_t *view = mem_alloc(sizeof(view_t));
    if (view == NULL) {
        keprint("view: malloc for view failed!\n");
        return NULL;
    }
    view->section = view_section_create(width, height);
    if (!view->section) {
        keprint("view: new section failed!\n");
        free(view);
        return NULL;
    }
    view->msgpool = msgpool_create(sizeof(view_msg_t), VIEW_MSGPOOL_MSG_NR);
    if (!view->msgpool) {
        keprint("view: create msgpool failed!\n");
        view_section_destroy(view->section);
        free(view);
        return NULL;
    }
    view->x = x;
    view->y = y;
    view->id = view_next_id++;
    view->z = -1;
    view->width = width;
    view->height = height;
    view->width_min = VIEW_RESIZE_SIZE_MIN;
    view->height_min = VIEW_RESIZE_SIZE_MIN;
    
    view->type = VIEW_TYPE_FIXED;
    view->attr = 0;
    int i;
    for (i = 0; i < VIEW_DRAG_REGION_NR; i++) {
        view_region_reset(&view->drag_regions[i]);
    }
    view_region_init(&view->drag_regions[0], 0, 0, view->width, view->height);
    view_region_init(&view->resize_region, VIEW_RESIZE_BORDER_SIZE,
        VIEW_RESIZE_BORDER_SIZE, view->width - VIEW_RESIZE_BORDER_SIZE,
        view->height - VIEW_RESIZE_BORDER_SIZE);

    list_init(&view->list);
    list_add(&view->global_list, &view_global_list_head);
    return view;
}

view_t *view_find_by_id(int id)
{
    view_t *view;
    list_for_each_owner (view, &view_global_list_head, global_list) {
        if (view->id == id)
            return view;
    }
    return NULL;
}

list_t *view_get_show_list()
{
    return &view_show_list_head;
}

int view_destroy(view_t *view)
{
    if (!view)
        return -1;
    // 记录上一个销毁的视图的位置
    view_last_x = view->x;
    view_last_y = view->y;
    
    if (msgpool_destroy(view->msgpool) < 0) {
        return -1;
    }
    if (view_section_destroy(view->section) < 0) {
        return -1;
    }
    if (list_find(&view->list, &view_show_list_head))
        list_del_init(&view->list);
        
    list_del_init(&view->global_list);
    free(view);
    return 0;
}

int view_set_type(view_t *view, int type)
{
    if (type < VIEW_TYPE_FIXED && type > VIEW_TYPE_FLOAT)
        return -1;
    view->type = type;
    if (view->type == VIEW_TYPE_WINDOW)
        view->attr |= (VIEW_ATTR_MOVEABLE | VIEW_ATTR_RESIZABLE);
    return 0;
}

int view_get_type(view_t *view)
{
    return view->type;
}

int view_add_attr(view_t *view, int attr)
{
    if (!view)
        return -1;
    view->attr |= attr;
    return 0;
}

int view_del_attr(view_t *view, int attr)
{
    if (!view)
        return -1;
    view->attr &= ~attr;
    return 0;
}

int view_get_msg(view_t *view, void *buf, int flags)
{
    if (!view)
        return -1;
    if (!view->msgpool)
        return -1;
    if (flags & DEV_NOWAIT) {
        if (msgpool_try_get(view->msgpool, buf, NULL) < 0)
            return -1;
    } else {
        if (msgpool_get(view->msgpool, buf, NULL) < 0)
            return -1;
    }
    return 0;
}

int view_put_msg(view_t *view, void *buf, int flags)
{
    if (!view)
        return -1;
    if (!view->msgpool)
        return -1;
    if (flags & VIEW_MSG_NOWAIT) {
        if (msgpool_try_put(view->msgpool, buf, sizeof(view_msg_t)) < 0)
            return -1;
    } else {
        if (msgpool_put(view->msgpool, buf, sizeof(view_msg_t)) < 0)
            return -1;
    }
    return 0;
}

void view_clear(view_t *view)
{
    if (view && view->section)
        view_section_clear(view->section);
}

int view_set_size_min(view_t *view, int width, int height)
{
    if (!view)
        return -1;
    view->width_min = width;
    if (view->width_min < VIEW_RESIZE_SIZE_MIN)
        view->width_min = VIEW_RESIZE_SIZE_MIN;
    view->height_min = width;
    if (view->height_min < VIEW_RESIZE_SIZE_MIN)
        view->height_min = VIEW_RESIZE_SIZE_MIN;
    return 0;
}

static void view_refresh_map(int left, int top, int right, int buttom, int z0)
{
    int view_left, view_top, view_right, view_buttom;
    int screen_x, screen_y;
    int view_x, view_y;

    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > view_screen.width)
        right = view_screen.width;
	if (buttom > view_screen.height)
        buttom = view_screen.height;
    
    view_t *view;
    view_color_t *colors;

    /* 刷新高度为[z0-top]区间的视图 */
    list_for_each_owner (view, &view_show_list_head, list) {
        if (view->z >= z0) {
            view_left = left - view->x;
            view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (view_top < 0)
                view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            colors = (view_color_t *)view->section->addr;
            for(view_y = view_top; view_y < view_buttom; view_y++){
                screen_y = view->y + view_y;
                if (screen_y < 0)
                    continue;
                if (screen_y >= view_screen.height)
                    break;
                for(view_x = view_left; view_x < view_right; view_x++){
                    screen_x = view->x + view_x;
                    if (screen_x < 0)
                        continue;
                    if (screen_x >= view_screen.width)
                        break;
                       /* 不是全透明的，就把视图标识写入到映射表中 */
                    if ((colors[view_y * view->width + view_x] >> 24) & 0xff) {
                        view_id_map[(screen_y * view_screen.width + screen_x)] = view->z;
                    }
                }
            }
        }
    }
}

static void __view_adjust_by_z(view_t *view, int z)
{
    view_t *tmp;
    view_t *old_view = NULL;
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
        view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
        view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
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
            view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
            view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
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
            view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z + 1);
            view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z + 1, old_z);
        }
    }
}

static void __view_hiden_by_z(view_t *view, int z)
{
    int old_z = view->z;
    list_del_init(&view->list);
    if (view_top_z > old_z) {  /* 旧视图必须在顶视图下面 */
        /* 把位于当前视图后面的视图的高度都向下降1 */
        view_t *tmp;
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
    view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, 0);
    view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, 0, old_z - 1);
}

static void __view_show_by_z(view_t *view, int z)
{
    view_t *tmp;
    view_t *old_view = NULL;
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
        view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
        view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
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
        view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
        view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
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
void view_set_z(view_t *view, int z)
{
    if (view->z == z)
        return;
    if (list_find(&view->list, &view_show_list_head)) {
        if (z >= 0) {
            __view_adjust_by_z(view, z);
        } else { /* 小于0就是要隐藏起来的视图 */
            __view_hiden_by_z(view, z);
        }
    } else {    /* 插入新视图 */
        if (z >= 0) {
            __view_show_by_z(view, z);
        }
    }
}

int view_drag_rect_check(view_t *view, int x, int y)
{
    view_region_t *rect;
    int i; 
    for (i = 0; i < VIEW_DRAG_REGION_NR; i++) {
        rect = &view->drag_regions[i];
        if (view_region_valid(rect)) {
            if (view_region_in_range(rect, x, y)) {
                return 1;
            }   
        }
    }
    return 0;
}

int view_move_to_top(view_t *view)
{
    if (!view)
        return -1;
    view_set_z(view, view_top_z);
    return 0;
}

int view_move_to_bottom(view_t *view)
{
    if (!view)
        return -1;
    view_set_z(view, 0);
    return 0;
}

int view_move_under_top(view_t *view)
{
    if (!view)
        return -1;
    view_set_z(view, view_top_z - 1);
    return 0;
}

int view_move_upper_top(view_t *view)
{
    if (!view)
        return -1;
    view_set_z(view, view_top_z + 1);
    return 0;
}

int view_hide(view_t *view)
{
    if (!view)
        return -1;
    view_set_z(view, -1);
    return 0;
}

int view_show(view_t *view)
{
    if (!view)
        return -1;
    if (view->z < 0)
        view_move_to_top(view);
    else
        view_move_under_top(view);

    return 0;
}


int view_set_xy(view_t *view, int x, int y)
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
        view_refresh_map(x0, y0, x1, y1, 0);
        view_refresh_by_z(x0, y0, x1, y1, 0, view->z);
    }
    return 0;
}

view_t *view_get_top()
{
    view_t *view = list_last_owner_or_null(&view_show_list_head, view_t, list);
    return view;
}

view_t *view_get_bottom()
{
    view_t *view = list_first_owner_or_null(&view_show_list_head, view_t, list);
    return view;
}

void view_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1)
{
    int view_left, view_top, view_right, view_buttom;

    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > view_screen.width)
        right = view_screen.width;
	if (buttom > view_screen.height)
        buttom = view_screen.height;
    
    int vx, vy;
    int sx, sy;
    
    view_color_t color;
    view_color_t *buf;
    view_t *view;
    list_for_each_owner (view, &view_show_list_head, list) {
        if (view->z >= z0 && view->z <= z1) {
            view_left = left - view->x;
            view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (view_top < 0)
                view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            for (vy = view_top; vy < view_buttom; vy++) {
                sy = view->y + vy;
                for (vx = view_left; vx < view_right; vx++) {
                    sx = view->x + vx;
                    if (view_id_map[sy * view_screen.width + sx] == view->z) {
                        buf = (view_color_t *)view->section->addr;
                        color = buf[vy * view->width + vx];
                        view_screen_write_pixel(sx, sy, color);
                    }
                }
            }
        }
    }
}

void view_refresh(view_t *view, int left, int top, int right, int buttom)
{
    if (view->z >= 0) {
        view_refresh_map(view->x + left, view->y + top, view->x + right,
            view->y + buttom, view->z);
        view_refresh_by_z(view->x + left, view->y + top, view->x + right,
            view->y + buttom, view->z, view->z);
    }
}

void view_refresh_rect(view_t *view, int x, int y, uint32_t width, uint32_t height)
{
    view_refresh(view, x, y, x + width, y + height);
}

/**
 * 刷新图层以及其下面的所有图层
 */
void view_refresh_from_bottom(view_t *view, int left, int top, int right, int buttom)
{
    if (view->z >= 0) {
        view_refresh_map(view->x + left, view->y + top, view->x + right,
            view->y + buttom, 0);
        view_refresh_by_z(view->x + left, view->y + top, view->x + right,
            view->y + buttom, 0, view->z);
    }
}

void view_refresh_rect_from_bottom(view_t *view, int x, int y, uint32_t width, uint32_t height)
{
    view_refresh_from_bottom(view, x, y, x + width, y + height);
}

/**
 * 重新设置图层的大小，并擦除之前的显示内容
 * 
 */
int view_resize(view_t *view, int x, int y, uint32_t width, uint32_t height)
{
    if (!view || !width || !height)
        return -1;
    
    if (!view->section) {
        return -1;
    }
    view_section_t *new_sction = view_section_create(width, height);
    if (!new_sction) {
        errprint("view resize create section failed!\n");
        return -1;
    }
    /* 先将原来位置里面的内容绘制成透明 */
    view_section_clear(view->section);
    /* 重新设置位置才能完整刷新图层 */
    view_set_xy(view, x, y);
    /* 销毁旧的缓冲区 */
    view_section_destroy(view->section);
    /* 重新绑定缓冲区 */
    view->section = new_sction;
    view->width = new_sction->width;
    view->height = new_sction->height;

    /* 重新设置调整大小地区域 */
    view_region_init(&view->resize_region, VIEW_RESIZE_BORDER_SIZE,
        VIEW_RESIZE_BORDER_SIZE, view->width - VIEW_RESIZE_BORDER_SIZE,
        view->height - VIEW_RESIZE_BORDER_SIZE);
    return 0;
}

/**
 * button: 桌面
 * 一般窗口
 * 系统预留窗口
 * top: 鼠标
*/
int view_init()
{
    size_t id_map_size = view_screen.width * view_screen.height * sizeof(uint16_t);
    view_id_map = mem_alloc(id_map_size);
    if (!view_id_map) {
        keprint("malloc for view id map failed!\n");
        return -1;
    }
    memset(view_id_map, 0, id_map_size);

    view_last_x = view_last_y = 0;
    return 0;
}

void view_exit()
{
    view_t *view, *next;
    list_for_each_owner_safe (view, next, &view_global_list_head, global_list) {
        view_destroy(view);
    }
    list_init(&view_show_list_head);
    list_init(&view_global_list_head);
    free(view_id_map);
    view_id_map = NULL;
    view_top_z = -1;
    view_next_id = 0;
}