#ifndef _XGUI_MSG_H
#define _XGUI_MSG_H

#include <stdint.h>

/* GUI message */
enum {
    XGUI_MSG_NONE = 0,
    XGUI_MSG_KEY_DOWN,
    XGUI_MSG_KEY_UP,
    XGUI_MSG_MOUSE_MOTION,
    XGUI_MSG_MOUSE_LBTN_DOWN,
    XGUI_MSG_MOUSE_LBTN_UP,
    XGUI_MSG_MOUSE_LBTN_DBLCLK,
    XGUI_MSG_MOUSE_RBTN_DOWN,
    XGUI_MSG_MOUSE_RBTN_UP,
    XGUI_MSG_MOUSE_RBTN_DBLCLK,
    XGUI_MSG_MOUSE_MBTN_DOWN,
    XGUI_MSG_MOUSE_MBTN_UP,
    XGUI_MSG_MOUSE_MBTN_DBLCLK,
    XGUI_MSG_MOUSE_WHEEL_UP,
    XGUI_MSG_MOUSE_WHEEL_DOWN,
    XGUI_MSG_MOUSE_WHEEL_LEFT,
    XGUI_MSG_MOUSE_WHEEL_RIGHT,
    XGUI_MSG_TIMER,
    XGUI_MSG_NR,
};

typedef struct {
    uint32_t id;        /* 消息id */
    uint32_t data0;     
    uint32_t data1;     
    uint32_t data2;     
    uint32_t data3;
} xbrower_msg_t;

static inline void xbrower_msg_set(xbrower_msg_t *msg, uint32_t id, 
        uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3)
{
    msg->id = id;
    msg->data0 = data0;
    msg->data1 = data1;
    msg->data2 = data2;
    msg->data3 = data3;
}

/* 获取消息的数据 */
#define xbrower_msg_get_type(msg) ((msg)->id)

#define xbrower_msg_get_mouse_x(msg) ((msg)->data0)
#define xbrower_msg_get_mouse_y(msg) ((msg)->data1)

#define xbrower_msg_get_mouse_wheel(msg) ((int)(msg)->data2)

#define xbrower_msg_get_key_code(msg) ((msg)->data0)
#define xbrower_msg_get_key_modify(msg) ((msg)->data1)

#define xbrower_msg_get_timer_id(msg) ((msg)->data0)
#define xbrower_msg_get_timer_time(msg) ((msg)->data1)

#endif /* _XGUI_MSG_H */
