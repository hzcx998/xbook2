#include <sys/syscall.h>
#include <gmessage.h>
#include <gwindow.h>
#include <gtouch.h>
#include <gkeycode.h>
#include <stdio.h>

int (*_g_msg_routine_call) (g_msg_t *msg);

/**
 * 过滤系统消息。
 * 某些特殊功能，是需要进程和内核互相协作才能完成。
 * 因此过滤时就是来完成这些事情的。
 * 如果消息被处理，则返回0，没有则返回-1
 */
static int g_filter_msg(g_msg_t *msg)
{
    g_window_t *win = g_find_window(g_msg_get_target(msg));
    if (win == NULL)
        return -1;
    int val = -1; /* -1表示没有过滤，0表示已经过滤 */
    g_point_t po;
    switch (g_msg_get_type(msg))
    {
    case GM_MOUSE_MOTION:
        /* 触摸状态改变 */
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_state_check_group(&win->touch_list, &po);
        if (g_region_in(&win->body_region, po.x, po.y)) {
            /* 进行坐标转换 */
            g_msg_get_mouse_x(msg) = po.x - win->body_region.left;
            g_msg_get_mouse_y(msg) = po.y - win->body_region.top;
        } else {
            val = 0; /* 过滤该消息 */
        }
        break;
    /* 标题栏的按钮处理 */
    case GM_MOUSE_LBTN_DOWN:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_click_check_group(&win->touch_list, &po, 1);
        if (g_region_in(&win->body_region, po.x, po.y)) {
            /* 进行坐标转换 */
            g_msg_get_mouse_x(msg) = po.x - win->body_region.left;
            g_msg_get_mouse_y(msg) = po.y - win->body_region.top;
        } else {
            val = 0; /* 过滤该消息 */
        }
        break;
    case GM_MOUSE_LBTN_UP:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_click_check_group(&win->touch_list, &po, 0);
        if (g_region_in(&win->body_region, po.x, po.y)) {
            /* 进行坐标转换 */
            g_msg_get_mouse_x(msg) = po.x - win->body_region.left;
            g_msg_get_mouse_y(msg) = po.y - win->body_region.top;
        } else {
            val = 0; /* 过滤该消息 */
        }
        break;
    case GM_MOUSE_LBTN_DBLCLK: /* 过滤非活动区域内的坐标信息 */
    case GM_MOUSE_RBTN_DOWN:
    case GM_MOUSE_RBTN_UP:
    case GM_MOUSE_RBTN_DBLCLK:
    case GM_MOUSE_MBTN_DOWN:
    case GM_MOUSE_MBTN_UP:
    case GM_MOUSE_MBTN_DBLCLK:
    case GM_MOUSE_WHEEL:
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        if (g_region_in(&win->body_region, po.x, po.y)) {
            /* 进行坐标转换 */
            g_msg_get_mouse_x(msg) = po.x - win->body_region.left;
            g_msg_get_mouse_y(msg) = po.y - win->body_region.top;
        } else {
            val = 0; /* 过滤该消息 */
        }
        break;
    case GM_RESIZE: /* 调整大小 */
        val = g_resize_window(win->layer, g_msg_get_resize_width(msg), g_msg_get_resize_height(msg));
        if (!val) {
            win->x =  g_msg_get_resize_x(msg);
            win->y =  g_msg_get_resize_y(msg);
        }
        /* 调整窗口后，鼠标位置发生了改变 */
        po.x = -1;
        po.y = -1;
        g_touch_state_check_group(&win->touch_list, &po);
        break;
    case GM_MOVE:   /* 窗口移动 */
        win->x = g_msg_get_move_x(msg);
        win->y = g_msg_get_move_y(msg);
        if (win->flags & GW_MAXIM) {    /* 最大化时移动，就需要调整位置 */
            /* 要修改备份的位置，因为恢复需要根据备份的位置来设置 */
            win->backup.x = win->x;
            win->backup.y = win->y;
            g_maxim_window(win->layer);
        }
        val = 0;
        break;
    case GM_GET_FOCUS:  /* 窗口获得焦点 */
        g_focus_window(win->layer, 1);
        val = 0;
        break;
    case GM_LOST_FOCUS: /* 窗口丢失焦点 */
        g_focus_window(win->layer, 0);
        val = 0;
        break;
    case GM_LAYER_LEAVE: /* 离开窗口 */
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_state_check_group(&win->touch_list, &po);
        break;
    case GM_LAYER_ENTER: /* 进入窗口 */
        po.x = g_msg_get_mouse_x(msg);
        po.y = g_msg_get_mouse_y(msg);
        g_touch_state_check_group(&win->touch_list, &po);
        break;
    case GM_HIDE: /* 隐藏窗口 */
        /* 调整窗口后，鼠标位置发生了改变 */
        po.x = -1;
        po.y = -1;
        g_touch_state_check_group(&win->touch_list, &po);
        
        g_hide_window(win->layer);
        val = 0;
        break;
    case GM_SHOW: /* 显示窗口 */
        /* 调整窗口后，鼠标位置发生了改变 */
        po.x = -1;
        po.y = -1;
        g_touch_state_check_group(&win->touch_list, &po);
        g_show_window(win->layer);
        val = 0;
        break;   
    default:
        break;
    }
    return val;
}

/**
 * 默认的消息处理方式
 */
static int g_default_msg_proc(g_msg_t *msg)
{

    return 0;
}

int g_set_msg_routine(int (*routine)(g_msg_t *))
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

/* 将小键盘的按键转换成主键盘上面的按键 */
static const unsigned char _g_keycode_map_table[] = {
    GK_ESCAPE,         GK_ESCAPE,
    GK_BACKSPACE,      GK_BACKSPACE,
    GK_TAB,            GK_TAB,
    /* 0x00,            GK_BACK_TAB,*/

    GK_PAUSE,          GK_PAUSE,

    GK_PRINT,          GK_PRINT,
    GK_SYSREQ,         GK_SYSREQ,

    GK_CLEAR,          GK_CLEAR,

    GK_INSERT,         GK_INSERT,
    GK_ENTER,          GK_ENTER,
    GK_DELETE,         GK_DELETE,
    GK_KP_ENTER,       GK_ENTER,

    GK_LEFT,           GK_LEFT,
    GK_RIGHT,          GK_RIGHT,
    GK_UP,             GK_UP,
    GK_DOWN,           GK_DOWN,
    GK_HOME,           GK_HOME,
    GK_END,            GK_END,
    GK_PAGEUP,         GK_PAGEUP,
    GK_PAGEDOWN,       GK_PAGEDOWN,

    GK_KP0,            GK_INSERT,
    GK_KP_PERIOD,      GK_DELETE,
    GK_KP1,            GK_END,
    GK_KP2,            GK_DOWN,
    GK_KP3,            GK_PAGEDOWN,
    GK_KP4,            GK_LEFT,
    GK_KP5,            GK_5,
    GK_KP6,            GK_RIGHT,
    GK_KP7,            GK_HOME,
    GK_KP8,            GK_UP,
    GK_KP9,            GK_PAGEUP,

    GK_SPACE,          GK_SPACE,           /*   */
    GK_EXCLAIM,        GK_EXCLAIM,         /* ! */
    GK_QUOTEDBL,       GK_QUOTEDBL,        /* " */
    GK_HASH,           GK_HASH,            /* # */
    GK_DOLLAR,         GK_DOLLAR,          /* $ */
    GK_PERSENT,        GK_PERSENT,         /* % */
    GK_AMPERSAND,      GK_AMPERSAND,       /* & */
    GK_LEFTPAREN,      GK_LEFTPAREN,      /* ( */
    GK_RIGHTPAREN,     GK_RIGHTPAREN,     /* ) */
    GK_ASTERISK,       GK_ASTERISK,        /* * */
    GK_KP_MULTIPLY,       GK_ASTERISK,     /* * */
    GK_PLUS,           GK_PLUS,            /* + */
    GK_KP_PLUS,        GK_PLUS,            /* + */
    GK_COMMA,          GK_COMMA,           /* , */
    GK_MINUS,          GK_MINUS,           /* - */
    GK_KP_MINUS,       GK_MINUS,           /* - */
    GK_PERIOD,         GK_PERIOD,             /* . */
    GK_SLASH,          GK_SLASH,           /* / */
    GK_KP_DIVIDE,      GK_SLASH,           /* / */

    GK_0,              GK_0,               /* 0 */
    GK_1,              GK_1,               /* 1 */
    GK_2,              GK_2,               /* 2 */
    GK_3,              GK_3,               /* 3 */
    GK_4,              GK_4,               /* 4 */
    GK_5,              GK_5,               /* 5 */
    GK_6,              GK_6,               /* 6 */
    GK_7,              GK_7,               /* 7 */
    GK_8,              GK_8,               /* 8 */
    GK_9,              GK_9,               /* 9 */
    GK_KP0,            GK_0,               /* 0 */
    GK_KP1,            GK_1,               /* 1 */
    GK_KP2,            GK_2,               /* 2 */
    GK_KP3,            GK_3,               /* 3 */
    GK_KP4,            GK_4,               /* 4 */
    GK_KP5,            GK_5,               /* 5 */
    GK_KP6,            GK_6,               /* 6 */
    GK_KP7,            GK_7,               /* 7 */
    GK_KP8,            GK_8,               /* 8 */
    GK_KP9,            GK_9,               /* 9 */
    GK_COLON,          GK_COLON,           /* : */
    GK_SEMICOLON,      GK_SEMICOLON,       /* ; */
    GK_LESS,           GK_LESS,            /* < */
    GK_EQUALS,         GK_EQUALS,          /* = */
    GK_GREATER,        GK_GREATER,         /* > */
    GK_QUESTION,       GK_QUESTION,        /* ? */
    GK_AT,             GK_AT,              /* @ */
    GK_A,              GK_A,               /* A */
    GK_B,              GK_B,               /* B */
    GK_C,              GK_C,               /* C */
    GK_D,              GK_D,               /* D */
    GK_E,              GK_E,               /* E */
    GK_F,              GK_F,               /* F */
    GK_G,              GK_G,               /* G */
    GK_H,              GK_H,               /* H */
    GK_I,              GK_I,               /* I */
    GK_J,              GK_J,               /* J */
    GK_K,              GK_K,               /* K */
    GK_L,              GK_L,               /* L */
    GK_M,              GK_M,               /* M */
    GK_N,              GK_N,               /* N */
    GK_O,              GK_O,               /* O */
    GK_P,              GK_P,               /* P */
    GK_Q,              GK_Q,               /* Q */
    GK_R,              GK_R,               /* R */
    GK_S,              GK_S,               /* S */
    GK_T,              GK_T,               /* T */
    GK_U,              GK_U,               /* U */
    GK_V,              GK_V,               /* V */
    GK_W,              GK_W,               /* W */
    GK_X,              GK_X,               /* X */
    GK_Y,              GK_Y,               /* Y */
    GK_Z,              GK_Z,               /* Z */
    GK_a,              GK_a,               /* A */
    GK_b,              GK_b,               /* B */
    GK_c,              GK_c,               /* C */
    GK_d,              GK_d,               /* D */
    GK_e,              GK_e,               /* E */
    GK_f,              GK_f,               /* F */
    GK_g,              GK_g,               /* G */
    GK_h,              GK_h,               /* H */
    GK_i,              GK_i,               /* I */
    GK_j,              GK_j,               /* J */
    GK_k,              GK_k,               /* K */
    GK_l,              GK_l,               /* L */
    GK_m,              GK_m,               /* M */
    GK_n,              GK_n,               /* N */
    GK_o,              GK_o,               /* O */
    GK_p,              GK_p,               /* P */
    GK_q,              GK_q,               /* Q */
    GK_r,              GK_r,               /* R */
    GK_s,              GK_s,               /* S */
    GK_t,              GK_t,               /* T */
    GK_u,              GK_u,               /* U */
    GK_v,              GK_v,               /* V */
    GK_w,              GK_w,               /* W */
    GK_x,              GK_x,               /* X */
    GK_y,              GK_y,               /* Y */
    GK_z,              GK_z,               /* Z */
    GK_LEFTSQUAREBRACKET,  GK_LEFTSQUAREBRACKET,    /* [ */
    GK_BACKSLASH,      GK_BACKSLASH,      /* \ */
    GK_RIGHTSQUAREBRACKET, GK_RIGHTSQUAREBRACKET,   /* ] */
    GK_CARET,          GK_CARET,    /* ^ */
    GK_UNDERSCRE,      GK_UNDERSCRE,      /* _ */
    GK_BACKQUOTE,      GK_BACKQUOTE,           /* ` */

    GK_LEFTBRACKET,    GK_LEFTBRACKET,      /* { */
    GK_VERTICAL,       GK_VERTICAL,             /* | */
    GK_RIGHTBRACKET,   GK_RIGHTBRACKET,     /* } */
    GK_TILDE,          GK_TILDE,     /* ~ */
    GK_QUOTE,          GK_QUOTE,      /* ' */

    GK_F1,             GK_F1,
    GK_F2,             GK_F2,
    GK_F3,             GK_F3,
    GK_F4,             GK_F4,
    GK_F5,             GK_F5,
    GK_F6,             GK_F6,
    GK_F7,             GK_F7,
    GK_F8,             GK_F8,
    GK_F9,             GK_F9,
    GK_F10,            GK_F10,
    GK_F11,            GK_F11,
    GK_F12,            GK_F12,

    GK_LSHIFT,         GK_LSHIFT,
    GK_RSHIFT,         GK_RCTRL,

    GK_LCTRL,          GK_LCTRL,
    GK_RCTRL,          GK_RCTRL,

    GK_LALT,           GK_LALT,
    GK_RALT,           GK_RALT,

    GK_LMETA,          GK_LMETA,
    GK_RMETA,          GK_RMETA,

    GK_CAPSLOCK,       GK_CAPSLOCK,
    GK_NUMLOCK,        GK_NUMLOCK,
    GK_SCROLLOCK,      GK_SCROLLOCK,

    0xFF,               GK_UNKNOWN
};
/**
 * 键值转换
 */
static unsigned char _g_key_code_switch(int code)
{
    unsigned char key_value = '?';
    unsigned int i = 0;
    for ( i = 0;  i < sizeof(_g_keycode_map_table);  i += 2 ) {
        if (_g_keycode_map_table[i] == code) {
            key_value = _g_keycode_map_table[i + 1]; // 返回转换后的键值
            break;
        }
    }
    return key_value;
}

int g_translate_msg(g_msg_t *msg)
{
    switch (g_msg_get_type(msg))
    {
    case GM_KEY_DOWN:
        g_msg_get_key_code(msg) = _g_key_code_switch(g_msg_get_key_code(msg));
        break;
    case GM_KEY_UP:
        g_msg_get_key_code(msg) = _g_key_code_switch(g_msg_get_key_code(msg));
        break;
    default:
        break;
    }
    return 0;
}