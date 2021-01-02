#ifndef _XBOOK_DRIVERS_VIEW_H
#define _XBOOK_DRIVERS_VIEW_H

#include <stddef.h>
#include <xbook/list.h>
#include <xbook/msgpool.h>
#include "drivers/view/section.h"

// 32个视图
#define VIEW_MAX_NR 32

#define VIEW_MSGPOOL_MSG_NR 16   // 默认消息数量

/* 视图用来表达逻辑上的图层 */
typedef struct {
    list_t list;        // 显示的视图链表
    list_t global_list; // 存在的视图链表
    int id;
    int width;
    int height;
    int x;
    int y;
    int z;
    view_section_t *section;
    msgpool_t *msgpool;
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
int view_move_to_top(view_t *view);
int view_move_to_bottom(view_t *view);
int view_move_under_top(view_t *view);
int view_move_upper_top(view_t *view);
int view_hide(view_t *view);
int view_show(view_t *view);

view_t *view_get_focused();
list_t *view_get_show_list();

void view_refresh_view_by_z(int left, int top, int right, int buttom, int z0, int z1);
void view_refresh(view_t *view, int left, int top, int right, int buttom);
#define view_self_refresh(view) view_refresh((view), 0, 0, view->width, view->height)

int view_get_msg(view_t *view, void *buf, int flags);
int view_put_msg(view_t *view, void *buf, int flags);

#endif /* _XBOOK_DRIVERS_VIEW_H */