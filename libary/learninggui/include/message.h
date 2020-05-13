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

#ifndef  __LGUI_MESSAGE_HEADER__
#define  __LGUI_MESSAGE_HEADER__

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

#ifdef  _LG_WINDOW_
#include  <win_type_widget.h>
#endif



/* 
 * LearningGUI messages ID
 */

/* Null message */
#define  MSG_NULL                   0x0000

/* Quit  message */
#define  MSG_QUIT                   0x0001

/* key  message */
#define  MSG_KEY_FIRST              0x000A
#define  MSG_KEY_TYPE               0x001B
#define  MSG_KEY_DOWN               0x000C
#define  MSG_KEY_UP                 0x000D
#define  MSG_KEY_CHAR               0x000E
#define  MSG_KEY_LAST               0x000F

/* MTJT message */
#define  MSG_MTJT_FIRST             0x0030
#define  MSG_MTJT_TYPE              0x0031
#define  MSG_MTJT_MOVE              0x0032
#define  MSG_MTJT_LBUTTON_DOWN      0x0033
#define  MSG_MTJT_LBUTTON_UP        0x0034
#define  MSG_MTJT_LBUTTON_DBLCLK    0x0035
#define  MSG_MTJT_RBUTTON_DOWN      0x0036
#define  MSG_MTJT_RBUTTON_UP        0x0037
#define  MSG_MTJT_RBUTTON_DBLCLK    0x0038
#define  MSG_MTJT_MBUTTON_DOWN      0x0039
#define  MSG_MTJT_MBUTTON_UP        0x003A
#define  MSG_MTJT_MBUTTON_DBLCLK    0x003B
#define  MSG_MTJT_PRESS             0x003C
#define  MSG_MTJT_WHEEL             0x003D
#define  MSG_MTJT_LAST              0x003E

/* Timer message */
#define  MSG_TIMER_FIRST            0x0050
#define  MSG_TIMER_CREATE           0x0050
#define  MSG_TIMER                  0x0051
#define  MSG_TIMER_DELETE           0x0052
#define  MSG_TIMER_OVERFLOW         0x0053
#define  MSG_TIMER_LAST             0x0053

/* Counter message */
#define  MSG_COUNTER_FIRST          0x0058
#define  MSG_COUNTER                0x0058
#define  MSG_COUNTER_LAST           0x0058

/* Misc  message */
#define  MSG_MSC_FIRST              0x0070
#define  MSG_MSC_TYPE               0x0071
#define  MSG_MSC_LAST               0x0072

/* Window message */
#define  MSG_WINDOW_FIRST           0x0080
#define  MSG_CREATE                 0x0080
#define  MSG_CREATE_NEXT            0x0081
#define  MSG_CLOSE                  0x0082
#define  MSG_CLOSE_NEXT             0x0083
#define  MSG_PAINT                  0x0084
#define  MSG_PAINT_NEXT             0x0085
#define  MSG_HIDE                   0x0086
#define  MSG_MAXIZE                 0x0087
#define  MSG_NORMAL                 0x0088
#define  MSG_MOVE                   0x0089
#define  MSG_RESIZE                 0x008A
#define  MSG_GET_FOCUS              0x008B
#define  MSG_LOST_FOCUS             0x008C
#define  MSG_CARET                  0x008D

/* Widget message */
#define  MSG_NOTIFY_SEL_CHANGED     0x00A0
#define  MSG_NOTIFY_VALUE_CHANGED   0x00A1

#define  MSG_WINDOW_LAST            0x00A1


/* User message */
#define  MSG_USER                   0x00E0



/* Window message callback flag */
#define  HWND_APP_CALLBACK          (1<<0)
#define  HWND_IN_CALLBACK           (1<<1)

/* Window message callback flag mask */
#define  HWND_CALLBACK_MASK         (HWND_APP_CALLBACK | HWND_IN_CALLBACK)


/* Callback function typedef */
typedef  int  (*GUI_CALLBACK)(void *msg);


/* Message struction definition */
struct  _STRUCT_UINT16
{
    UINT16    high;
    UINT16    low;
};
typedef  struct  _STRUCT_UINT16    STRUCT_UINT16;

struct  _STRUCT_UINT8
{
    UINT8     a;
    UINT8     b;
    UINT8     c;
    UINT8     d;
};
typedef  struct  _STRUCT_UINT8    STRUCT_UINT8;

union  _GUI_MESSAGE_UNION
{
    int            value;
    STRUCT_UINT16  struct_u16;
    STRUCT_UINT8   struct_u8;
    int            *pint;
    INT16          *pint16;
    char           *pchar;
};
typedef  union  _GUI_MESSAGE_UNION    GUI_MESSAGE_UNION;

struct  _GUI_MESSAGE
{
    UINT               id;
    #ifdef  _LG_WINDOW_
    void              *to_hwnd;
    int                callback_flag;
    #endif
    GUI_MESSAGE_UNION  data0;
    GUI_MESSAGE_UNION  data1;
    GUI_MESSAGE_UNION  data2;
    #ifdef  _LG_LONG_MESSAGE_
    GUI_MESSAGE_UNION  data3;
    GUI_MESSAGE_UNION  data4;
    #endif
};
typedef  struct  _GUI_MESSAGE    GUI_MESSAGE;


struct  _IN_GUI_MESSAGE
{
    BUINT         flag;
    GUI_MESSAGE   user_msg;
};
typedef	struct	_IN_GUI_MESSAGE  IN_GUI_MESSAGE;



/* Message macros */
#define  MESSAGE_GET_ID(msg)                    (((GUI_MESSAGE *)(msg))->id)
#define  MESSAGE_SET_ID(msg,id)                 (((GUI_MESSAGE *)(msg))->id = id)

#define  MESSAGE_GET_KEY_VALUE(msg)             (((GUI_MESSAGE *)(msg))->data0.value)
#define  MESSAGE_SET_KEY_VALUE(msg,value)       (((GUI_MESSAGE *)(msg))->data0.value = value)

#define  MESSAGE_GET_DATA0_VALUE(msg)           (((GUI_MESSAGE *)(msg))->data0.value)
#define  MESSAGE_SET_DATA0_VALUE(msg,value)     (((GUI_MESSAGE *)(msg))->data0.value = value)

#define  MESSAGE_GET_DATA1_VALUE(msg)           (((GUI_MESSAGE *)(msg))->data1.value)
#define  MESSAGE_SET_DATA1_VALUE(msg,value)     (((GUI_MESSAGE *)(msg))->data1.value = value)

#define  MESSAGE_GET_DATA2_VALUE(msg)           (((GUI_MESSAGE *)(msg))->data2.value)
#define  MESSAGE_SET_DATA2_VALUE(msg,value)     (((GUI_MESSAGE *)(msg))->data2.value = value)
    
#ifdef  _LG_LONG_MESSAGE_
#define  MESSAGE_GET_DATA3_VALUE(msg)           (((GUI_MESSAGE *)(msg))->data3.value)
#define  MESSAGE_SET_DATA3_VALUE(msg,value)     (((GUI_MESSAGE *)(msg))->data3.value = value)

#define  MESSAGE_GET_DATA4_VALUE(msg)           (((GUI_MESSAGE *)(msg))->data4.value)
#define  MESSAGE_SET_DATA4_VALUE(msg,value)     (((GUI_MESSAGE *)(msg))->data4.value = value)
#endif


#define  MESSAGE_IS_MTJT(msg)                   (((((GUI_MESSAGE *)msg)->id) >= MSG_MTJT_FIRST) ? (((((GUI_MESSAGE *)msg)->id) <= MSG_MTJT_LAST) ? 1: 0) : 0)

#define  MESSAGE_GET_MTJT_X(msg)                (((GUI_MESSAGE *)(msg))->data0.value)
#define  MESSAGE_GET_MTJT_Y(msg)                (((GUI_MESSAGE *)(msg))->data1.value)
#define  MESSAGE_SET_MTJT_X(msg,x)              (((GUI_MESSAGE *)(msg))->data0.value = x)
#define  MESSAGE_SET_MTJT_Y(msg,y)              (((GUI_MESSAGE *)(msg))->data1.value = y)

#define  MESSAGE_GET_COUNTER_ID(msg)            (((GUI_MESSAGE *)(msg))->data0.value)
#define  MESSAGE_SET_COUNTER_ID(msg,id)         (((GUI_MESSAGE *)(msg))->data0.value = id)

#define  MESSAGE_GET_TIMER_ID(msg)              (((GUI_MESSAGE *)(msg))->data0.value)
#define  MESSAGE_SET_TIMER_ID(msg,id)           (((GUI_MESSAGE *)(msg))->data0.value = id)

#define  MESSAGE_IS_QUIT(msg)                   (((((GUI_MESSAGE *)(msg))->id)==MSG_QUIT) ? 1 : 0)


/* Message function */

#ifdef  _LG_MESSAGE_

#ifdef  __cplusplus
extern  "C"
{
#endif

    /* Internal Function */
    int  in_message_init(void);


    int  in_message_set_routine(void *routine);
    int  in_message_post(void *msg);
    int  in_message_post_quit(void);
    int  in_message_get(void *msg);
    int  in_message_dispatch(void *msg);
    int  in_message_dispatch_all(void);
    int  in_message_clear_queue(void);
    #define  in_message_send(msg)       in_message_dispatch(msg)


    #ifndef  _LG_ALONE_VERSION_

    int  message_set_routine(void *routine);
    int  message_post(void *msg);
    int  message_post_quit(void);
    int  message_get(void *msg);
    int  message_dispatch(void *msg);
    int  message_dispatch_all(void);
    int  message_clear_queue(void);
    #define  message_send(msg)                    message_dispatch(msg)

    #else  /* _LG_ALONE_VERSION_ */

    #define  message_set_routine(routine)         in_message_set_routine(routine)
    #define  message_post(msg)                    in_message_post(msg)
    #define  message_post_quit()                  in_message_post_quit()
    #define  message_get(msg)                     in_message_get(msg)
    #define  message_dispatch(msg)                in_message_dispatch(msg)
    #define  message_dispatch_all()               in_message_dispatch_all()
    #define  message_clear_queue()                in_message_clear_queue()
    #define  message_send(msg)                    in_message_dispatch(msg)

    #endif  /* _LG_ALONE_VERSION_ */

#ifdef  __cplusplus
}
#endif


#endif  /* _LG_MESSAGE_ */

#endif  /* __LGUI_MESSAGE_HEADER__ */
