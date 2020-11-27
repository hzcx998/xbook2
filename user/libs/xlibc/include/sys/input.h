#ifndef _SYS_INPUT_H
#define _SYS_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct input_event {
    uint16_t type;
    uint16_t code;
    uint32_t value;
} input_event_t;

#define EV_SYN              0x00
#define EV_KEY              0x01
#define EV_REL              0x02
#define EV_ABS              0x03 //绝对坐标
#define EV_MSC              0x04 //其他

/* 按键 */
enum input_key_code {
    KEY_UNKNOWN      = 0,                /* unknown keycode */
    KEY_FIRST,                           /* first key */
    KEY_CLEAR,     /* clear */
    KEY_PAUSE,   /* pause */
    KEY_UP,           /* up arrow */
    KEY_DOWN,         /* down arrow */
    KEY_RIGHT,        /* right arrow */
    KEY_LEFT,         /* left arrow */
    KEY_BACKSPACE,    /* backspace */
    KEY_TAB,          /* 9: tab */
    KEY_ENTER,        /* 10: enter */
    KEY_INSERT,       /* insert */
    KEY_HOME,         /* home */
    KEY_END,          /* end */
    KEY_PAGEUP,       /* page up */
    KEY_PAGEDOWN,     /* page down */
    KEY_F1,           /* F1 */
    KEY_F2,           /* F2 */
    KEY_F3,           /* F3 */
    KEY_F4,           /* F4 */
    KEY_F5,           /* F5 */
    KEY_F6,           /* F6 */
    KEY_F7,           /* F7 */
    KEY_F8,           /* F8 */
    KEY_F9,           /* F9 */
    KEY_F10,          /* F10 */
    KEY_F11,          /* F11 */
    KEY_ESCAPE,          /* 27: escape */
    KEY_F12,          /* F12 */
    KEY_F13,      /* F13 */
    KEY_F14,      /* F14 */
    KEY_F15,      /* F15 */
    /* 可显示字符按照ascill码排列 */
    KEY_SPACE,              /*  space */
    KEY_EXCLAIM,              /* ! exclamation mark */
    KEY_QUOTEDBL,              /*" double quote */
    KEY_HASH,              /* # hash */
    KEY_DOLLAR,              /* $ dollar */
    KEY_PERSENT,              /* % persent */
    KEY_AMPERSAND,              /* & ampersand */
    KEY_QUOTE,             /* ' single quote */
    KEY_LEFTPAREN,              /* ( left parenthesis */
    KEY_RIGHTPAREN,              /* ) right parenthesis */
    KEY_ASTERISK,              /* * asterisk */
    KEY_PLUS,              /* + plus sign */
    KEY_COMMA,              /* , comma */
    KEY_MINUS,              /* - minus sign */
    KEY_PERIOD,              /* . period/full stop */
    KEY_SLASH,              /* / forward slash */
    KEY_0,              /* 0 */
    KEY_1,              /* 1 */
    KEY_2,              /* 2 */
    KEY_3,              /* 3 */
    KEY_4,              /* 4 */
    KEY_5,              /* 5 */
    KEY_6,              /* 6 */
    KEY_7,              /* 7 */
    KEY_8,              /* 8 */
    KEY_9,              /* 9 */
    KEY_COLON,              /* : colon */
    KEY_SEMICOLON,              /* ;semicolon */
    KEY_LESS,              /* < less-than sign */
    KEY_EQUALS,              /* = equals sign */
    KEY_GREATER,              /* > greater-then sign */
    KEY_QUESTION,      /* ? question mark */
    KEY_AT,              /* @ at */
    KEY_A,              /* A */
    KEY_B,              /* B */
    KEY_C,              /* C */
    KEY_D,              /* D */
    KEY_E,              /* E */
    KEY_F,              /* F */
    KEY_G,              /* G */
    KEY_H,              /* H */
    KEY_I,              /* I */
    KEY_J,              /* J */
    KEY_K,              /* K */
    KEY_L,              /* L */
    KEY_M,              /* M */
    KEY_N,              /* N */
    KEY_O,              /* O */
    KEY_P,              /* P */
    KEY_Q,              /* Q */
    KEY_R,              /* R */
    KEY_S,              /* S */
    KEY_T,              /* T */
    KEY_U,              /* U */
    KEY_V,              /* V */
    KEY_W,              /* W */
    KEY_X,              /* X */
    KEY_Y,              /* Y */
    KEY_Z,              /* Z */
    KEY_LEFTSQUAREBRACKET,     /* [ left square bracket */
    KEY_BACKSLASH,             /* \ backslash */
    KEY_RIGHTSQUAREBRACKET,    /* ]right square bracket */
    KEY_CARET,              /* ^ caret */
    KEY_UNDERSCRE,              /* _ underscore */
    KEY_BACKQUOTE,              /* ` grave */
    KEY_a,              /* a */
    KEY_b,              /* b */
    KEY_c,              /* c */
    KEY_d,              /* d */
    KEY_e,              /* e */
    KEY_f,              /* f */
    KEY_g,              /* g */
    KEY_h,              /* h */
    KEY_i,              /* i */
    KEY_j,              /* j */
    KEY_k,              /* k */
    KEY_l,              /* l */
    KEY_m,              /* m */
    KEY_n,              /* n */
    KEY_o,              /* o */
    KEY_p,              /* p */
    KEY_q,              /* q */
    KEY_r,              /* r */
    KEY_s,              /* s */
    KEY_t,              /* t */
    KEY_u,              /* u */
    KEY_v,              /* v */
    KEY_w,              /* w */
    KEY_x,              /* x */
    KEY_y,              /* y */
    KEY_z,              /* z */
    KEY_LEFTBRACKET,              /* { left bracket */
    KEY_VERTICAL,              /* | vertical virgul */
    KEY_RIGHTBRACKET,              /* } left bracket */
    KEY_TILDE,              /* ~ tilde */
    KEY_DELETE,       /* 127 delete */
    KEY_KP0,        /* keypad 0 */
    KEY_KP1,        /* keypad 1 */
    KEY_KP2,        /* keypad 2 */
    KEY_KP3,        /* keypad 3 */
    KEY_KP4,        /* keypad 4 */
    KEY_KP5,        /* keypad 5 */
    KEY_KP6,        /* keypad 6 */
    KEY_KP7,        /* keypad 7 */
    KEY_KP8,        /* keypad 8 */
    KEY_KP9,        /* keypad 9 */
    KEY_KP_PERIOD,      /* keypad period    '.' */
    KEY_KP_DIVIDE,    /* keypad divide    '/' */
    KEY_KP_MULTIPLY,     /* keypad multiply  '*' */
    KEY_KP_MINUS,    /* keypad minus     '-' */
    KEY_KP_PLUS,     /* keypad plus      '+' */
    KEY_KP_ENTER,    /* keypad enter     '\r'*/
    KEY_KP_EQUALS,    /* !keypad equals   '=' */
    KEY_NUMLOCK,     /* numlock */
    KEY_CAPSLOCK,    /* capslock */
    KEY_SCROLLOCK,  /* scrollock */
    KEY_RSHIFT,      /* right shift */
    KEY_LSHIFT,      /* left shift */
    KEY_RCTRL,       /* right ctrl */
    KEY_LCTRL,       /* left ctrl */
    KEY_RALT,        /* right alt / alt gr */
    KEY_LALT,        /* left alt */
    KEY_RMETA,   /* right meta */
    KEY_LMETA,   /* left meta */
    KEY_RSUPER,   /* right windows key */
    KEY_LSUPER,   /* left windows key */
    KEY_MODE,   /* mode shift */
    KEY_COMPOSE,   /* compose */
    KEY_HELP,   /* help */
    KEY_PRINT,  /* print-screen */
    KEY_SYSREQ,   /* sys rq */
    KEY_BREAK,   /* break */
    KEY_MENU,   /* menu */
    KEY_POWER,   /* power */
    KEY_EURO,   /* euro */
    KEY_UNDO,   /* undo */
    BTN_LEFT,  /* mouse left */
    BTN_RIGHT,  /* mouse right */
    BTN_MIDDLE,  /* mouse middle */
    KEY_LAST       /* last one */        
};

/* 相对坐标 */
enum input_rel_code {
    REL_MISC = 0,
    REL_X,
    REL_Y,
    REL_WHEEL
};

#ifdef __cplusplus
}
#endif

#endif   /* _SYS_INPUT_H */