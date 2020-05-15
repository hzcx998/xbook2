#include  <string.h>
#include  <unistd.h>
#include  <sys/input.h>
#include  <sys/res.h>
#include  <sys/ioctl.h>

#include  "driver_keyboard.h"

#include  <message.h>
#include  <keyboard.h>
#include  <driver.h>



#define _LG_KEYBOARD_



#ifdef    _LG_KEYBOARD_


#ifndef  GUI_KEYBOARD_DEVICE_NAME 
#define  GUI_KEYBOARD_DEVICE_NAME         "kbd"
#endif


/*
#define  KEY_EXCLAM			    195
#define  KEY_AT			        196
#define  KEY_NUMBER_SIGN        197
#define  KEY_ASCII_CIRCUM       198
#define  KEY_PERCENT            199

#define  KEY_COLON              248
#define  KEY_LESS               249
#define  KEY_GREATER            250

#define  KEY_CHDOLLAR           251
*/


/* Keyboard mapping table */
static  const  unsigned char  map_table[] = {
    KEY_ESCAPE,         GUI_KEY_ESCAPE,
    KEY_BACKSPACE,      GUI_KEY_BACKSPACE,
    KEY_TAB,            GUI_KEY_TAB,
    /* 0x00,            GUI_KEY_BACK_TAB,*/

    KEY_PAUSE,          GUI_KEY_PAUSE,

    KEY_PRINT,          GUI_KEY_PRINT,
    KEY_SYSREQ,         GUI_KEY_SYS_REQ,

    KEY_CLEAR,          GUI_KEY_CLEAR,

    KEY_INSERT,         GUI_KEY_INSERT,
    KEY_ENTER,          GUI_KEY_ENTER,
    KEY_DELETE,         GUI_KEY_DELETE,
    KEY_KP_ENTER,        GUI_KEY_ENTER,

    KEY_LEFT,           GUI_KEY_LEFT,
    KEY_RIGHT,          GUI_KEY_RIGHT,
    KEY_UP,             GUI_KEY_UP,
    KEY_DOWN,           GUI_KEY_DOWN,
    KEY_HOME,           GUI_KEY_HOME,
    KEY_END,            GUI_KEY_END,
    KEY_PAGEUP,         GUI_KEY_PAGE_UP,
    KEY_PAGEDOWN,       GUI_KEY_PAGE_DOWN,

    KEY_KP0,            GUI_KEY_INSERT,
    KEY_KP_PERIOD,      GUI_KEY_DELETE,
    KEY_KP1,            GUI_KEY_END,
    KEY_KP2,            GUI_KEY_DOWN,
    KEY_KP3,            GUI_KEY_PAGE_DOWN,
    KEY_KP4,            GUI_KEY_LEFT,
    KEY_KP5,            GUI_KEY_5,
    KEY_KP6,            GUI_KEY_RIGHT,
    KEY_KP7,            GUI_KEY_HOME,
    KEY_KP8,            GUI_KEY_UP,
    KEY_KP9,            GUI_KEY_PAGE_UP,

    KEY_SPACE,          GUI_KEY_SPACE,           /*   */
    KEY_EXCLAIM,        GUI_KEY_EXCLAM,          /* ! */
    KEY_QUOTEDBL,       GUI_KEY_QUOTE_DBL,       /* " */
    KEY_HASH,           GUI_KEY_NUMBER_SIGN,     /* # */
    KEY_DOLLAR,         GUI_KEY_DOLLAR,          /* $ */
    KEY_PERSENT,        GUI_KEY_PERCENT,         /* % */
    KEY_AMPERSAND,      GUI_KEY_AMPERSAND,       /* & */
    KEY_BACKQUOTE,      GUI_KEY_GRAVE,           /* ` */
    KEY_LEFTPAREN,      GUI_KEY_PAREN_LEFT,      /* ( */
    KEY_RIGHTPAREN,     GUI_KEY_PAREN_RIGHT,     /* ) */
    KEY_ASTERISK,       GUI_KEY_ASTERISK,        /* * */
    KEY_KP_MULTIPLY,       GUI_KEY_ASTERISK,     /* * */
    KEY_PLUS,           GUI_KEY_PLUS,            /* + */
    KEY_KP_PLUS,        GUI_KEY_PLUS,            /* + */
    KEY_COMMA,          GUI_KEY_COMMA,           /* , */
    KEY_MINUS,          GUI_KEY_MINUS,           /* - */
    KEY_KP_MINUS,       GUI_KEY_MINUS,           /* - */
    KEY_PERIOD,         GUI_KEY_DOT,             /* . */
    KEY_SLASH,          GUI_KEY_SLASH,           /* / */
    KEY_KP_DIVIDE,      GUI_KEY_SLASH,           /* / */

    KEY_0,              GUI_KEY_0,               /* 0 */
    KEY_1,              GUI_KEY_1,               /* 1 */
    KEY_2,              GUI_KEY_2,               /* 2 */
    KEY_3,              GUI_KEY_3,               /* 3 */
    KEY_4,              GUI_KEY_4,               /* 4 */
    KEY_5,              GUI_KEY_5,               /* 5 */
    KEY_6,              GUI_KEY_6,               /* 6 */
    KEY_7,              GUI_KEY_7,               /* 7 */
    KEY_8,              GUI_KEY_8,               /* 8 */
    KEY_9,              GUI_KEY_9,               /* 9 */
    KEY_KP0,            GUI_KEY_0,               /* 0 */
    KEY_KP1,            GUI_KEY_1,               /* 1 */
    KEY_KP2,            GUI_KEY_2,               /* 2 */
    KEY_KP3,            GUI_KEY_3,               /* 3 */
    KEY_KP4,            GUI_KEY_4,               /* 4 */
    KEY_KP5,            GUI_KEY_5,               /* 5 */
    KEY_KP6,            GUI_KEY_6,               /* 6 */
    KEY_KP7,            GUI_KEY_7,               /* 7 */
    KEY_KP8,            GUI_KEY_8,               /* 8 */
    KEY_KP9,            GUI_KEY_9,               /* 9 */
    KEY_COLON,          GUI_KEY_COLON,           /* : */
    KEY_SEMICOLON,      GUI_KEY_SEMICOLON,       /* ; */
    KEY_LESS,           GUI_KEY_LESS,            /* < */
    KEY_EQUALS,         GUI_KEY_EQUAL,           /* = */
    KEY_GREATER,        GUI_KEY_GREATER,         /* > */
    KEY_QUESTION,       GUI_KEY_QUESTION,        /* ? */
    KEY_AT,             GUI_KEY_AT,              /* @ */
    KEY_A,              GUI_KEY_A,               /* A */
    KEY_B,              GUI_KEY_B,               /* B */
    KEY_C,              GUI_KEY_C,               /* C */
    KEY_D,              GUI_KEY_D,               /* D */
    KEY_E,              GUI_KEY_E,               /* E */
    KEY_F,              GUI_KEY_F,               /* F */
    KEY_G,              GUI_KEY_G,               /* G */
    KEY_H,              GUI_KEY_H,               /* H */
    KEY_I,              GUI_KEY_I,               /* I */
    KEY_J,              GUI_KEY_J,               /* J */
    KEY_K,              GUI_KEY_K,               /* K */
    KEY_L,              GUI_KEY_L,               /* L */
    KEY_M,              GUI_KEY_M,               /* M */
    KEY_N,              GUI_KEY_N,               /* N */
    KEY_O,              GUI_KEY_O,               /* O */
    KEY_P,              GUI_KEY_P,               /* P */
    KEY_Q,              GUI_KEY_Q,               /* Q */
    KEY_R,              GUI_KEY_R,               /* R */
    KEY_S,              GUI_KEY_S,               /* S */
    KEY_T,              GUI_KEY_T,               /* T */
    KEY_U,              GUI_KEY_U,               /* U */
    KEY_V,              GUI_KEY_V,               /* V */
    KEY_W,              GUI_KEY_W,               /* W */
    KEY_X,              GUI_KEY_X,               /* X */
    KEY_Y,              GUI_KEY_Y,               /* Y */
    KEY_Z,              GUI_KEY_Z,               /* Z */
    KEY_a,              GUI_KEY_a,               /* A */
    KEY_b,              GUI_KEY_b,               /* B */
    KEY_c,              GUI_KEY_c,               /* C */
    KEY_d,              GUI_KEY_d,               /* D */
    KEY_e,              GUI_KEY_e,               /* E */
    KEY_f,              GUI_KEY_f,               /* F */
    KEY_g,              GUI_KEY_g,               /* G */
    KEY_h,              GUI_KEY_h,               /* H */
    KEY_i,              GUI_KEY_i,               /* I */
    KEY_j,              GUI_KEY_j,               /* J */
    KEY_k,              GUI_KEY_k,               /* K */
    KEY_l,              GUI_KEY_l,               /* L */
    KEY_m,              GUI_KEY_m,               /* M */
    KEY_n,              GUI_KEY_n,               /* N */
    KEY_o,              GUI_KEY_o,               /* O */
    KEY_p,              GUI_KEY_p,               /* P */
    KEY_q,              GUI_KEY_q,               /* Q */
    KEY_r,              GUI_KEY_r,               /* R */
    KEY_s,              GUI_KEY_s,               /* S */
    KEY_t,              GUI_KEY_t,               /* T */
    KEY_u,              GUI_KEY_u,               /* U */
    KEY_v,              GUI_KEY_v,               /* V */
    KEY_w,              GUI_KEY_w,               /* W */
    KEY_x,              GUI_KEY_x,               /* X */
    KEY_y,              GUI_KEY_y,               /* Y */
    KEY_z,              GUI_KEY_z,               /* Z */
    KEY_LEFTSQUAREBRACKET,  GUI_KEY_BRACKET_LEFT,    /* [ */
    KEY_BACKSLASH,      GUI_KEY_BACK_SLASH,      /* \ */
    KEY_RIGHTSQUAREBRACKET, GUI_KEY_BRACKET_RIGHT,   /* ] */
    KEY_CARET,          GUI_KEY_ASCII_CIRCUM,    /* ^ */
    KEY_UNDERSCRE,      GUI_KEY_UNDERSCORE,      /* _ */
    KEY_BACKQUOTE,      GUI_KEY_GRAVE,           /* ` */

    KEY_LEFTBRACKET,    GUI_KEY_BRACE_LEFT,      /* { */
    KEY_VERTICAL,       GUI_KEY_BAR,             /* | */
    KEY_RIGHTBRACKET,   GUI_KEY_BRACE_RIGHT,     /* } */
    KEY_TILDE,          GUI_KEY_ASCII_TILDE,     /* ~ */
    KEY_QUOTE,          GUI_KEY_APOSTROPHE,      /* ' */

    KEY_F1,             GUI_KEY_F1,
    KEY_F2,             GUI_KEY_F2,
    KEY_F3,             GUI_KEY_F3,
    KEY_F4,             GUI_KEY_F4,
    KEY_F5,             GUI_KEY_F5,
    KEY_F6,             GUI_KEY_F6,
    KEY_F7,             GUI_KEY_F7,
    KEY_F8,             GUI_KEY_F8,
    KEY_F9,             GUI_KEY_F9,
    KEY_F10,            GUI_KEY_F10,
    KEY_F11,            GUI_KEY_F11,
    KEY_F12,            GUI_KEY_F12,

    KEY_LSHIFT,         GUI_KEY_SHIFT,
    KEY_RSHIFT,         GUI_KEY_SHIFT,

    KEY_LCTRL,          GUI_KEY_CONTROL,
    KEY_RCTRL,          GUI_KEY_CONTROL,

    KEY_LALT,           GUI_KEY_ALT,
    KEY_RALT,           GUI_KEY_ALT,

    KEY_LMETA,          GUI_KEY_META,
    KEY_RMETA,          GUI_KEY_META,

    KEY_CAPSLOCK,       GUI_KEY_CAPS_LOCK,
    KEY_NUMLOCK,        GUI_KEY_NUM_LOCK,
    KEY_SCROLLOCK,      GUI_KEY_SCROLL_LOCK,

    0xFF,               GUI_KEY_UNKNOWN
};


static  int  kbd_res  = 0;
static  unsigned char  caps_lock_value = 0;    
static  unsigned char  num_lock_value  = 0;


static  int  input_open_key(void)
{
    /* 
     * Your keyboard device name: /dev/input/event4. 
     * Yes or no ? 
     */
    kbd_res = res_open( GUI_KEYBOARD_DEVICE_NAME, RES_DEV, 0 );
    if ( kbd_res < 0 )
        return  -1;

    int ledstate;
    res_ioctl( kbd_res, EVENIO_GETLED,(unsigned long) &ledstate);

    if ( ledstate&0x01 )
        num_lock_value = 1;
    else
        num_lock_value = 0;


    if ( ledstate&0x02 )
        caps_lock_value = 1;
    else
        caps_lock_value = 0;


    return  1;
}

static  int  input_close_key(void)
{
    return  res_close(kbd_res);
}

static  unsigned char  scan_code_to_gui_key_value(unsigned char code)
{
    unsigned char  key_value = 0xFF;
    unsigned int   i         = 0;


    for ( i = 0;  i < sizeof(map_table);  i += 2 )
    {
        if ( map_table[i] == code )
        {
            key_value = map_table[i+1];
            break;
        }
    }

    return    key_value;
}

static  unsigned char  decode_scan_code(struct input_event  *e)
{
    static  unsigned char  shift_value = 0;
            unsigned char  key_value = 0;


    if ( e == NULL )
        return  key_value;
    /*
    if ( (e->code) == KEY_DOLLAR )            
        e->code = KEY_CHDOLLAR;
    */
    if ( (e->value) > 0 )
    {
        if ( e->code == KEY_CAPSLOCK ) 
        {
            if ( caps_lock_value == 0)
                caps_lock_value = 1;
            else
                caps_lock_value = 0;
        }

        if ( e->code == KEY_NUMLOCK ) 
        {
            if ( num_lock_value == 0)
                num_lock_value = 1;
            else
                num_lock_value = 0;
        }
    }

    /* NumLock */
    if ( num_lock_value == 1 )
    {
        if (  e->code == KEY_KP0 )
            return  GUI_KEY_0;
        else if (  e->code == KEY_KP1 )
            return  GUI_KEY_1;
        else if (  e->code == KEY_KP2 )
            return  GUI_KEY_2;
        else if (  e->code == KEY_KP3 )
            return  GUI_KEY_3;
        else if (  e->code == KEY_KP4 )
            return  GUI_KEY_4;
        else if (  e->code == KEY_KP5 )
            return  GUI_KEY_5;
        else if (  e->code == KEY_KP6 )
            return  GUI_KEY_6;
        else if (  e->code == KEY_KP7 )
            return  GUI_KEY_7;
        else if (  e->code == KEY_KP8 )
            return  GUI_KEY_8;
        else if (  e->code == KEY_KP9 )
            return  GUI_KEY_9;
        else if (  e->code == KEY_KP_PERIOD )
            return  GUI_KEY_DOT;
    }

    key_value = scan_code_to_gui_key_value(e->code);
#if 0
    /* CapsLock */
    if ( (key_value >= GUI_KEY_A)&&(key_value <= GUI_KEY_Z) )
    {
        if ( caps_lock_value == 0 )
            key_value += 32;
        return  key_value;
    }

    /* Shift */        
    if ( key_value == GUI_KEY_SHIFT )
    {
        if ( (e->value) > 0 )
            shift_value = 1;
        else
            shift_value = 0;
    }
    if ( shift_value > 0 )
    {
        if ( key_value == GUI_KEY_GRAVE )
            key_value =  GUI_KEY_ASCII_TILDE;
        else if ( key_value == GUI_KEY_1 )
            key_value =  GUI_KEY_EXCLAM;
        else if ( key_value == GUI_KEY_2 )
            key_value =  GUI_KEY_AT;
        else if ( key_value == GUI_KEY_3 )
            key_value =  GUI_KEY_NUMBER_SIGN;
        else if ( key_value == GUI_KEY_4 )
            key_value =  GUI_KEY_DOLLAR;
        else if ( key_value == GUI_KEY_5 )
            key_value =  GUI_KEY_PERCENT;
        else if ( key_value == GUI_KEY_6 )
            key_value =  GUI_KEY_ASCII_CIRCUM;
        else if ( key_value == GUI_KEY_7 )
            key_value =  GUI_KEY_AMPERSAND;
        else if ( key_value == GUI_KEY_8 )
            key_value =  GUI_KEY_ASTERISK;
        else if ( key_value == GUI_KEY_9 )
            key_value =  GUI_KEY_PAREN_LEFT;
        else if ( key_value == GUI_KEY_0 )
            key_value =  GUI_KEY_PAREN_RIGHT;
        else if ( key_value == GUI_KEY_MINUS )
            key_value =  GUI_KEY_UNDERSCORE;
        else if ( key_value == GUI_KEY_EQUAL )
            key_value =  GUI_KEY_PLUS;
        else if ( key_value == GUI_KEY_COMMA )
            key_value =  GUI_KEY_LESS;
        else if ( key_value == GUI_KEY_DOT )
            key_value =  GUI_KEY_GREATER;
        else if ( key_value == GUI_KEY_SLASH )
            key_value =  GUI_KEY_QUESTION; 
        else if ( key_value == GUI_KEY_SEMICOLON )
            key_value =  GUI_KEY_COLON;
        else if ( key_value == GUI_KEY_APOSTROPHE )
            key_value =  GUI_KEY_QUOTE_DBL;
        else if ( key_value == GUI_KEY_BRACKET_LEFT )
            key_value =  GUI_KEY_BRACE_LEFT;
        else if ( key_value == GUI_KEY_BRACKET_RIGHT )
            key_value =  GUI_KEY_BRACE_RIGHT;
        else if ( key_value == GUI_KEY_BACK_SLASH )
            key_value =  GUI_KEY_BAR; 
    }
#endif
    return  key_value;
}

static  int  input_read_key(void *msg)
{
    GUI_MESSAGE  *p = (GUI_MESSAGE *)msg;
    struct  input_event  event;
    int     ret       = 0;


    if ( p == NULL )
        return  -1;

    memset(&event, 0, sizeof(event));
    ret = res_read(kbd_res, 0, &event, sizeof(event));
    if ( ret < 1 )
        return  0;

    switch (event.type)
    {                
        case EV_KEY:         
            if ( (event.value) > 0 )
                p->id = MSG_KEY_DOWN; 
            else
                p->id =  MSG_KEY_UP;  

            p->data0.value = decode_scan_code(&event);
            return  1;

        default:
            break;
    }

    return  0;
}

static  int  input_write_key(void *buffer, unsigned int len)
{
    return  0;
}

static  int  input_control_key(void *p1, void *p2)
{
    return  1;
}

static  int  input_reinit_key(void)
{
    return  1;
}


int  register_keyboard(void)
{
    GUI_KEYBOARD   keyboard;


    memset(&keyboard, 0, sizeof(keyboard));

    keyboard.open        = input_open_key;
    keyboard.close       = input_close_key;

    keyboard.read        = input_read_key;
    keyboard.write       = input_write_key;

    keyboard.control     = input_control_key;
    keyboard.reinit      = input_reinit_key;

    in_driver_register(DRIVER_KEYBOARD, &keyboard);

    return  1;
}

#endif  /*  _LG_KEYBOARD_ */
