#ifndef _GAPI_MESSAGE_H
#define _GAPI_MESSAGE_H

#include <stdint.h>

/* GUI message */
enum {
    GM_NONE = 0,
    GM_QUIT,
    GM_KEY_DOWN,
    GM_KEY_UP,
    GM_MOUSE_MOTION,
    GM_MOUSE_LBTN_DOWN,
    GM_MOUSE_LBTN_UP,
    GM_MOUSE_LBTN_DBLCLK,
    GM_MOUSE_RBTN_DOWN,
    GM_MOUSE_RBTN_UP,
    GM_MOUSE_RBTN_DBLCLK,
    GM_MOUSE_MBTN_DOWN,
    GM_MOUSE_MBTN_UP,
    GM_MOUSE_MBTN_DBLCLK,
    GM_MOUSE_WHEEL_UP,
    GM_MOUSE_WHEEL_DOWN,
    GM_MOUSE_WHEEL_LEFT,
    GM_MOUSE_WHEEL_RIGHT,
    GM_TIMER,
    GM_LAYER_ENTER,
    GM_LAYER_LEAVE,
    GM_RESIZE,
    GM_GET_FOCUS,
    GM_LOST_FOCUS,
    GM_MOVE,
    GM_WINDOW_CREATE,
    GM_WINDOW_CLOSE,
    GM_HIDE,
    GM_SHOW,
    GM_PAINT,
    GM_WINDOW_ICON,
    GM_NR,
};


typedef struct {
    uint32_t id;        /* 消息id */
    int target;         /* 目标句柄 */
    uint32_t data0;     
    uint32_t data1;     
    uint32_t data2;     
    uint32_t data3;
} g_msg_t;

int g_get_msg(g_msg_t *msg);
int g_try_get_msg(g_msg_t *msg);
int g_post_msg(g_msg_t *msg);
int g_send_msg(g_msg_t *msg);
int g_post_quit_msg(int target);
int g_translate_msg(g_msg_t *msg);

#define g_is_quit_msg(msg) ((msg)->id == GM_QUIT)

/* 获取消息的数据 */
#define g_msg_get_type(msg) ((msg)->id)
#define g_msg_get_target(msg) ((msg)->target)

#define g_msg_get_mouse_x(msg) ((msg)->data0)
#define g_msg_get_mouse_y(msg) ((msg)->data1)
#define g_msg_get_mouse_x_global(msg) ((msg)->data2)
#define g_msg_get_mouse_y_global(msg) ((msg)->data3)

#define g_msg_get_move_x(msg) ((msg)->data0)
#define g_msg_get_move_y(msg) ((msg)->data1)

#define g_msg_get_mouse_wheel(msg) ((int)(msg)->data2)

#define g_msg_get_resize_x(msg) ((msg)->data0)
#define g_msg_get_resize_y(msg) ((msg)->data1)
#define g_msg_get_resize_width(msg) ((msg)->data2)
#define g_msg_get_resize_height(msg) ((msg)->data3)

#define g_msg_get_key_code(msg) ((msg)->data0)
#define g_msg_get_key_modify(msg) ((msg)->data1)

#define g_msg_get_sender(msg) ((msg)->data0)

#define g_msg_get_paint_x(msg) ((msg)->data0)
#define g_msg_get_paint_y(msg) ((msg)->data1)
#define g_msg_get_paint_width(msg) ((msg)->data2)
#define g_msg_get_paint_height(msg) ((msg)->data3)

#define g_msg_get_timer_id(msg) ((msg)->data0)
#define g_msg_get_timer_time(msg) ((msg)->data1)



#endif /* _GAPI_MESSAGE_H */