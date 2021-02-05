#include <drivers/view/msg.h>
#include <drivers/view/view.h>
#include <drivers/view/mouse.h>
#include <drivers/view/env.h>
#include <stddef.h>

static msgpool_t *view_global_msgpool = NULL;

int view_global_msg_init()
{
    view_global_msgpool = msgpool_create(sizeof(view_msg_t), VIEW_MSGPOOL_MSG_NR * 2);
    if (view_global_msgpool == NULL)
        return -1;
    return 0;
}

void view_global_msg_exit()
{
    if (view_global_msgpool)
        msgpool_destroy(view_global_msgpool);
    view_global_msgpool = NULL;
}

int view_get_global_msg(view_msg_t *msg)
{
    return msgpool_try_get(view_global_msgpool, msg, NULL);
}

int view_put_global_msg(view_msg_t *msg)
{
    return msgpool_try_put(view_global_msgpool, msg, sizeof(view_msg_t));
}

/**
 * 派发键盘消息
 */
int view_dispatch_keycode_msg(view_msg_t *msg)
{
    view_t *view = view_env_get_activity();
    int val = -1;
    /* 发送给聚焦图层 */
    if (view) {
        /* 发送消息 */
        view_msg_t m;
        view_msg_header(&m, msg->id, view->id);
        view_msg_data(&m, msg->data0, msg->data1, 0, 0);
        val = view_put_msg(view, &m, VIEW_MSG_NOWAIT);
    }
    return val;
}

/**
 * 派发鼠标消息
 */
int view_dispatch_mouse_msg(view_msg_t *msg)
{
    if (!view_env_filter_mouse_msg(msg))
        return 0;

    list_t *list_head = view_get_show_list();
    view_t *view;
    list_for_each_owner_reverse (view, list_head, list) {
        /* 鼠标视图就跳过 */
        if (view == view_mouse.view)
            continue;
        int local_mx, local_my;
        local_mx = msg->data0 - view->x;
        local_my = msg->data1 - view->y;
        if (local_mx >= 0 && local_mx < view->width && 
            local_my >= 0 && local_my < view->height) {
            /* 如果是在图层上点击了鼠标左键，那么就进行激活 */
            if (view_msg_get_type(msg) == VIEW_MSG_MOUSE_LBTN_DOWN) {
                view_env_try_activate(view);
            }
            view_env_do_mouse_hover(view, msg, local_mx, local_my);
            if (!view_env_do_resize(view, msg, local_mx, local_my)) 
                return 0;
            view_env_do_drag(view, msg, local_mx, local_my);
            
            view_msg_t m;
            view_msg_header(&m, msg->id, view->id);
            view_msg_data(&m, local_mx, local_my, msg->data0, msg->data1);
            view_put_msg(view, &m, VIEW_MSG_NOWAIT);
            return 0;
        }
    }
    return -1;
}

int view_dispatch_target_msg(view_msg_t *msg)
{
    if (msg->target == VIEW_TARGET_NONE)
        return -1;
    
    view_t *view = view_find_by_id(msg->target);
    if (!view) {
        return -1;
    }
    /* 转发-发送消息 */
    return view_put_msg(view, msg, VIEW_MSG_NOWAIT);
}
