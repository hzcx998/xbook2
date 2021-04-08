#ifndef _UVIEW_UVIEW_KEYCODE_H
#define _UVIEW_UVIEW_KEYCODE_H

#define UVIEW_KMOD_SHIFT_L      0x01
#define UVIEW_KMOD_SHIFT_R      0x02
#define UVIEW_KMOD_SHIFT        (UVIEW_KMOD_SHIFT_L | UVIEW_KMOD_SHIFT_R)
#define UVIEW_KMOD_CTRL_L       0x04
#define UVIEW_KMOD_CTRL_R       0x08
#define UVIEW_KMOD_CTRL         (UVIEW_KMOD_CTRL_L | UVIEW_KMOD_CTRL_R)
#define UVIEW_KMOD_ALT_L        0x10
#define UVIEW_KMOD_ALT_R        0x20
#define UVIEW_KMOD_ALT          (UVIEW_KMOD_ALT_L | UVIEW_KMOD_ALT_R)
#define UVIEW_KMOD_PAD	        0x40
#define UVIEW_KMOD_NUM	        0x80
#define UVIEW_KMOD_CAPS	        0x100

enum {
    UVIEW_KEY_UNKNOWN      = 0,                /* unknown keycode */
    UVIEW_KEY_FIRST,                           /* first key */
    UVIEW_KEY_CLEAR,     /* clear */
    UVIEW_KEY_PAUSE,   /* pause */
    UVIEW_KEY_UP,           /* up arrow */
    UVIEW_KEY_DOWN,         /* down arrow */
    UVIEW_KEY_RIGHT,        /* right arrow */
    UVIEW_KEY_LEFT,         /* left arrow */
    UVIEW_KEY_BACKSPACE,    /* backspace */
    UVIEW_KEY_TAB,          /* 9: tab */
    UVIEW_KEY_ENTER,        /* 10: enter */
    UVIEW_KEY_INSERT,       /* insert */
    UVIEW_KEY_HOME,         /* home */
    UVIEW_KEY_END,          /* end */
    UVIEW_KEY_PAGEUP,       /* page up */
    UVIEW_KEY_PAGEDOWN,     /* page down */
    UVIEW_KEY_F1,           /* F1 */
    UVIEW_KEY_F2,           /* F2 */
    UVIEW_KEY_F3,           /* F3 */
    UVIEW_KEY_F4,           /* F4 */
    UVIEW_KEY_F5,           /* F5 */
    UVIEW_KEY_F6,           /* F6 */
    UVIEW_KEY_F7,           /* F7 */
    UVIEW_KEY_F8,           /* F8 */
    UVIEW_KEY_F9,           /* F9 */
    UVIEW_KEY_F10,          /* F10 */
    UVIEW_KEY_F11,          /* F11 */
    UVIEW_KEY_ESCAPE,          /* 27: escape */
    UVIEW_KEY_F12,          /* F12 */
    UVIEW_KEY_F13,      /* F13 */
    UVIEW_KEY_F14,      /* F14 */
    UVIEW_KEY_F15,      /* F15 */
    /* 可显示字符按照ascill码排列 */
    UVIEW_KEY_SPACE,              /*  space */
    UVIEW_KEY_EXCLAIM,              /* ! exclamation mark */
    UVIEW_KEY_QUOTEDBL,              /*" double quote */
    UVIEW_KEY_HASH,              /* # hash */
    UVIEW_KEY_DOLLAR,              /* $ dollar */
    UVIEW_KEY_PERSENT,              /* % persent */
    UVIEW_KEY_AMPERSAND,              /* & ampersand */
    UVIEW_KEY_QUOTE,             /* ' single quote */
    UVIEW_KEY_LEFTPAREN,              /* ( left parenthesis */
    UVIEW_KEY_RIGHTPAREN,              /* ) right parenthesis */
    UVIEW_KEY_ASTERISK,              /* * asterisk */
    UVIEW_KEY_PLUS,              /* + plus sign */
    UVIEW_KEY_COMMA,              /* , comma */
    UVIEW_KEY_MINUS,              /* - minus sign */
    UVIEW_KEY_PERIOD,              /* . period/full stop */
    UVIEW_KEY_SLASH,              /* / forward slash */
    UVIEW_KEY_0,              /* 0 */
    UVIEW_KEY_1,              /* 1 */
    UVIEW_KEY_2,              /* 2 */
    UVIEW_KEY_3,              /* 3 */
    UVIEW_KEY_4,              /* 4 */
    UVIEW_KEY_5,              /* 5 */
    UVIEW_KEY_6,              /* 6 */
    UVIEW_KEY_7,              /* 7 */
    UVIEW_KEY_8,              /* 8 */
    UVIEW_KEY_9,              /* 9 */
    UVIEW_KEY_COLON,              /* : colon */
    UVIEW_KEY_SEMICOLON,              /* ;semicolon */
    UVIEW_KEY_LESS,              /* < less-than sign */
    UVIEW_KEY_EQUALS,              /* = equals sign */
    UVIEW_KEY_GREATER,              /* > greater-then sign */
    UVIEW_KEY_QUESTION,      /* ? question mark */
    UVIEW_KEY_AT,              /* @ at */
    UVIEW_KEY_A,              /* A */
    UVIEW_KEY_B,              /* B */
    UVIEW_KEY_C,              /* C */
    UVIEW_KEY_D,              /* D */
    UVIEW_KEY_E,              /* E */
    UVIEW_KEY_F,              /* F */
    UVIEW_KEY_G,              /* G */
    UVIEW_KEY_H,              /* H */
    UVIEW_KEY_I,              /* I */
    UVIEW_KEY_J,              /* J */
    UVIEW_KEY_K,              /* K */
    UVIEW_KEY_L,              /* L */
    UVIEW_KEY_M,              /* M */
    UVIEW_KEY_N,              /* N */
    UVIEW_KEY_O,              /* O */
    UVIEW_KEY_P,              /* P */
    UVIEW_KEY_Q,              /* Q */
    UVIEW_KEY_R,              /* R */
    UVIEW_KEY_S,              /* S */
    UVIEW_KEY_T,              /* T */
    UVIEW_KEY_U,              /* U */
    UVIEW_KEY_V,              /* V */
    UVIEW_KEY_W,              /* W */
    UVIEW_KEY_X,              /* X */
    UVIEW_KEY_Y,              /* Y */
    UVIEW_KEY_Z,              /* Z */
    UVIEW_KEY_LEFTSQUAREBRACKET,     /* [ left square bracket */
    UVIEW_KEY_BACKSLASH,             /* \ backslash */
    UVIEW_KEY_RIGHTSQUAREBRACKET,    /* ]right square bracket */
    UVIEW_KEY_CARET,              /* ^ caret */
    UVIEW_KEY_UNDERSCRE,              /* _ underscore */
    UVIEW_KEY_BACKQUOTE,              /* ` grave */
    UVIEW_KEY_a,              /* a */
    UVIEW_KEY_b,              /* b */
    UVIEW_KEY_c,              /* c */
    UVIEW_KEY_d,              /* d */
    UVIEW_KEY_e,              /* e */
    UVIEW_KEY_f,              /* f */
    UVIEW_KEY_g,              /* g */
    UVIEW_KEY_h,              /* h */
    UVIEW_KEY_i,              /* i */
    UVIEW_KEY_j,              /* j */
    UVIEW_KEY_k,              /* k */
    UVIEW_KEY_l,              /* l */
    UVIEW_KEY_m,              /* m */
    UVIEW_KEY_n,              /* n */
    UVIEW_KEY_o,              /* o */
    UVIEW_KEY_p,              /* p */
    UVIEW_KEY_q,              /* q */
    UVIEW_KEY_r,              /* r */
    UVIEW_KEY_s,              /* s */
    UVIEW_KEY_t,              /* t */
    UVIEW_KEY_u,              /* u */
    UVIEW_KEY_v,              /* v */
    UVIEW_KEY_w,              /* w */
    UVIEW_KEY_x,              /* x */
    UVIEW_KEY_y,              /* y */
    UVIEW_KEY_z,              /* z */
    UVIEW_KEY_LEFTBRACKET,              /* { left bracket */
    UVIEW_KEY_VERTICAL,              /* | vertical virgul */
    UVIEW_KEY_RIGHTBRACKET,              /* } left bracket */
    UVIEW_KEY_TILDE,              /* ~ tilde */
    UVIEW_KEY_DELETE,       /* 127 delete */
    UVIEW_KEY_KP0,        /* keypad 0 */
    UVIEW_KEY_KP1,        /* keypad 1 */
    UVIEW_KEY_KP2,        /* keypad 2 */
    UVIEW_KEY_KP3,        /* keypad 3 */
    UVIEW_KEY_KP4,        /* keypad 4 */
    UVIEW_KEY_KP5,        /* keypad 5 */
    UVIEW_KEY_KP6,        /* keypad 6 */
    UVIEW_KEY_KP7,        /* keypad 7 */
    UVIEW_KEY_KP8,        /* keypad 8 */
    UVIEW_KEY_KP9,        /* keypad 9 */
    UVIEW_KEY_KP_PERIOD,      /* keypad period    '.' */
    UVIEW_KEY_KP_DIVIDE,    /* keypad divide    '/' */
    UVIEW_KEY_KP_MULTIPLY,     /* keypad multiply  '*' */
    UVIEW_KEY_KP_MINUS,    /* keypad minus     '-' */
    UVIEW_KEY_KP_PLUS,     /* keypad plus      '+' */
    UVIEW_KEY_KP_ENTER,    /* keypad enter     '\r'*/
    UVIEW_KEY_KP_EQUALS,    /* !keypad equals   '=' */
    UVIEW_KEY_NUMLOCK,     /* numlock */
    UVIEW_KEY_CAPSLOCK,    /* capslock */
    UVIEW_KEY_SCROLLOCK,  /* scrollock */
    UVIEW_KEY_RSHIFT,      /* right shift */
    UVIEW_KEY_LSHIFT,      /* left shift */
    UVIEW_KEY_RCTRL,       /* right ctrl */
    UVIEW_KEY_LCTRL,       /* left ctrl */
    UVIEW_KEY_RALT,        /* right alt / alt gr */
    UVIEW_KEY_LALT,        /* left alt */
    UVIEW_KEY_RMETA,   /* right meta */
    UVIEW_KEY_LMETA,   /* left meta */
    UVIEW_KEY_RSUPER,   /* right windows key */
    UVIEW_KEY_LSUPER,   /* left windows key */
    UVIEW_KEY_MODE,   /* mode shift */
    UVIEW_KEY_COMPOSE,   /* compose */
    UVIEW_KEY_HELP,   /* help */
    UVIEW_KEY_PRINT,  /* print-screen */
    UVIEW_KEY_SYSREQ,   /* sys rq */
    UVIEW_KEY_BREAK,   /* break */
    UVIEW_KEY_MENU,   /* menu */
    UVIEW_KEY_POWER,   /* power */
    UVIEW_KEY_EURO,   /* euro */
    UVIEW_KEY_UNDO,   /* undo */
    UVIEW_BTN_LEFT,  /* mouse left */
    UVIEW_BTN_RIGHT,  /* mouse right */
    UVIEW_BTN_MIDDLE,  /* mouse middle */
    UVIEW_KEY_LAST       /* last one */        
};

#endif /* _UVIEW_UVIEW_KEYCODE_H */