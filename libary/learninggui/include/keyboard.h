/*   
 *  Copyright (C) 2011- 2018 Rao Youkun(960747373@qq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef  __LGUI_KEYBOARD_HEADER__
#define  __LGUI_KEYBOARD_HEADER__

#include  <type_color.h>

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

#include  <type_gui.h>



enum  GUI_KEY  {
    GUI_KEY_ESCAPE        = 0x01,

    GUI_KEY_BACKSPACE     = 0x02,
    GUI_KEY_TAB           = 0x03,
    GUI_KEY_BACK_TAB      = 0x04,

    GUI_KEY_PAUSE         = 0x05,
    GUI_KEY_PRINT         = 0x06,
    GUI_KEY_SYS_REQ       = 0x07,
    GUI_KEY_CLEAR         = 0x08,

    GUI_KEY_INSERT        = 0x09,
    GUI_KEY_ENTER         = 0x0A,
    GUI_KEY_DELETE        = 0x0B,

    GUI_KEY_LEFT          = 0x11,
    #define GUI_KEY_FIRST_MOVE    GUI_KEY_LEFT
    GUI_KEY_RIGHT         = 0x12,
    GUI_KEY_UP            = 0x13,
    GUI_KEY_DOWN          = 0x14,
    GUI_KEY_HOME          = 0x15,
    GUI_KEY_END           = 0x16,
    GUI_KEY_PAGE_UP       = 0x17,
    GUI_KEY_PAGE_DOWN     = 0x18,
    #define GUI_KEY_END_MOVE    GUI_KEY_PAGE_DOWN

    GUI_KEY_SPACE         = 0x20, 
    #define GUI_KEY_FIRST_CHAR    GUI_KEY_SPACE
    GUI_KEY_EXCLAM        = 0x21,
    GUI_KEY_QUOTE_DBL     = 0x22,
    GUI_KEY_NUMBER_SIGN   = 0x23,
    GUI_KEY_DOLLAR        = 0x24,
    GUI_KEY_PERCENT       = 0x25,
    GUI_KEY_AMPERSAND     = 0x26,
    GUI_KEY_APOSTROPHE    = 0x27,
    GUI_KEY_PAREN_LEFT    = 0x28,
    GUI_KEY_PAREN_RIGHT   = 0x29,
    GUI_KEY_ASTERISK      = 0x2A,
    GUI_KEY_PLUS          = 0x2B,
    GUI_KEY_COMMA         = 0x2C,
    GUI_KEY_MINUS         = 0x2D,
    GUI_KEY_DOT           = 0x2E,
    GUI_KEY_SLASH         = 0x2F,

    GUI_KEY_0             = 0x30,
    GUI_KEY_1             = 0x31,
    GUI_KEY_2             = 0x32,
    GUI_KEY_3             = 0x33,
    GUI_KEY_4             = 0x34,
    GUI_KEY_5             = 0x35,
    GUI_KEY_6             = 0x36,
    GUI_KEY_7             = 0x37,
    GUI_KEY_8             = 0x38,
    GUI_KEY_9             = 0x39,
    GUI_KEY_COLON         = 0x3A,
    GUI_KEY_SEMICOLON     = 0x3B,
    GUI_KEY_LESS          = 0x3C,
    GUI_KEY_EQUAL         = 0x3D,
    GUI_KEY_GREATER       = 0x3E,
    GUI_KEY_QUESTION      = 0x3F,
    GUI_KEY_AT            = 0x40,
    GUI_KEY_A             = 0x41,
    GUI_KEY_B             = 0x42,
    GUI_KEY_C             = 0x43,
    GUI_KEY_D             = 0x44,
    GUI_KEY_E             = 0x45,
    GUI_KEY_F             = 0x46,
    GUI_KEY_G             = 0x47,
    GUI_KEY_H             = 0x48,
    GUI_KEY_I             = 0x49,
    GUI_KEY_J             = 0x4A,
    GUI_KEY_K             = 0x4B,
    GUI_KEY_L             = 0x4C,
    GUI_KEY_M             = 0x4D,
    GUI_KEY_N             = 0x4E,
    GUI_KEY_O             = 0x4F,
    GUI_KEY_P             = 0x50,
    GUI_KEY_Q             = 0x51,
    GUI_KEY_R             = 0x52,
    GUI_KEY_S             = 0x53,
    GUI_KEY_T             = 0x54,
    GUI_KEY_U             = 0x55,
    GUI_KEY_V             = 0x56,
    GUI_KEY_W             = 0x57,
    GUI_KEY_X             = 0x58,
    GUI_KEY_Y             = 0x59,
    GUI_KEY_Z             = 0x5A,
    GUI_KEY_BRACKET_LEFT  = 0x5B,
    GUI_KEY_BACK_SLASH    = 0x5C,
    GUI_KEY_BRACKET_RIGHT = 0x5D,
    GUI_KEY_ASCII_CIRCUM  = 0x5E,
    GUI_KEY_UNDERSCORE    = 0x5F,
    GUI_KEY_GRAVE         = 0x60,
    GUI_KEY_a             = 0x61,
    GUI_KEY_b             = 0x62,
    GUI_KEY_c             = 0x63,
    GUI_KEY_d             = 0x64,
    GUI_KEY_e             = 0x65,
    GUI_KEY_f             = 0x66,
    GUI_KEY_g             = 0x67,
    GUI_KEY_h             = 0x68,
    GUI_KEY_i             = 0x69,
    GUI_KEY_j             = 0x6A,
    GUI_KEY_k             = 0x6B,
    GUI_KEY_l             = 0x6C,
    GUI_KEY_m             = 0x6D,
    GUI_KEY_n             = 0x6E,
    GUI_KEY_o             = 0x6F,
    GUI_KEY_p             = 0x70,
    GUI_KEY_q             = 0x71,
    GUI_KEY_r             = 0x72,
    GUI_KEY_s             = 0x73,
    GUI_KEY_t             = 0x74,
    GUI_KEY_u             = 0x75,
    GUI_KEY_v             = 0x76,
    GUI_KEY_w             = 0x77,
    GUI_KEY_x             = 0x78,
    GUI_KEY_y             = 0x79,
    GUI_KEY_z             = 0x7A, 
    GUI_KEY_BRACE_LEFT    = 0x7B,
    GUI_KEY_BAR           = 0x7C,
    GUI_KEY_BRACE_RIGHT   = 0x7D,
    GUI_KEY_ASCII_TILDE   = 0x7E,
    #define GUI_KEY_END_CHAR      GUI_KEY_ASCII_TILDE

    GUI_KEY_F1            = 0x81,
    GUI_KEY_F2            = 0x82,
    GUI_KEY_F3            = 0x83,
    GUI_KEY_F4            = 0x84,
    GUI_KEY_F5            = 0x85,
    GUI_KEY_F6            = 0x86,
    GUI_KEY_F7            = 0x87,
    GUI_KEY_F8            = 0x88,
    GUI_KEY_F9            = 0x89,
    GUI_KEY_F10           = 0x8A,
    GUI_KEY_F11           = 0x8B,
    GUI_KEY_F12           = 0x8C,

    GUI_KEY_SHIFT         = 0xA1,
    GUI_KEY_CONTROL       = 0xA2,
    GUI_KEY_ALT           = 0xA3,
    GUI_KEY_META          = 0xA4,

    GUI_KEY_CAPS_LOCK     = 0xAA,
    GUI_KEY_NUM_LOCK      = 0xAB,
    GUI_KEY_SCROLL_LOCK   = 0xAC,

    GUI_KEY_UNKNOWN       = 0xFF
};

struct  _GUI_KEYBOARD
{
    int  (*open)(void);
    int  (*close)(void);

    int  (*read)(void *msg);
    int  (*write)(void *buffer, unsigned int len);

    int  (*control)(void *p1, void *p2);
    int	 (*reinit)(void);
};
typedef	struct	_GUI_KEYBOARD  GUI_KEYBOARD;

   
#ifdef  _LG_KEYBOARD_


#ifdef  __cplusplus
extern  "C"
{
#endif

    extern  volatile  GUI_KEYBOARD  lkbd;

    /* Internal Function */
    int  in_keyboard_open(void);
    int  in_keyboard_close(void);
    /* Internal Function ok */


    int  in_keyboard_control(void *p1, void *p2);
    int  in_keyboard_reinit(void);

    #ifndef  _LG_ALONE_VERSION_
    int  keyboard_control(void *p1, void *p2);
    int  keyboard_reinit(void);
    #else
    #define  keyboard_control(p1, p2)                  in_keyboard_control(p1, p2)
    #define  keyboard_reinit()                         in_keyboard_reinit()
    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_KEYBOARD_ */

#endif  /* __LGUI_KEYBOARD_HEADER__ */
