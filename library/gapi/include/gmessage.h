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
    GM_MOUSE_WHEEL,
    GM_COUNTER,
    GM_TIMER,
    GM_LAYER_ENTER,
    GM_LAYER_LEAVE,
    GM_RESIZE,
    GM_GET_FOCUS,
    GM_LOST_FOCUS,
    GM_MOVE,
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

int g_init_msg();
int g_set_routine(int (*routine)(g_msg_t *));
int g_get_msg(g_msg_t *msg);
int g_dispatch_msg(g_msg_t *msg);
int g_try_get_msg(g_msg_t *msg);
int g_post_msg(g_msg_t *msg);
int g_send_msg(g_msg_t *msg);
int g_post_quit_msg(int target);

#define g_is_quit_msg(msg) ((msg)->id == GM_QUIT)

#endif /* _GAPI_MESSAGE_H */