#ifndef _GUI_MESSAGE_H
#define _GUI_MESSAGE_H

#include <stdint.h>
#include <xbook/task.h>

#define GUI_MSG_NR      64

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
    GM_NR
};

typedef struct {
    uint32_t id;        /* 消息id */
    int target;         /* 目标句柄 */
    uint32_t data0;     
    uint32_t data1;     
    uint32_t data2;     
    uint32_t data3;
} g_msg_t;

int gui_init_msg();
int gui_push_msg(g_msg_t *msg);
int gui_pop_msg(g_msg_t *msg);

int gui_msgpool_init(task_t *task);
int gui_msgpool_exit(task_t *task);

int gui_send_quit_msg(task_t *task);

int sys_g_get_msg(g_msg_t *msg);
int sys_g_try_get_msg(g_msg_t *msg);

int sys_g_post_msg(g_msg_t *msg);
int sys_g_send_msg(g_msg_t *msg);

#endif /* _GUI_MESSAGE_H */