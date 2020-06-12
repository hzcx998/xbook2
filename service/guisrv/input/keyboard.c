#include <drivers/keyboard.h>
#include <input/keyboard.h>
#include <stdio.h>
#include <sys/input.h>
#include <window/event.h>

#define DEBUG_LOCAL 0

input_keyboard_t input_keyboard;

/* Keyboard mapping table */
static  const  unsigned char  map_table[] = {
    KEY_ESCAPE,         SGIK_ESCAPE,
    KEY_BACKSPACE,      SGIK_BACKSPACE,
    KEY_TAB,            SGIK_TAB,
    /* 0x00,            SGIK_BACK_TAB,*/

    KEY_PAUSE,          SGIK_PAUSE,

    KEY_PRINT,          SGIK_PRINT,
    KEY_SYSREQ,         SGIK_SYSREQ,

    KEY_CLEAR,          SGIK_CLEAR,

    KEY_INSERT,         SGIK_INSERT,
    KEY_ENTER,          SGIK_ENTER,
    KEY_DELETE,         SGIK_DELETE,
    KEY_KP_ENTER,        SGIK_ENTER,

    KEY_LEFT,           SGIK_LEFT,
    KEY_RIGHT,          SGIK_RIGHT,
    KEY_UP,             SGIK_UP,
    KEY_DOWN,           SGIK_DOWN,
    KEY_HOME,           SGIK_HOME,
    KEY_END,            SGIK_END,
    KEY_PAGEUP,         SGIK_PAGEUP,
    KEY_PAGEDOWN,       SGIK_PAGEDOWN,

    KEY_KP0,            SGIK_INSERT,
    KEY_KP_PERIOD,      SGIK_DELETE,
    KEY_KP1,            SGIK_END,
    KEY_KP2,            SGIK_DOWN,
    KEY_KP3,            SGIK_PAGEDOWN,
    KEY_KP4,            SGIK_LEFT,
    KEY_KP5,            SGIK_5,
    KEY_KP6,            SGIK_RIGHT,
    KEY_KP7,            SGIK_HOME,
    KEY_KP8,            SGIK_UP,
    KEY_KP9,            SGIK_PAGEUP,

    KEY_SPACE,          SGIK_SPACE,           /*   */
    KEY_EXCLAIM,        SGIK_EXCLAIM,         /* ! */
    KEY_QUOTEDBL,       SGIK_QUOTEDBL,        /* " */
    KEY_HASH,           SGIK_HASH,            /* # */
    KEY_DOLLAR,         SGIK_DOLLAR,          /* $ */
    KEY_PERSENT,        SGIK_PERSENT,         /* % */
    KEY_AMPERSAND,      SGIK_AMPERSAND,       /* & */
    KEY_LEFTPAREN,      SGIK_LEFTPAREN,      /* ( */
    KEY_RIGHTPAREN,     SGIK_RIGHTPAREN,     /* ) */
    KEY_ASTERISK,       SGIK_ASTERISK,        /* * */
    KEY_KP_MULTIPLY,       SGIK_ASTERISK,     /* * */
    KEY_PLUS,           SGIK_PLUS,            /* + */
    KEY_KP_PLUS,        SGIK_PLUS,            /* + */
    KEY_COMMA,          SGIK_COMMA,           /* , */
    KEY_MINUS,          SGIK_MINUS,           /* - */
    KEY_KP_MINUS,       SGIK_MINUS,           /* - */
    KEY_PERIOD,         SGIK_PERIOD,             /* . */
    KEY_SLASH,          SGIK_SLASH,           /* / */
    KEY_KP_DIVIDE,      SGIK_SLASH,           /* / */

    KEY_0,              SGIK_0,               /* 0 */
    KEY_1,              SGIK_1,               /* 1 */
    KEY_2,              SGIK_2,               /* 2 */
    KEY_3,              SGIK_3,               /* 3 */
    KEY_4,              SGIK_4,               /* 4 */
    KEY_5,              SGIK_5,               /* 5 */
    KEY_6,              SGIK_6,               /* 6 */
    KEY_7,              SGIK_7,               /* 7 */
    KEY_8,              SGIK_8,               /* 8 */
    KEY_9,              SGIK_9,               /* 9 */
    KEY_KP0,            SGIK_0,               /* 0 */
    KEY_KP1,            SGIK_1,               /* 1 */
    KEY_KP2,            SGIK_2,               /* 2 */
    KEY_KP3,            SGIK_3,               /* 3 */
    KEY_KP4,            SGIK_4,               /* 4 */
    KEY_KP5,            SGIK_5,               /* 5 */
    KEY_KP6,            SGIK_6,               /* 6 */
    KEY_KP7,            SGIK_7,               /* 7 */
    KEY_KP8,            SGIK_8,               /* 8 */
    KEY_KP9,            SGIK_9,               /* 9 */
    KEY_COLON,          SGIK_COLON,           /* : */
    KEY_SEMICOLON,      SGIK_SEMICOLON,       /* ; */
    KEY_LESS,           SGIK_LESS,            /* < */
    KEY_EQUALS,         SGIK_EQUALS,          /* = */
    KEY_GREATER,        SGIK_GREATER,         /* > */
    KEY_QUESTION,       SGIK_QUESTION,        /* ? */
    KEY_AT,             SGIK_AT,              /* @ */
    KEY_A,              SGIK_A,               /* A */
    KEY_B,              SGIK_B,               /* B */
    KEY_C,              SGIK_C,               /* C */
    KEY_D,              SGIK_D,               /* D */
    KEY_E,              SGIK_E,               /* E */
    KEY_F,              SGIK_F,               /* F */
    KEY_G,              SGIK_G,               /* G */
    KEY_H,              SGIK_H,               /* H */
    KEY_I,              SGIK_I,               /* I */
    KEY_J,              SGIK_J,               /* J */
    KEY_K,              SGIK_K,               /* K */
    KEY_L,              SGIK_L,               /* L */
    KEY_M,              SGIK_M,               /* M */
    KEY_N,              SGIK_N,               /* N */
    KEY_O,              SGIK_O,               /* O */
    KEY_P,              SGIK_P,               /* P */
    KEY_Q,              SGIK_Q,               /* Q */
    KEY_R,              SGIK_R,               /* R */
    KEY_S,              SGIK_S,               /* S */
    KEY_T,              SGIK_T,               /* T */
    KEY_U,              SGIK_U,               /* U */
    KEY_V,              SGIK_V,               /* V */
    KEY_W,              SGIK_W,               /* W */
    KEY_X,              SGIK_X,               /* X */
    KEY_Y,              SGIK_Y,               /* Y */
    KEY_Z,              SGIK_Z,               /* Z */
    KEY_a,              SGIK_a,               /* A */
    KEY_b,              SGIK_b,               /* B */
    KEY_c,              SGIK_c,               /* C */
    KEY_d,              SGIK_d,               /* D */
    KEY_e,              SGIK_e,               /* E */
    KEY_f,              SGIK_f,               /* F */
    KEY_g,              SGIK_g,               /* G */
    KEY_h,              SGIK_h,               /* H */
    KEY_i,              SGIK_i,               /* I */
    KEY_j,              SGIK_j,               /* J */
    KEY_k,              SGIK_k,               /* K */
    KEY_l,              SGIK_l,               /* L */
    KEY_m,              SGIK_m,               /* M */
    KEY_n,              SGIK_n,               /* N */
    KEY_o,              SGIK_o,               /* O */
    KEY_p,              SGIK_p,               /* P */
    KEY_q,              SGIK_q,               /* Q */
    KEY_r,              SGIK_r,               /* R */
    KEY_s,              SGIK_s,               /* S */
    KEY_t,              SGIK_t,               /* T */
    KEY_u,              SGIK_u,               /* U */
    KEY_v,              SGIK_v,               /* V */
    KEY_w,              SGIK_w,               /* W */
    KEY_x,              SGIK_x,               /* X */
    KEY_y,              SGIK_y,               /* Y */
    KEY_z,              SGIK_z,               /* Z */
    KEY_LEFTSQUAREBRACKET,  SGIK_LEFTSQUAREBRACKET,    /* [ */
    KEY_BACKSLASH,      SGIK_BACKSLASH,      /* \ */
    KEY_RIGHTSQUAREBRACKET, SGIK_RIGHTSQUAREBRACKET,   /* ] */
    KEY_CARET,          SGIK_CARET,    /* ^ */
    KEY_UNDERSCRE,      SGIK_UNDERSCRE,      /* _ */
    KEY_BACKQUOTE,      SGIK_BACKQUOTE,           /* ` */

    KEY_LEFTBRACKET,    SGIK_LEFTBRACKET,      /* { */
    KEY_VERTICAL,       SGIK_VERTICAL,             /* | */
    KEY_RIGHTBRACKET,   SGIK_RIGHTBRACKET,     /* } */
    KEY_TILDE,          SGIK_TILDE,     /* ~ */
    KEY_QUOTE,          SGIK_QUOTE,      /* ' */

    KEY_F1,             SGIK_F1,
    KEY_F2,             SGIK_F2,
    KEY_F3,             SGIK_F3,
    KEY_F4,             SGIK_F4,
    KEY_F5,             SGIK_F5,
    KEY_F6,             SGIK_F6,
    KEY_F7,             SGIK_F7,
    KEY_F8,             SGIK_F8,
    KEY_F9,             SGIK_F9,
    KEY_F10,            SGIK_F10,
    KEY_F11,            SGIK_F11,
    KEY_F12,            SGIK_F12,

    KEY_LSHIFT,         SGIK_LSHIFT,
    KEY_RSHIFT,         SGIK_RCTRL,

    KEY_LCTRL,          SGIK_LCTRL,
    KEY_RCTRL,          SGIK_RCTRL,

    KEY_LALT,           SGIK_LALT,
    KEY_RALT,           SGIK_RALT,

    KEY_LMETA,          SGIK_LMETA,
    KEY_RMETA,          SGIK_RMETA,

    KEY_CAPSLOCK,       SGIK_CAPSLOCK,
    KEY_NUMLOCK,        SGIK_NUMLOCK,
    KEY_SCROLLOCK,      SGIK_SCROLLOCK,

    0xFF,               SGIK_UNKNOWN
};

static unsigned char __scan_code_to_gui_value(int code)
{
    unsigned char  key_value = SGIK_UNKNOWN;
    unsigned int   i         = 0;
    for ( i = 0;  i < sizeof(map_table);  i += 2 ) {
        if (map_table[i] == code) {
            key_value = map_table[i + 1]; // 返回转换后的键值
            break;
        }
    }
    return key_value;
}


/* 特殊按键：
ALT+TAB 切换窗口 */

/* 需要记录一些按键的状态，尤其是组合按键的时候就很有必要了。 */

int __process_special_key(int keycode, int press)
{

    if (keycode == KEY_E || keycode == KEY_e) {
        /* alt + tab */
        if (input_keyboard.key_modify & GUI_KMOD_ALT_L) {
            if (press) {
#if DEBUG_LOCAL == 1                
                /* switch window */
                printf("[keyboard] [alt + tab] switch window.\n");
#endif
            }
            return 1;
        }
    }
    return 0;
}

int __key_pressed(int keycode)
{
#if DEBUG_LOCAL == 1
    printf("[keyboard ] key %x->%c pressed.\n", keycode, keycode);
#endif
    /* 处理修饰按键 */
    if (keycode == KEY_NUMLOCK) {
        /* 如果数字锁按键已经置位，那么就清除 */
        if (input_keyboard.key_modify & GUI_KMOD_NUM) {
            input_keyboard.key_modify &= ~GUI_KMOD_NUM;
        } else {    /* 没有则添加数字锁 */
            input_keyboard.key_modify |= GUI_KMOD_NUM;    
        }
    }
    if (keycode == KEY_CAPSLOCK) {
        /* 如果大写锁按键已经置位，那么就清除 */
        if (input_keyboard.key_modify & GUI_KMOD_CAPS) {
            input_keyboard.key_modify &= ~GUI_KMOD_CAPS;
        } else {    /* 没有则添加数字锁 */
            input_keyboard.key_modify |= GUI_KMOD_CAPS;    
        }
    }
    
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_LSHIFT:    /* left shift */
        input_keyboard.key_modify |= GUI_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        input_keyboard.key_modify |= GUI_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        input_keyboard.key_modify |= GUI_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        input_keyboard.key_modify |= GUI_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        input_keyboard.key_modify |= GUI_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        input_keyboard.key_modify |= GUI_KMOD_CTRL_R;
        break;
    default:
        break;
    }
    
    /* 如果是一些特殊按键，就做预处理 */
    if (__process_special_key(keycode, 1))
        return 0;

    /* 发送给指定窗口 */
    if (current_window) {   /* 当前窗口有数据 */
        if (current_window->display_id > 0) {   /* 发送给客户端窗口 */
#if DEBUG_LOCAL == 1
            printf("[keyboard] send event to client window.\n");
#endif
            /* 发送消息到窗口 */ 
            gui_event_t event;
            event.type = SGI_KEY;
            event.key.state = SGI_PRESSED;
            event.key.keycode.code = __scan_code_to_gui_value(keycode);
            event.key.keycode.modify = input_keyboard.key_modify; /* 按键修饰 */
            gui_window_send_event(current_window, &event);
        } else {    /* 发送给服务端窗口 */
#if DEBUG_LOCAL == 1
            printf("[keyboard] send event to server window.\n");
#endif
        }   
    }

    return 0;
}

int __key_released(int keycode)
{
#if DEBUG_LOCAL == 1
    printf("[keyboard ] key %x->%c released.\n", keycode, keycode);
#endif
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_LSHIFT:    /* left shift */
        input_keyboard.key_modify &= ~GUI_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        input_keyboard.key_modify &= ~GUI_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        input_keyboard.key_modify &= ~GUI_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        input_keyboard.key_modify &= ~GUI_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        input_keyboard.key_modify &= ~GUI_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        input_keyboard.key_modify &= ~GUI_KMOD_CTRL_R;
        break;
    default:
        break;
    }

    /* 如果是一些特殊按键，就做预处理 */
    if (__process_special_key(keycode, 0))
        return 0;

    /* 发送给指定窗口 */
    if (current_window) {   /* 当前窗口有数据 */
        if (current_window->display_id > 0) {   /* 发送给客户端窗口 */
#if DEBUG_LOCAL == 1
            printf("[keyboard] send event to client window.\n");
#endif
            /* 发送消息到窗口 */
            gui_event_t event;
            event.type = SGI_KEY;
            event.key.state = SGI_RELEASED;
            event.key.keycode.code = __scan_code_to_gui_value(keycode);
            event.key.keycode.modify = input_keyboard.key_modify; /* 按键修饰 */
            gui_window_send_event(current_window, &event);
        } else {    /* 发送给服务端窗口 */
#if DEBUG_LOCAL == 1
            printf("[keyboard] send event to server window.\n");
#endif
        }
    }
    return 0;
}

int init_keyboard_input()
{
    input_keyboard.key_modify = drv_keyboard.ledstate;
    input_keyboard.key_pressed = __key_pressed;
    input_keyboard.key_released = __key_released;
    return 0;
}