#include <string.h>
#include <stdio.h>
#include <xbook/driver.h>
#include <sys/ioctl.h>
#include <xbook/kmalloc.h>
#include <xbook/vmarea.h>

/// 程序本地头文件
#include <gui/keyboard.h>
#include <gui/event.h>
#include <sys/input.h>

#include <gui/console/console.h>

/* 特殊按键：
ALT+TAB 切换窗口 */

/* 需要记录一些按键的状态，尤其是组合按键的时候就很有必要了。 */

int __process_special_key(int keycode, int press)
{

    if (keycode == KEY_E || keycode == KEY_e) {
        /* alt + tab */
        if (gui_keyboard.key_modify & GUI_KMOD_ALT_L) {
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

/* Keyboard mapping table */
static  const  unsigned char  map_table[] = {
    KEY_ESCAPE,         KEY_ESCAPE,
    KEY_BACKSPACE,      KEY_BACKSPACE,
    KEY_TAB,            KEY_TAB,
    /* 0x00,            KEY_BACK_TAB,*/

    KEY_PAUSE,          KEY_PAUSE,

    KEY_PRINT,          KEY_PRINT,
    KEY_SYSREQ,         KEY_SYSREQ,

    KEY_CLEAR,          KEY_CLEAR,

    KEY_INSERT,         KEY_INSERT,
    KEY_ENTER,          KEY_ENTER,
    KEY_DELETE,         KEY_DELETE,
    KEY_KP_ENTER,       KEY_ENTER,

    KEY_LEFT,           KEY_LEFT,
    KEY_RIGHT,          KEY_RIGHT,
    KEY_UP,             KEY_UP,
    KEY_DOWN,           KEY_DOWN,
    KEY_HOME,           KEY_HOME,
    KEY_END,            KEY_END,
    KEY_PAGEUP,         KEY_PAGEUP,
    KEY_PAGEDOWN,       KEY_PAGEDOWN,

    KEY_KP0,            KEY_INSERT,
    KEY_KP_PERIOD,      KEY_DELETE,
    KEY_KP1,            KEY_END,
    KEY_KP2,            KEY_DOWN,
    KEY_KP3,            KEY_PAGEDOWN,
    KEY_KP4,            KEY_LEFT,
    KEY_KP5,            KEY_5,
    KEY_KP6,            KEY_RIGHT,
    KEY_KP7,            KEY_HOME,
    KEY_KP8,            KEY_UP,
    KEY_KP9,            KEY_PAGEUP,

    KEY_SPACE,          KEY_SPACE,           /*   */
    KEY_EXCLAIM,        KEY_EXCLAIM,         /* ! */
    KEY_QUOTEDBL,       KEY_QUOTEDBL,        /* " */
    KEY_HASH,           KEY_HASH,            /* # */
    KEY_DOLLAR,         KEY_DOLLAR,          /* $ */
    KEY_PERSENT,        KEY_PERSENT,         /* % */
    KEY_AMPERSAND,      KEY_AMPERSAND,       /* & */
    KEY_LEFTPAREN,      KEY_LEFTPAREN,      /* ( */
    KEY_RIGHTPAREN,     KEY_RIGHTPAREN,     /* ) */
    KEY_ASTERISK,       KEY_ASTERISK,        /* * */
    KEY_KP_MULTIPLY,       KEY_ASTERISK,     /* * */
    KEY_PLUS,           KEY_PLUS,            /* + */
    KEY_KP_PLUS,        KEY_PLUS,            /* + */
    KEY_COMMA,          KEY_COMMA,           /* , */
    KEY_MINUS,          KEY_MINUS,           /* - */
    KEY_KP_MINUS,       KEY_MINUS,           /* - */
    KEY_PERIOD,         KEY_PERIOD,             /* . */
    KEY_SLASH,          KEY_SLASH,           /* / */
    KEY_KP_DIVIDE,      KEY_SLASH,           /* / */

    KEY_0,              KEY_0,               /* 0 */
    KEY_1,              KEY_1,               /* 1 */
    KEY_2,              KEY_2,               /* 2 */
    KEY_3,              KEY_3,               /* 3 */
    KEY_4,              KEY_4,               /* 4 */
    KEY_5,              KEY_5,               /* 5 */
    KEY_6,              KEY_6,               /* 6 */
    KEY_7,              KEY_7,               /* 7 */
    KEY_8,              KEY_8,               /* 8 */
    KEY_9,              KEY_9,               /* 9 */
    KEY_KP0,            KEY_0,               /* 0 */
    KEY_KP1,            KEY_1,               /* 1 */
    KEY_KP2,            KEY_2,               /* 2 */
    KEY_KP3,            KEY_3,               /* 3 */
    KEY_KP4,            KEY_4,               /* 4 */
    KEY_KP5,            KEY_5,               /* 5 */
    KEY_KP6,            KEY_6,               /* 6 */
    KEY_KP7,            KEY_7,               /* 7 */
    KEY_KP8,            KEY_8,               /* 8 */
    KEY_KP9,            KEY_9,               /* 9 */
    KEY_COLON,          KEY_COLON,           /* : */
    KEY_SEMICOLON,      KEY_SEMICOLON,       /* ; */
    KEY_LESS,           KEY_LESS,            /* < */
    KEY_EQUALS,         KEY_EQUALS,          /* = */
    KEY_GREATER,        KEY_GREATER,         /* > */
    KEY_QUESTION,       KEY_QUESTION,        /* ? */
    KEY_AT,             KEY_AT,              /* @ */
    KEY_A,              KEY_A,               /* A */
    KEY_B,              KEY_B,               /* B */
    KEY_C,              KEY_C,               /* C */
    KEY_D,              KEY_D,               /* D */
    KEY_E,              KEY_E,               /* E */
    KEY_F,              KEY_F,               /* F */
    KEY_G,              KEY_G,               /* G */
    KEY_H,              KEY_H,               /* H */
    KEY_I,              KEY_I,               /* I */
    KEY_J,              KEY_J,               /* J */
    KEY_K,              KEY_K,               /* K */
    KEY_L,              KEY_L,               /* L */
    KEY_M,              KEY_M,               /* M */
    KEY_N,              KEY_N,               /* N */
    KEY_O,              KEY_O,               /* O */
    KEY_P,              KEY_P,               /* P */
    KEY_Q,              KEY_Q,               /* Q */
    KEY_R,              KEY_R,               /* R */
    KEY_S,              KEY_S,               /* S */
    KEY_T,              KEY_T,               /* T */
    KEY_U,              KEY_U,               /* U */
    KEY_V,              KEY_V,               /* V */
    KEY_W,              KEY_W,               /* W */
    KEY_X,              KEY_X,               /* X */
    KEY_Y,              KEY_Y,               /* Y */
    KEY_Z,              KEY_Z,               /* Z */
    KEY_a,              KEY_a,               /* A */
    KEY_b,              KEY_b,               /* B */
    KEY_c,              KEY_c,               /* C */
    KEY_d,              KEY_d,               /* D */
    KEY_e,              KEY_e,               /* E */
    KEY_f,              KEY_f,               /* F */
    KEY_g,              KEY_g,               /* G */
    KEY_h,              KEY_h,               /* H */
    KEY_i,              KEY_i,               /* I */
    KEY_j,              KEY_j,               /* J */
    KEY_k,              KEY_k,               /* K */
    KEY_l,              KEY_l,               /* L */
    KEY_m,              KEY_m,               /* M */
    KEY_n,              KEY_n,               /* N */
    KEY_o,              KEY_o,               /* O */
    KEY_p,              KEY_p,               /* P */
    KEY_q,              KEY_q,               /* Q */
    KEY_r,              KEY_r,               /* R */
    KEY_s,              KEY_s,               /* S */
    KEY_t,              KEY_t,               /* T */
    KEY_u,              KEY_u,               /* U */
    KEY_v,              KEY_v,               /* V */
    KEY_w,              KEY_w,               /* W */
    KEY_x,              KEY_x,               /* X */
    KEY_y,              KEY_y,               /* Y */
    KEY_z,              KEY_z,               /* Z */
    KEY_LEFTSQUAREBRACKET,  KEY_LEFTSQUAREBRACKET,    /* [ */
    KEY_BACKSLASH,      KEY_BACKSLASH,      /* \ */
    KEY_RIGHTSQUAREBRACKET, KEY_RIGHTSQUAREBRACKET,   /* ] */
    KEY_CARET,          KEY_CARET,    /* ^ */
    KEY_UNDERSCRE,      KEY_UNDERSCRE,      /* _ */
    KEY_BACKQUOTE,      KEY_BACKQUOTE,           /* ` */

    KEY_LEFTBRACKET,    KEY_LEFTBRACKET,      /* { */
    KEY_VERTICAL,       KEY_VERTICAL,             /* | */
    KEY_RIGHTBRACKET,   KEY_RIGHTBRACKET,     /* } */
    KEY_TILDE,          KEY_TILDE,     /* ~ */
    KEY_QUOTE,          KEY_QUOTE,      /* ' */

    KEY_F1,             KEY_F1,
    KEY_F2,             KEY_F2,
    KEY_F3,             KEY_F3,
    KEY_F4,             KEY_F4,
    KEY_F5,             KEY_F5,
    KEY_F6,             KEY_F6,
    KEY_F7,             KEY_F7,
    KEY_F8,             KEY_F8,
    KEY_F9,             KEY_F9,
    KEY_F10,            KEY_F10,
    KEY_F11,            KEY_F11,
    KEY_F12,            KEY_F12,

    KEY_LSHIFT,         KEY_LSHIFT,
    KEY_RSHIFT,         KEY_RCTRL,

    KEY_LCTRL,          KEY_LCTRL,
    KEY_RCTRL,          KEY_RCTRL,

    KEY_LALT,           KEY_LALT,
    KEY_RALT,           KEY_RALT,

    KEY_LMETA,          KEY_LMETA,
    KEY_RMETA,          KEY_RMETA,

    KEY_CAPSLOCK,       KEY_CAPSLOCK,
    KEY_NUMLOCK,        KEY_NUMLOCK,
    KEY_SCROLLOCK,      KEY_SCROLLOCK,

    0xFF,               KEY_UNKNOWN
};
/**
 * 键值转换
 */
static unsigned char code_switch(int code)
{
    unsigned char  key_value = '?';
    unsigned int   i         = 0;
    for ( i = 0;  i < sizeof(map_table);  i += 2 ) {
        if (map_table[i] == code) {
            key_value = map_table[i + 1]; // 返回转换后的键值
            break;
        }
    }
    return key_value;
}

int gui_key_pressed(int keycode)
{
#if DEBUG_LOCAL == 1
    printf("[keyboard ] key %x->%c pressed.\n", keycode, keycode);
#endif
    /* 处理修饰按键 */
    if (keycode == KEY_NUMLOCK) {
        /* 如果数字锁按键已经置位，那么就清除 */
        if (gui_keyboard.key_modify & GUI_KMOD_NUM) {
            gui_keyboard.key_modify &= ~GUI_KMOD_NUM;
        } else {    /* 没有则添加数字锁 */
            gui_keyboard.key_modify |= GUI_KMOD_NUM;    
        }
    }
    if (keycode == KEY_CAPSLOCK) {
        /* 如果大写锁按键已经置位，那么就清除 */
        if (gui_keyboard.key_modify & GUI_KMOD_CAPS) {
            gui_keyboard.key_modify &= ~GUI_KMOD_CAPS;
        } else {    /* 没有则添加数字锁 */
            gui_keyboard.key_modify |= GUI_KMOD_CAPS;    
        }
    }
    
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_LSHIFT:    /* left shift */
        gui_keyboard.key_modify |= GUI_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        gui_keyboard.key_modify |= GUI_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        gui_keyboard.key_modify |= GUI_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        gui_keyboard.key_modify |= GUI_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        gui_keyboard.key_modify |= GUI_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        gui_keyboard.key_modify |= GUI_KMOD_CTRL_R;
        break;
    default:
        break;
    }
    
    /* 如果是一些特殊按键，就做预处理 */
    if (__process_special_key(keycode, 1))
        return -1;

    /* 使用按键,keydown, keycode, modify */
    /*gui_keyboard.keyevent.state = 0;
    gui_keyboard.keyevent.code = code_switch(keycode);
    gui_keyboard.keyevent.modify = gui_keyboard.key_modify;
    return 0;*/

    gui_event e;
    e.type = GUI_EVENT_KEY;
    e.key.code = code_switch(keycode);
    e.key.modify = gui_keyboard.key_modify;
    e.key.state = GUI_PRESSED;
    return gui_event_add(&e);
}

int gui_key_released(int keycode)
{
#if DEBUG_LOCAL == 1
    printf("[keyboard ] key %x->%c released.\n", keycode, keycode);
#endif
    /* 处理CTRL, ALT, SHIFT*/
    switch (keycode) {
    case KEY_LSHIFT:    /* left shift */
        gui_keyboard.key_modify &= ~GUI_KMOD_SHIFT_L;
        break;
    case KEY_RSHIFT:    /* right shift */
        gui_keyboard.key_modify &= ~GUI_KMOD_SHIFT_R;
        break;
    case KEY_LALT:    /* left alt */
        gui_keyboard.key_modify &= ~GUI_KMOD_ALT_L;
        break;
    case KEY_RALT:    /* right alt */
        gui_keyboard.key_modify &= ~GUI_KMOD_ALT_R;
        break;
    case KEY_LCTRL:    /* left ctl */
        gui_keyboard.key_modify &= ~GUI_KMOD_CTRL_L;
        break;
    case KEY_RCTRL:    /* right ctl */
        gui_keyboard.key_modify &= ~GUI_KMOD_CTRL_R;
        break;
    default:
        break;
    }

    /* 如果是一些特殊按键，就做预处理 */
    if (__process_special_key(keycode, 0))
        return -1;

    /* 使用按键,keyup, keycode, modify */
    /*gui_keyboard.keyevent.state = 1;
    gui_keyboard.keyevent.code = code_switch(keycode);
    gui_keyboard.keyevent.modify = gui_keyboard.key_modify;
    */
    gui_event e;
    e.type = GUI_EVENT_KEY;
    e.key.code = code_switch(keycode);
    e.key.modify = gui_keyboard.key_modify;
    e.key.state = GUI_RELEASED;
    return gui_event_add(&e);
}
