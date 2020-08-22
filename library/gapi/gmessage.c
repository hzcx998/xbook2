#include <sys/syscall.h>
#include <gmessage.h>
#include <gwindow.h>

int g_btn_down = 0;
int (*_g_msg_routine_call) (g_msg_t *msg);

/**
 * 过滤系统消息。
 * 某些特殊功能，是需要进程和内核互相协作才能完成。
 * 因此过滤时就是来完成这些事情的。
 * 如果消息被处理，则返回0，没有则返回-1
 */
static int g_filter_msg(g_msg_t *msg)
{
    g_window_t *win = g_find_window(msg->target);
    if (win == NULL)
        return -1;
    int val = 0;
    switch (msg->id)
    {
    /* 标题栏的按钮处理 */
    case GM_MOUSE_LBTN_DOWN:
        if (msg->data1 < 24 && msg->data0 < 24) {
            g_btn_down = 1;
        } else if (msg->data1 < 24 && msg->data0 < 48 && msg->data0 >= 24) {
            g_btn_down = 2;
        } else if (msg->data1 < 48 && msg->data0 < 72 && msg->data0 >= 48) {
            g_btn_down = 3;
        }
        break;
    case GM_MOUSE_LBTN_UP:
        if (g_btn_down == 1) {
            if (msg->data1 < 24 && msg->data0 < 24) {
                val = g_post_quit_msg(win->layer);   /* 邮寄一个消息 */
                g_btn_down = 0;
            }
        } else if (g_btn_down == 2) {
            if (msg->data1 < 24 && msg->data0 < 48 && msg->data0 >= 24) {
                g_hide_window(win->layer);
                g_btn_down = 0;
            }
        } else if (g_btn_down == 3) {
            if (msg->data1 < 48 && msg->data0 < 72 && msg->data0 >= 48) {
                g_maxim_window(win->layer);
                g_btn_down = 0;
            }
        }
        break;
    case GM_RESIZE: /* 调整大小 */
        val = g_resize_window(win->layer, msg->data2, msg->data3);
        if (!val) {
            win->x = msg->data0;
            win->y = msg->data1;
        }
        break;
    case GM_MOVE:   /* 窗口移动 */
        win->x = msg->data0;
        win->y = msg->data1;
        if (win->flags & GW_MAXIM) {    /* 最大化时移动，就需要调整位置 */
            /* 要修改备份的位置，因为恢复需要根据备份的位置来设置 */
            win->backup.x = win->x;
            win->backup.y = win->y;
            g_maxim_window(win->layer);
        }
        break;
    case GM_GET_FOCUS:  /* 窗口获得焦点 */
        val = g_focus_window(win->layer, 1);
        break;
    case GM_LOST_FOCUS: /* 窗口丢失焦点 */
        val = g_focus_window(win->layer, 0);
        break;
    default:
        val = -1;   /* 没有处理 */
        break;
    }
    return 0;
}

/**
 * 默认的消息处理方式
 */
static int g_default_msg_proc(g_msg_t *msg)
{

    return 0;
}

int g_set_routine(int (*routine)(g_msg_t *))
{
    _g_msg_routine_call = routine;
    return 0;
}

int g_get_msg(g_msg_t *msg)
{
    int val = syscall1(int, SYS_GGETMSG, msg);
    
    return val;
}

int g_try_get_msg(g_msg_t *msg)
{
    int val = syscall1(int, SYS_GTRYGETMSG, msg);
    return val;
}

int g_dispatch_msg(g_msg_t *msg)
{
    if (!g_filter_msg(msg))
        return 0;
    if (_g_msg_routine_call) {
        if (!_g_msg_routine_call(msg))
            return 0;
    }
    /* 默认处理方式 */
    return g_default_msg_proc(msg);
}

int g_post_msg(g_msg_t *msg)
{
    return syscall1(int, SYS_GPOSTMSG, msg);
}

int g_send_msg(g_msg_t *msg)
{
    return syscall1(int, SYS_GSENDMSG, msg);
}

/**
 * 邮寄退出消息给目标图层
 */
int g_post_quit_msg(int target)
{
    g_msg_t m;
    m.target = target;
    m.id = GM_QUIT;
    return g_post_msg(&m);
}

int g_init_msg()
{
    _g_msg_routine_call = NULL;
    return 0;
}