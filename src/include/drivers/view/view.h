#ifndef _XBOOK_DRIVERS_VIEW_H
#define _XBOOK_DRIVERS_VIEW_H

#include <stddef.h>
#include <xbook/list.h>
#include <xbook/msgpool.h>
#include "drivers/view/section.h"
#include "drivers/view/msg.h"
#include "drivers/view/misc.h"

// 32个视图
#define VIEW_MAX_NR 32

#define VIEW_MSGPOOL_MSG_NR 16   // 默认消息数量

/* 视图类型 */
enum view_type {
    VIEW_TYPE_FIXED      = 0,
    VIEW_TYPE_WINDOW,
    VIEW_TYPE_FLOAT,
};

enum view_attr {
    VIEW_ATTR_RESIZABLE     = 0x01,
    VIEW_ATTR_MOVEABLE      = 0x02,
};

#define VIEW_DRAG_REGION_NR   4


/* 调整视图大小地边框大小 */
#define VIEW_RESIZE_BORDER_SIZE  4
#define VIEW_RESIZE_SIZE_MIN  16

/* 视图用来表达逻辑上的图层 */
typedef struct {
    list_t list;        // 显示的视图链表
    list_t global_list; // 存在的视图链表
    int id;
    int width;
    int height;
    int width_min;
    int height_min;
    int x;
    int y;
    int z;
    view_section_t *section;
    msgpool_t *msgpool;
    char type;
    char attr;       // 视图的属性
    
    // 区域设置
    view_region_t drag_regions[VIEW_DRAG_REGION_NR];
    view_region_t resize_region;
} view_t;

int view_init();
void view_exit();
view_t *view_create(int x, int y, int width, int height);
int view_destroy(view_t *view);
view_t *view_get_top();
view_t *view_get_bottom();
view_t *view_find_by_id(int id);
int view_set_xy(view_t *view, int x, int y);

void view_set_z(view_t *view, int z);
int view_resize(view_t *view, int x, int y, uint32_t width, uint32_t height);

int view_set_type(view_t *view, int type);
int view_get_type(view_t *view);
int view_add_attr(view_t *view, int attr);
int view_del_attr(view_t *view, int attr);

int view_move_to_top(view_t *view);
int view_move_to_bottom(view_t *view);
int view_move_under_top(view_t *view);
int view_move_upper_top(view_t *view);
int view_hide(view_t *view);
int view_show(view_t *view);
void view_clear(view_t *view);

int view_set_size_min(view_t *view, int width, int height);

int view_drag_rect_check(view_t *view, int x, int y);

int view_set_drag_region(view_t *view, view_region_t *region);

list_t *view_get_show_list();

void view_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1);
void view_refresh(view_t *view, int left, int top, int right, int buttom);
void view_refresh_rect(view_t *view, int x, int y, uint32_t width, uint32_t height);
#define view_self_refresh(view) view_refresh((view), 0, 0, view->width, view->height)
void view_refresh_rect_from_bottom(view_t *view, int x, int y, uint32_t width, uint32_t height);


int view_get_msg(view_t *view, void *buf, int flags);
int view_put_msg(view_t *view, void *buf, int flags);

#define view_try_get_msg(view, buf) view_get_msg(view, buf, VIEW_MSG_NOWAIT)
#define view_try_put_msg(view, buf) view_put_msg(view, buf, VIEW_MSG_NOWAIT)

#endif /* _XBOOK_DRIVERS_VIEW_H */