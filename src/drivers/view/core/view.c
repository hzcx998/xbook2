#include "drivers/view/view.h"
#include "drivers/view/hal.h"
#include "drivers/view/misc.h"
#include "drivers/view/render.h"
#include "drivers/view/msg.h"
#include "drivers/view/env.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/ioctl.h>

LIST_HEAD(view_show_list_head);
LIST_HEAD(view_global_list_head);
uint16_t *view_id_map;
static int view_top_z = -1;    
static int view_next_id = 0;    
int view_last_x, view_last_y; // 上一个关闭的视图的位置

/* 视图链表管理的自旋锁 */
DEFINE_SPIN_LOCK(view_list_spin_lock);

/* 视图全局变量自旋锁 */
DEFINE_SPIN_LOCK(view_global_lock);

void view_max_size_repair(int *width, int *height)
{
    if (*width > VIEW_MAX_SIZE_WIDTH) {
        noteprint("kernel view: width repair from %d to %d.\n", *width, VIEW_MAX_SIZE_WIDTH);
        *width = VIEW_MAX_SIZE_WIDTH;
    }
    if (*height > VIEW_MAX_SIZE_HEIGHT) {
        noteprint("kernel view: height repair from %d to %d.\n", *height, VIEW_MAX_SIZE_HEIGHT);
        *height = VIEW_MAX_SIZE_HEIGHT;
    }
}

view_t *view_create(int x, int y, int width, int height, int type)
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
    spin_lock(&view_global_lock);
    view->id = view_next_id++;
    spin_unlock(&view_global_lock);
    view->z = -1;
    view_max_size_repair(&width, &height);
    view->width = width;
    view->height = height;
    view->width_min = VIEW_RESIZE_SIZE_MIN;
    view->height_min = VIEW_RESIZE_SIZE_MIN;

    view->attr = 0;
    view_set_type(view, type);
    int i;
    for (i = 0; i < VIEW_DRAG_REGION_NR; i++) {
        view_region_reset(&view->drag_regions[i]);
    }
    view_region_init(&view->drag_regions[0], 0, 0, view->width, view->height);
    view_region_init(&view->resize_region, VIEW_RESIZE_BORDER_SIZE,
        VIEW_RESIZE_BORDER_SIZE, view->width - VIEW_RESIZE_BORDER_SIZE,
        view->height - VIEW_RESIZE_BORDER_SIZE);

    list_init(&view->list);
    spin_lock(&view_global_lock);
    list_add(&view->global_list, &view_global_list_head);
    spin_unlock(&view_global_lock);
    spinlock_init(&view->lock);

    view_render_rectfill(view, 0, 0, 
        view->width, view->height, VIEW_WHITE);
    return view;
}

view_t *view_find_by_id(int id)
{
    view_t *view;
    spin_lock(&view_global_lock);
    list_for_each_owner (view, &view_global_list_head, global_list) {
        if (view->id == id) {
            spin_unlock(&view_global_lock);
            return view;
        }
    }
    spin_unlock(&view_global_lock);
    return NULL;
}

view_t *view_find_by_z(int z)
{
    view_t *view;
    spin_lock(&view_global_lock);
    list_for_each_owner (view, &view_global_list_head, global_list) {
        if (view->z == z) {
            spin_unlock(&view_global_lock);
            return view;
        }
    }
    spin_unlock(&view_global_lock);
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
    spin_lock(&view_list_spin_lock);
    if (list_find(&view->list, &view_show_list_head))
        list_del_init(&view->list);
    spin_unlock(&view_list_spin_lock);
    
    spin_lock(&view_global_lock);
    list_del_init(&view->global_list);
    spin_unlock(&view_global_lock);
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
    view_max_size_repair(&width, &height);
    view->width_min = width;
    if (view->width_min < VIEW_RESIZE_SIZE_MIN)
        view->width_min = VIEW_RESIZE_SIZE_MIN;
    view->height_min = width;
    if (view->height_min < VIEW_RESIZE_SIZE_MIN)
        view->height_min = VIEW_RESIZE_SIZE_MIN;
    return 0;
}

static void __view_adjust_by_z(view_t *view, int z)
{
    view_t *tmp;
    view_t *old_view = NULL;
    int old_z = view->z;
    
    spin_lock(&view_global_lock);
    if (z > view_top_z) {
        z = view_top_z;
    }
    
    spin_lock(&view_list_spin_lock);
    /* 先从链表中移除 */
    list_del_init(&view->list);
    if (z == view_top_z) {
        spin_unlock(&view_global_lock);
        /* 其它视图降低高度 */
        list_for_each_owner (tmp, &view_show_list_head, list) {
            if (tmp->z > view->z) {
                tmp->z--;
            }
        }
        view->z = z;
        list_add_tail(&view->list, &view_show_list_head);
        
        spin_unlock(&view_list_spin_lock);
        
        /* 刷新新视图[z, z] */
        view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
        view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
    } else {    /* 不是最高视图，那么就和其它视图交换 */
        spin_unlock(&view_global_lock);
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
            
            spin_unlock(&view_list_spin_lock);
        
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
            
            spin_unlock(&view_list_spin_lock);
        
            /* 刷新新视图[z + 1, old z] */
            view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z + 1);
            view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z + 1, old_z);
        }
    }
}

static void __view_hiden_by_z(view_t *view, int z)
{
    int old_z = view->z;
    spin_lock(&view_list_spin_lock);
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
    spin_unlock(&view_list_spin_lock);
    /* 刷新视图, [0, view->z - 1] */
    view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, 0);
    view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, 0, old_z - 1);
}

static void __view_show_by_z(view_t *view, int z)
{
    view_t *tmp;
    view_t *old_view = NULL;
    
    spin_lock(&view_global_lock);
    if (z > view_top_z) {
        view_top_z++;
        z = view_top_z;
    } else {
        view_top_z++;
    }
    spin_lock(&view_list_spin_lock);
    /* 如果新高度就是最高的视图，就直接插入到视图队列末尾 */
    if (z == view_top_z) {
        spin_unlock(&view_global_lock);
        view->z = z;
        list_add_tail(&view->list, &view_show_list_head);
        spin_unlock(&view_list_spin_lock);
        /* 刷新新视图[z, z] */
        view_refresh_map(view->x, view->y, view->x + view->width, view->y + view->height, z);
        view_refresh_by_z(view->x, view->y, view->x + view->width, view->y + view->height, z, z);
    } else {
        spin_unlock(&view_global_lock);
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
        spin_unlock(&view_list_spin_lock);
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
    
    unsigned long iflags;
    spin_lock_irqsave(&view->lock, iflags);

    spin_lock(&view_list_spin_lock);    
    if (list_find(&view->list, &view_show_list_head)) {
        spin_unlock(&view_list_spin_lock);    
    
        if (z >= 0) {
            __view_adjust_by_z(view, z);
        } else { /* 小于0就是要隐藏起来的视图 */
            __view_hiden_by_z(view, z);
        }
    } else {    /* 插入新视图 */
        spin_unlock(&view_list_spin_lock);    
    
        if (z >= 0) {
            __view_show_by_z(view, z);
        }
    }
    spin_unlock_irqrestore(&view->lock, iflags);
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

int view_move_under_view(view_t *view, view_t *target)
{
    if (!view)
        return -1;
    view_set_z(view, target->z - 1);
    return 0;
}

int view_move_upper_view(view_t *view, view_t *target)
{
    if (!view)
        return -1;
    view_set_z(view, target->z + 1);
    return 0;
}

int view_hide(view_t *view)
{
    if (!view)
        return -1;
    view_set_z(view, -1);
    return 0;
}

int view_set_drag_region(view_t *view, view_region_t *region)
{
    if (!view)
        return -1;
    view_region_copy(&view->drag_regions[0], region);
    return 0;
}

int view_show(view_t *view)
{
    if (!view)
        return -1;
    if (view->z < 0) {
        return view_move_to_top(view);
    }
    return view_move_under_top(view);
}

int view_set_xy(view_t *view, int x, int y)
{
    if (!view)
        return -1;
        
    unsigned long iflags;
    spin_lock_irqsave(&view->lock, iflags);

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

    spin_unlock_irqrestore(&view->lock, iflags);
    return 0;
}

view_t *view_get_top()
{
    spin_lock(&view_list_spin_lock);    
    view_t *view = list_last_owner_or_null(&view_show_list_head, view_t, list);
    spin_unlock(&view_list_spin_lock);    
    return view;
}

view_t *view_get_bottom()
{
    spin_lock(&view_list_spin_lock);    
    view_t *view = list_first_owner_or_null(&view_show_list_head, view_t, list);
    spin_unlock(&view_list_spin_lock);    
    return view;
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
    view_max_size_repair((int *) &width, (int *) &height);
    view_section_t *new_sction = view_section_create(width, height);
    if (!new_sction) {
        errprint("view resize create section failed!\n");
        return -1;
    }
    
    unsigned long iflags;
    spin_lock_irqsave(&view->lock, iflags);
    
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
    
    spin_unlock_irqrestore(&view->lock, iflags);
    return 0;
}

void *view_get_vram_start(view_t *view)
{
    if (!view)
        return NULL;
    if (!view->section)
        return NULL;
    return view->section->addr;
}

size_t view_get_vram_size(view_t *view)
{
    if (!view)
        return 0;
    if (!view->section)
        return 0;
    return view->section->size;
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
    if (view_init_refresh() < 0) {
        keprint("view init refresh failed!\n");
        mem_free(view_id_map);
        return -1;;
    }
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