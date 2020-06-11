#ifndef __SGI_KEYCODE_H__    /* keycode */
#define __SGI_KEYCODE_H__

/* 图形按键修饰 */
enum SGI_KeyModify {
    SGI_KMOD_NONE = 0,        /* 无按键修饰 */

    SGI_KMOD_NUM = 0x01,      /* 数字键 */
    SGI_KMOD_CAPS = 0x02,     /* 大写键 */

    SGI_KMOD_LCTRL = 0x04,    /* 左ctrl */
    SGI_KMOD_RCTRL = 0x08,    /* 右ctrl */
    SGI_KMOD_CTRL = (SGI_KMOD_LCTRL | SGI_KMOD_RCTRL),     /* 任意ctrl */
    
    SGI_KMOD_LSHIFT = 0x20,   /* 左shift */
    SGI_KMOD_RSHIFT = 0x40,   /* 右shift */
    SGI_KMOD_SHIFT = (SGI_KMOD_LSHIFT | SGI_KMOD_RSHIFT),    /* 任意shift */
    
    SGI_KMOD_LALT = 0x100,    /* 左alt */
    SGI_KMOD_RALT = 0x200,    /* 右alt */
    SGI_KMOD_ALT = (SGI_KMOD_LALT | SGI_KMOD_RALT),     /* 任意alt */

    SGI_KMOD_PAD = 0x400,    /* 小键盘按键 */    
};

/* 内核图形按键信息 */
typedef struct _SGI_KeyInfo {
    int scancode;       /* 扫描码 */
    int code;           /* 键值 */
    int modify;         /* 修饰按键 */
} SGI_KeyInfo;

/* 输入键值定义0~512,0x200 */
enum SGI_KeyCode {
    SGIK_UNKNOWN      = 0,                /* unknown keycode */
    SGIK_FIRST,                           /* first key */
    SGIK_CLEAR,     /* clear */
    SGIK_PAUSE,   /* pause */
    SGIK_UP,           /* up arrow */
    SGIK_DOWN,         /* down arrow */
    SGIK_RIGHT,        /* right arrow */
    SGIK_LEFT,         /* left arrow */
    SGIK_BACKSPACE,    /* backspace */
    SGIK_TAB,          /* 9: tab */
    SGIK_INSERT,       /* insert */
    SGIK_HOME,         /* home */
    SGIK_END,          /* end */
    SGIK_ENTER,        /* 13: enter */
    SGIK_PAGEUP,       /* page up */
    SGIK_PAGEDOWN,     /* page down */
    SGIK_F1,           /* F1 */
    SGIK_F2,           /* F2 */
    SGIK_F3,           /* F3 */
    SGIK_F4,           /* F4 */
    SGIK_F5,           /* F5 */
    SGIK_F6,           /* F6 */
    SGIK_F7,           /* F7 */
    SGIK_F8,           /* F8 */
    SGIK_F9,           /* F9 */
    SGIK_F10,          /* F10 */
    SGIK_F11,          /* F11 */
    SGIK_ESCAPE,          /* 27: escape */
    SGIK_F12,          /* F12 */
    SGIK_F13,      /* F13 */
    SGIK_F14,      /* F14 */
    SGIK_F15,      /* F15 */
    /* 可显示字符按照ascill码排列 */
    SGIK_SPACE,              /*  space */
    SGIK_EXCLAIM,              /* ! exclamation mark */
    SGIK_QUOTEDBL,              /*" double quote */
    SGIK_HASH,              /* # hash */
    SGIK_DOLLAR,              /* $ dollar */
    SGIK_PERSENT,              /* % persent */
    SGIK_AMPERSAND,              /* & ampersand */
    SGIK_QUOTE,             /* ' single quote */
    SGIK_LEFTPAREN,              /* ( left parenthesis */
    SGIK_RIGHTPAREN,              /* ) right parenthesis */
    SGIK_ASTERISK,              /* * asterisk */
    SGIK_PLUS,              /* + plus sign */
    SGIK_COMMA,              /* , comma */
    SGIK_MINUS,              /* - minus sign */
    SGIK_PERIOD,              /* . period/full stop */
    SGIK_SLASH,              /* / forward slash */
    SGIK_0,              /* 0 */
    SGIK_1,              /* 1 */
    SGIK_2,              /* 2 */
    SGIK_3,              /* 3 */
    SGIK_4,              /* 4 */
    SGIK_5,              /* 5 */
    SGIK_6,              /* 6 */
    SGIK_7,              /* 7 */
    SGIK_8,              /* 8 */
    SGIK_9,              /* 9 */
    SGIK_COLON,              /* : colon */
    SGIK_SEMICOLON,              /* ;semicolon */
    SGIK_LESS,              /* < less-than sign */
    SGIK_EQUALS,              /* = equals sign */
    SGIK_GREATER,              /* > greater-then sign */
    SGIK_QUESTION,      /* ? question mark */
    SGIK_AT,              /* @ at */
    SGIK_A,              /* A */
    SGIK_B,              /* B */
    SGIK_C,              /* C */
    SGIK_D,              /* D */
    SGIK_E,              /* E */
    SGIK_F,              /* F */
    SGIK_G,              /* G */
    SGIK_H,              /* H */
    SGIK_I,              /* I */
    SGIK_J,              /* J */
    SGIK_K,              /* K */
    SGIK_L,              /* L */
    SGIK_M,              /* M */
    SGIK_N,              /* N */
    SGIK_O,              /* O */
    SGIK_P,              /* P */
    SGIK_Q,              /* Q */
    SGIK_R,              /* R */
    SGIK_S,              /* S */
    SGIK_T,              /* T */
    SGIK_U,              /* U */
    SGIK_V,              /* V */
    SGIK_W,              /* W */
    SGIK_X,              /* X */
    SGIK_Y,              /* Y */
    SGIK_Z,              /* Z */
    SGIK_LEFTSQUAREBRACKET,     /* [ left square bracket */
    SGIK_BACKSLASH,             /* \ backslash */
    SGIK_RIGHTSQUAREBRACKET,    /* ]right square bracket */
    SGIK_CARET,              /* ^ caret */
    SGIK_UNDERSCRE,              /* _ underscore */
    SGIK_BACKQUOTE,              /* ` grave */
    SGIK_a,              /* a */
    SGIK_b,              /* b */
    SGIK_c,              /* c */
    SGIK_d,              /* d */
    SGIK_e,              /* e */
    SGIK_f,              /* f */
    SGIK_g,              /* g */
    SGIK_h,              /* h */
    SGIK_i,              /* i */
    SGIK_j,              /* j */
    SGIK_k,              /* k */
    SGIK_l,              /* l */
    SGIK_m,              /* m */
    SGIK_n,              /* n */
    SGIK_o,              /* o */
    SGIK_p,              /* p */
    SGIK_q,              /* q */
    SGIK_r,              /* r */
    SGIK_s,              /* s */
    SGIK_t,              /* t */
    SGIK_u,              /* u */
    SGIK_v,              /* v */
    SGIK_w,              /* w */
    SGIK_x,              /* x */
    SGIK_y,              /* y */
    SGIK_z,              /* z */
    SGIK_LEFTBRACKET,              /* { left bracket */
    SGIK_VERTICAL,              /* | vertical virgul */
    SGIK_RIGHTBRACKET,              /* } left bracket */
    SGIK_TILDE,              /* ~ tilde */
    SGIK_DELETE,       /* 127 delete */
    SGIK_KP0,        /* keypad 0 */
    SGIK_KP1,        /* keypad 1 */
    SGIK_KP2,        /* keypad 2 */
    SGIK_KP3,        /* keypad 3 */
    SGIK_KP4,        /* keypad 4 */
    SGIK_KP5,        /* keypad 5 */
    SGIK_KP6,        /* keypad 6 */
    SGIK_KP7,        /* keypad 7 */
    SGIK_KP8,        /* keypad 8 */
    SGIK_KP9,        /* keypad 9 */
    SGIK_KP_PERIOD,      /* keypad period    '.' */
    SGIK_KP_DIVIDE,    /* keypad divide    '/' */
    SGIK_KP_MULTIPLY,     /* keypad multiply  '*' */
    SGIK_KP_MINUS,    /* keypad minus     '-' */
    SGIK_KP_PLUS,     /* keypad plus      '+' */
    SGIK_KP_ENTER,    /* keypad enter     '\r'*/
    SGIK_KP_EQUALS,    /* !keypad equals   '=' */
    SGIK_NUMLOCK,     /* numlock */
    SGIK_CAPSLOCK,    /* capslock */
    SGIK_SCROLLOCK,  /* scrollock */
    SGIK_RSHIFT,      /* right shift */
    SGIK_LSHIFT,      /* left shift */
    SGIK_RCTRL,       /* right ctrl */
    SGIK_LCTRL,       /* left ctrl */
    SGIK_RALT,        /* right alt / alt gr */
    SGIK_LALT,        /* left alt */
    SGIK_RMETA,   /* right meta */
    SGIK_LMETA,   /* left meta */
    SGIK_RSUPER,   /* right windows key */
    SGIK_LSUPER,   /* left windows key */
    SGIK_MODE,   /* mode shift */
    SGIK_COMPOSE,   /* compose */
    SGIK_HELP,   /* help */
    SGIK_PRINT,  /* print-screen */
    SGIK_SYSREQ,   /* sys rq */
    SGIK_BREAK,   /* break */
    SGIK_MENU,   /* menu */
    SGIK_POWER,   /* power */
    SGIK_EURO,   /* euro */
    SGIK_UNDO,   /* undo */
    SGIK_LAST       /* last one */        
};

/* 控制标志 */
enum SGI_KeycodeFlag {
    SGIK_FLAG_KEY_MASK  = 0x1FF,        /* 键值的mask值 */
    SGIK_FLAG_SHIFT_L   = 0x0200,		/* Shift key			*/
    SGIK_FLAG_SHIFT_R   = 0x0400,		/* Shift key			*/
    SGIK_FLAG_CTRL_L    = 0x0800,		/* Control key			*/
    SGIK_FLAG_CTRL_R    = 0x1000,		/* Control key			*/
    SGIK_FLAG_ALT_L     = 0x2000,		/* Alternate key		*/
    SGIK_FLAG_ALT_R     = 0x4000,		/* Alternate key		*/
    SGIK_FLAG_PAD	    = 0x8000,		/* keys in num pad		*/
    SGIK_FLAG_NUM	    = 0x10000,	    /* 数字锁		*/
    SGIK_FLAG_CAPS	    = 0x20000,	    /* 数字锁		*/
    SGIK_FLAG_BREAK	    = 0x40000,		/* Break Code   */
};

#endif  /* __SGI_KEYCODE_H__ */