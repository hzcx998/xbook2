#ifndef _XBOOK_DRIVERS_VIEW_MSG_H
#define _XBOOK_DRIVERS_VIEW_MSG_H

#include <stdint.h>

/* GUI message */
enum {
    VIEW_MSG_NONE = 0,
    VIEW_MSG_KEY_DOWN,
    VIEW_MSG_KEY_UP,
    VIEW_MSG_MOUSE_MOTION,
    VIEW_MSG_MOUSE_LBTN_DOWN,
    VIEW_MSG_MOUSE_LBTN_UP,
    VIEW_MSG_MOUSE_LBTN_DBLCLK,
    VIEW_MSG_MOUSE_RBTN_DOWN,
    VIEW_MSG_MOUSE_RBTN_UP,
    VIEW_MSG_MOUSE_RBTN_DBLCLK,
    VIEW_MSG_MOUSE_MBTN_DOWN,
    VIEW_MSG_MOUSE_MBTN_UP,
    VIEW_MSG_MOUSE_MBTN_DBLCLK,
    VIEW_MSG_MOUSE_WHEEL_UP,
    VIEW_MSG_MOUSE_WHEEL_DOWN,
    VIEW_MSG_MOUSE_WHEEL_LEFT,
    VIEW_MSG_MOUSE_WHEEL_RIGHT,
    VIEW_MSG_TIMER,
    VIEW_MSG_NR,
};

typedef struct {
    uint32_t id;        /* 消息id */
    int target;         /* 消息目标 */
    uint32_t data0;     
    uint32_t data1;     
    uint32_t data2;     
    uint32_t data3;
} view_msg_t;

static inline void view_msg_header(view_msg_t *msg, uint32_t id, int target)
{
    msg->id = id;
    msg->target = target;
}

static inline void view_msg_data(view_msg_t *msg, 
        uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3)
{
    msg->data0 = data0;
    msg->data1 = data1;
    msg->data2 = data2;
    msg->data3 = data3;
}

static inline void view_msg_reset(view_msg_t *msg)
{
    msg->id = 0;
    msg->target = -1;
    msg->data0 = 0;
    msg->data1 = 0;
    msg->data2 = 0;
    msg->data3 = 0;
}

#define VIEW_MSG_NOWAIT 0x01

#define is_view_msg_valid(msg) ((msg)->id > VIEW_MSG_NONE)

/* 获取消息的数据 */
#define view_msg_get_type(msg) ((msg)->id)

#define view_msg_get_mouse_x(msg) ((msg)->data0)
#define view_msg_get_mouse_y(msg) ((msg)->data1)

#define view_msg_get_mouse_wheel(msg) ((int)(msg)->data2)

#define view_msg_get_key_code(msg) ((msg)->data0)
#define view_msg_get_key_modify(msg) ((msg)->data1)

#define view_msg_get_timer_id(msg) ((msg)->data0)
#define view_msg_get_timer_time(msg) ((msg)->data1)

int view_global_msg_init();
int view_get_global_msg(view_msg_t *msg);
int view_put_global_msg(view_msg_t *msg);

int view_dispatch_keycode_msg(view_msg_t *msg);
int view_dispatch_mouse_msg(view_msg_t *msg);
int view_dispatch_target_msg(view_msg_t *msg);

#endif /* _XBOOK_DRIVERS_VIEW_MSG_H */
