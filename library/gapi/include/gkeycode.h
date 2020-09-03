
#ifndef _G_KEYCODE_H
#define _G_KEYCODE_H

/* 按键 */
enum {
    GK_UNKNOWN      = 0,                /* unknown keycode */
    GK_FIRST,                           /* first key */
    GK_CLEAR,     /* clear */
    GK_PAUSE,   /* pause */
    GK_UP,           /* up arrow */
    GK_DOWN,         /* down arrow */
    GK_RIGHT,        /* right arrow */
    GK_LEFT,         /* left arrow */
    GK_BACKSPACE,    /* backspace */
    GK_TAB,          /* 9: tab */
    GK_ENTER,        /* 10: enter */
    GK_INSERT,       /* insert */
    GK_HOME,         /* home */
    GK_END,          /* end */
    GK_PAGEUP,       /* page up */
    GK_PAGEDOWN,     /* page down */
    GK_F1,           /* F1 */
    GK_F2,           /* F2 */
    GK_F3,           /* F3 */
    GK_F4,           /* F4 */
    GK_F5,           /* F5 */
    GK_F6,           /* F6 */
    GK_F7,           /* F7 */
    GK_F8,           /* F8 */
    GK_F9,           /* F9 */
    GK_F10,          /* F10 */
    GK_F11,          /* F11 */
    GK_ESCAPE,          /* 27: escape */
    GK_F12,          /* F12 */
    GK_F13,      /* F13 */
    GK_F14,      /* F14 */
    GK_F15,      /* F15 */
    /* 可显示字符按照ascill码排列 */
    GK_SPACE,              /*  space */
    GK_EXCLAIM,              /* ! exclamation mark */
    GK_QUOTEDBL,              /*" double quote */
    GK_HASH,              /* # hash */
    GK_DOLLAR,              /* $ dollar */
    GK_PERSENT,              /* % persent */
    GK_AMPERSAND,              /* & ampersand */
    GK_QUOTE,             /* ' single quote */
    GK_LEFTPAREN,              /* ( left parenthesis */
    GK_RIGHTPAREN,              /* ) right parenthesis */
    GK_ASTERISK,              /* * asterisk */
    GK_PLUS,              /* + plus sign */
    GK_COMMA,              /* , comma */
    GK_MINUS,              /* - minus sign */
    GK_PERIOD,              /* . period/full stop */
    GK_SLASH,              /* / forward slash */
    GK_0,              /* 0 */
    GK_1,              /* 1 */
    GK_2,              /* 2 */
    GK_3,              /* 3 */
    GK_4,              /* 4 */
    GK_5,              /* 5 */
    GK_6,              /* 6 */
    GK_7,              /* 7 */
    GK_8,              /* 8 */
    GK_9,              /* 9 */
    GK_COLON,              /* : colon */
    GK_SEMICOLON,              /* ;semicolon */
    GK_LESS,              /* < less-than sign */
    GK_EQUALS,              /* = equals sign */
    GK_GREATER,              /* > greater-then sign */
    GK_QUESTION,      /* ? question mark */
    GK_AT,              /* @ at */
    GK_A,              /* A */
    GK_B,              /* B */
    GK_C,              /* C */
    GK_D,              /* D */
    GK_E,              /* E */
    GK_F,              /* F */
    GK_G,              /* G */
    GK_H,              /* H */
    GK_I,              /* I */
    GK_J,              /* J */
    GK_K,              /* K */
    GK_L,              /* L */
    GK_M,              /* M */
    GK_N,              /* N */
    GK_O,              /* O */
    GK_P,              /* P */
    GK_Q,              /* Q */
    GK_R,              /* R */
    GK_S,              /* S */
    GK_T,              /* T */
    GK_U,              /* U */
    GK_V,              /* V */
    GK_W,              /* W */
    GK_X,              /* X */
    GK_Y,              /* Y */
    GK_Z,              /* Z */
    GK_LEFTSQUAREBRACKET,     /* [ left square bracket */
    GK_BACKSLASH,             /* \ backslash */
    GK_RIGHTSQUAREBRACKET,    /* ]right square bracket */
    GK_CARET,              /* ^ caret */
    GK_UNDERSCRE,              /* _ underscore */
    GK_BACKQUOTE,              /* ` grave */
    GK_a,              /* a */
    GK_b,              /* b */
    GK_c,              /* c */
    GK_d,              /* d */
    GK_e,              /* e */
    GK_f,              /* f */
    GK_g,              /* g */
    GK_h,              /* h */
    GK_i,              /* i */
    GK_j,              /* j */
    GK_k,              /* k */
    GK_l,              /* l */
    GK_m,              /* m */
    GK_n,              /* n */
    GK_o,              /* o */
    GK_p,              /* p */
    GK_q,              /* q */
    GK_r,              /* r */
    GK_s,              /* s */
    GK_t,              /* t */
    GK_u,              /* u */
    GK_v,              /* v */
    GK_w,              /* w */
    GK_x,              /* x */
    GK_y,              /* y */
    GK_z,              /* z */
    GK_LEFTBRACKET,              /* { left bracket */
    GK_VERTICAL,              /* | vertical virgul */
    GK_RIGHTBRACKET,              /* } left bracket */
    GK_TILDE,              /* ~ tilde */
    GK_DELETE,       /* 127 delete */
    GK_KP0,        /* keypad 0 */
    GK_KP1,        /* keypad 1 */
    GK_KP2,        /* keypad 2 */
    GK_KP3,        /* keypad 3 */
    GK_KP4,        /* keypad 4 */
    GK_KP5,        /* keypad 5 */
    GK_KP6,        /* keypad 6 */
    GK_KP7,        /* keypad 7 */
    GK_KP8,        /* keypad 8 */
    GK_KP9,        /* keypad 9 */
    GK_KP_PERIOD,      /* keypad period    '.' */
    GK_KP_DIVIDE,    /* keypad divide    '/' */
    GK_KP_MULTIPLY,     /* keypad multiply  '*' */
    GK_KP_MINUS,    /* keypad minus     '-' */
    GK_KP_PLUS,     /* keypad plus      '+' */
    GK_KP_ENTER,    /* keypad enter     '\r'*/
    GK_KP_EQUALS,    /* !keypad equals   '=' */
    GK_NUMLOCK,     /* numlock */
    GK_CAPSLOCK,    /* capslock */
    GK_SCROLLOCK,  /* scrollock */
    GK_RSHIFT,      /* right shift */
    GK_LSHIFT,      /* left shift */
    GK_RCTRL,       /* right ctrl */
    GK_LCTRL,       /* left ctrl */
    GK_RALT,        /* right alt / alt gr */
    GK_LALT,        /* left alt */
    GK_RMETA,   /* right meta */
    GK_LMETA,   /* left meta */
    GK_RSUPER,   /* right windows key */
    GK_LSUPER,   /* left windows key */
    GK_MODE,   /* mode shift */
    GK_COMPOSE,   /* compose */
    GK_HELP,   /* help */
    GK_PRINT,  /* print-screen */
    GK_SYSREQ,   /* sys rq */
    GK_BREAK,   /* break */
    GK_MENU,   /* menu */
    GK_POWER,   /* power */
    GK_EURO,   /* euro */
    GK_UNDO,   /* undo */
    GK_LAST       /* last one */        
};

/* 控制按键状态 */
#define GKMOD_SHIFT_L    0x01
#define GKMOD_SHIFT_R    0x02
#define GKMOD_SHIFT      (GKMOD_SHIFT_L | GKMOD_SHIFT_R)
#define GKMOD_CTRL_L     0x04
#define GKMOD_CTRL_R     0x08
#define GKMOD_CTRL       (GKMOD_CTRL_L | GKMOD_CTRL_R)
#define GKMOD_ALT_L      0x10
#define GKMOD_ALT_R      0x20
#define GKMOD_ALT        (GKMOD_ALT_L | GKMOD_ALT_R)
#define GKMOD_PAD	    0x40
#define GKMOD_NUM	    0x80
#define GKMOD_CAPS	    0x100

#endif /* _G_KEYCODE_H */
