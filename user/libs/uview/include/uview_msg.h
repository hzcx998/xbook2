#ifndef _LIB_UVIEW_MSG_H
#define _LIB_UVIEW_MSG_H

#include <stdint.h>

/* view message */
enum {
    UVIEW_MSG_NONE = 0,
    UVIEW_MSG_KEY_DOWN,
    UVIEW_MSG_KEY_UP,
    UVIEW_MSG_MOUSE_MOTION,
    UVIEW_MSG_MOUSE_LBTN_DOWN,
    UVIEW_MSG_MOUSE_LBTN_UP,
    UVIEW_MSG_MOUSE_LBTN_DBLCLK,
    UVIEW_MSG_MOUSE_RBTN_DOWN,
    UVIEW_MSG_MOUSE_RBTN_UP,
    UVIEW_MSG_MOUSE_RBTN_DBLCLK,
    UVIEW_MSG_MOUSE_MBTN_DOWN,
    UVIEW_MSG_MOUSE_MBTN_UP,
    UVIEW_MSG_MOUSE_MBTN_DBLCLK,
    UVIEW_MSG_MOUSE_WHEEL_UP,
    UVIEW_MSG_MOUSE_WHEEL_DOWN,
    UVIEW_MSG_MOUSE_WHEEL_LEFT,
    UVIEW_MSG_MOUSE_WHEEL_RIGHT,
    UVIEW_MSG_TIMER,
    UVIEW_MSG_QUIT,
    UVIEW_MSG_ENTER,
    UVIEW_MSG_LEAVE,
    UVIEW_MSG_RESIZE,
    UVIEW_MSG_ACTIVATE,
    UVIEW_MSG_INACTIVATE,
    UVIEW_MSG_MOVE,
    UVIEW_MSG_CREATE,
    UVIEW_MSG_CLOSE,
    UVIEW_MSG_HIDE,
    UVIEW_MSG_SHOW,
    UVIEW_MSG_PAINT,
    UVIEW_MSG_NR,
};

typedef struct {
    uint32_t id;        /* 消息id */
    int target;         /* 消息目标 */
    uint32_t data0;     
    uint32_t data1;     
    uint32_t data2;     
    uint32_t data3;
} uview_msg_t;


static inline void uview_msg_header(uview_msg_t *msg, uint32_t id, int target)
{
    msg->id = id;
    msg->target = target;
}

static inline void uview_msg_data(uview_msg_t *msg, 
        uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3)
{
    msg->data0 = data0;
    msg->data1 = data1;
    msg->data2 = data2;
    msg->data3 = data3;
}

static inline void uview_msg_reset(uview_msg_t *msg)
{
    msg->id = 0;
    msg->target = -1;
    msg->data0 = 0;
    msg->data1 = 0;
    msg->data2 = 0;
    msg->data3 = 0;
}

#define is_uview_msg_valid(msg) ((msg)->id > UVIEW_MSG_NONE)
#define uview_msg_get_type(msg) ((msg)->id)
#define uview_msg_get_mouse_x(msg) ((msg)->data0)
#define uview_msg_get_mouse_y(msg) ((msg)->data1)
#define uview_msg_get_mouse_wheel(msg) ((int)(msg)->data2)
#define uview_msg_get_key_code(msg) ((msg)->data0)
#define uview_msg_get_key_modify(msg) ((msg)->data1)

#define uview_msg_get_resize_x(msg) ((msg)->data0)
#define uview_msg_get_resize_y(msg) ((msg)->data1)
#define uview_msg_get_resize_width(msg) ((msg)->data2)
#define uview_msg_get_resize_height(msg) ((msg)->data3)

#endif  /* _LIB_UVIEW_MSG_H */