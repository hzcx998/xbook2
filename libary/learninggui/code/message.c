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

#include  <string.h>

#include  <lock.h>
#include  <cursor.h>

#include  <message.h>
#include  <keyboard.h>
#include  <mtjt.h>
#include  <counter.h>
#include  <timer.h>

#include  <screen.h>
#include  <snapshot.h>

#ifdef  _LG_WINDOW_
#include  <win_caret.h>
#include  <win_invalidate.h>
#include  <win_callback.h>

#include  "win_arithmetic_in.h"
#endif


#define   NONE_LOOP_ID                 0x00
#define   TIMER_LOOP_ID                0x01
#define   KEYBOARD_LOOP_ID             0x02
#define   MTJT_LOOP_ID                 0x03
#define   COUNTER_LOOP_ID              0x04


#ifdef  _LG_MESSAGE_
/* Application  message current routine */
static  volatile  GUI_CALLBACK    lmsgro = NULL;

static  volatile  unsigned int    lmhead = 0;
static  volatile  unsigned int    lmtail = 0;
static  volatile  IN_GUI_MESSAGE  lmqueu[MESSAGE_QUEUE_LEN] = { {0} };

static  volatile  unsigned int    llooid = 0;

#ifdef  _LG_WINDOW_
extern  volatile  HWND  lhfocu;    
#endif



/* Default message routine */ 
static  int  in_message_default_routine(void *msg)
{
    #ifdef  _LG_SCREEN_
    #ifdef  _LG_SNAPSHOT_
    #ifdef  _LG_FILE_SYSTEM_
    int  key_value = 0;
    #endif
    #endif
    #endif


    if ( msg == NULL )
        return  -1;

    switch( MESSAGE_GET_ID(msg))
    {
        #ifdef   _LG_MTJT_
        #ifdef  _LG_CURSOR_
        case  MSG_MTJT_MOVE:
            in_cursor_set_position(MESSAGE_GET_MTJT_X(msg), MESSAGE_GET_MTJT_Y(msg));
            break;
        #endif
        #endif

        #ifdef  _LG_SCREEN_
        #ifdef  _LG_SNAPSHOT_
        #ifdef  _LG_FILE_SYSTEM_
        case  MSG_KEY_UP:
           key_value = MESSAGE_GET_KEY_VALUE(msg);
           if ( (key_value == GUI_KEY_PRINT) || (key_value == GUI_KEY_SYS_REQ))
               in_snapshot_get( );
           break;
        #endif
        #endif
        #endif

        default:
            break;
    }

    return  1;
}

int  in_message_init(void)
{
    lmhead = 0;
    lmtail = 0;
    memset((void *)lmqueu, 0, sizeof(lmqueu));

    lmsgro = NULL;

    return  1;
}

/* Set message routine */    
int  in_message_set_routine(void *routine)
{
    if (routine == NULL)
        return  -1;

    lmsgro = ( GUI_CALLBACK )routine;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  message_set_routine(void *routine)
{
    int  ret = 0;

    callback_lock( );
    ret = in_message_set_routine(routine);
    callback_unlock( );

    return  ret;
}
#endif


/* Post message */  
int  in_message_post(void *msg)
{
    if (msg == NULL)
        return  -1;

    lmqueu[lmtail].user_msg = *((GUI_MESSAGE *)msg);
    lmqueu[lmtail].flag     = 0;
    lmtail++;
   
    if ( lmtail >= MESSAGE_QUEUE_LEN )
        lmtail  = 0;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  message_post(void *msg)
{
    int  ret = 0;

    gui_lock( );
    ret = in_message_post(msg);
    gui_unlock( );

    return  ret;
}
#endif

/* Post quit message */  
int  in_message_post_quit(void)
{
    GUI_MESSAGE   msg;
    int           ret = 0;


    memset(&msg, 0, sizeof(GUI_MESSAGE));
    msg.id = MSG_QUIT;
    ret = in_message_post(&msg);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  message_post_quit(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_message_post_quit( );
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


/* Get message */  
int  in_message_get(void *msg)
{
    GUI_MESSAGE  *pmsg = (GUI_MESSAGE *)msg;
    #ifdef  _LG_TIMER_
    IN_GUI_TIMER     timer = { 0 };
    #endif
    #ifdef  _LG_COUNTER_
    IN_GUI_COUNTER   counter = { 0 };
    #endif
    int  ret      = 0;


    if ( pmsg == NULL )
        return  -1;

    memset(pmsg, 0, sizeof(GUI_MESSAGE));

    if ( lmhead == lmtail )
        goto  GET_INPUT_MESSAGE;

    *pmsg = lmqueu[lmhead].user_msg;
    lmqueu[lmhead].flag = 0;
    lmhead++;
   
    if ( lmhead >= MESSAGE_QUEUE_LEN )
        lmhead  = 0;

    if ( lmhead == lmtail )
    {
        lmhead = 0;
        lmtail = 0;
    }
    return  1;


GET_INPUT_MESSAGE:
    /* timer */
    #ifdef  _LG_TIMER_
    if ( llooid == TIMER_LOOP_ID )
        goto  KEYBOARD_LOOP;

    memset(&timer, 0, sizeof(timer));
    ret = in_gui_timer_get_expired(&timer);
    if ( ret > 0 )
    {
        pmsg->id            = MSG_TIMER;
        pmsg->data0.value   = timer.id;
        #ifdef  _LG_WINDOW_
        pmsg->to_hwnd       = timer.para;
        pmsg->callback_flag = HWND_APP_CALLBACK;
        #endif

        llooid     = TIMER_LOOP_ID;
        return  1;
    }

KEYBOARD_LOOP:
    #endif  /* _LG_TIMER_ */

    /* key message */
    #ifdef  _LG_KEYBOARD_
    if ( llooid == KEYBOARD_LOOP_ID )
        goto  MTJT_LOOP;

    ret = 0;
    if ( (lkbd.read) != 0 )
        ret = (lkbd.read)(pmsg);

    if ( ret > 0 )
    {
        llooid = KEYBOARD_LOOP_ID;
        return  1;
    }

MTJT_LOOP:
    #endif  /* _LG_KEYBOARD_ */


    /* mtjt message */
    #ifdef  _LG_MTJT_
    if ( llooid == MTJT_LOOP_ID )
        goto  COUNTER_LOOP;

    ret = 0;
    if ( (lmtjt.read) != 0 )
        ret = (lmtjt.read)(pmsg);
    if ( ret > 0 )
    {
        llooid = MTJT_LOOP_ID;
        return  1;
    }

COUNTER_LOOP:
    #endif  /* _LG_MTJT_ */

    /* counter message */
    #ifdef  _LG_COUNTER_
    memset( &counter, 0, sizeof(counter));
    ret = in_gui_counter_get_expired(&counter);
    if ( ret > 0 )
    {
        pmsg->id             = MSG_COUNTER;
        pmsg->data0.value    = counter.id;
        #ifdef  _LG_WINDOW_
        pmsg->to_hwnd        = counter.para;
        pmsg->callback_flag  = HWND_IN_CALLBACK | HWND_APP_CALLBACK;
        #endif
        llooid    = COUNTER_LOOP_ID;
        return  1;
    }
    #endif  /* _LG_COUNTER_ */


    /* Window invalidate area repaint */
    #ifdef  _LG_WINDOW_
    if ( linvan > 0 )
        in_win_paint_invalidate_recursion(lhlist);

    #ifdef  _LG_CARET_
    in_caret_deal();
    #endif
    #endif


    /* Refresh screen */
    if ( (lscrn.refresh) != NULL )
        (lscrn.refresh)();


    
    llooid = NONE_LOOP_ID;

    return  0;
}

#ifndef  _LG_ALONE_VERSION_
int  message_get(void *msg)
{
    int  ret = 0;

    gui_lock( );
    ret = in_message_get(msg);
    gui_unlock( );

    return  ret;
}
#endif

 
/* Dispatch message */
int  in_message_dispatch(void *msg)
{
    GUI_MESSAGE  *pmsg   = (GUI_MESSAGE *)msg;
    int              ret = 1;


    if ( pmsg == NULL )
        return  -1;

    #ifdef  _LG_WINDOW_
    if (pmsg->to_hwnd == NULL)
    {
        pmsg->to_hwnd       = lhfocu;
        pmsg->callback_flag = HWND_IN_CALLBACK | HWND_APP_CALLBACK;
    }
    #endif


    /* Keyboard ??  */


    /* ?? optimize */
    /* Repeation ?? */
    ret = 1;
    if ( lmsgro != NULL )
    {
        gui_unlock();
        ret = (*lmsgro)(pmsg);
        gui_lock();
    }
    if ( ret < 1 )
        return  1;

    #ifdef  _LG_WINDOW_
    ret = in_message_window_routine(pmsg);
    if ( ret < 1 )
        return  1;
    #endif

    ret = in_message_default_routine(pmsg);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  message_dispatch(void *msg)
{
    int ret = 0;

    callback_lock();
    ret = in_message_dispatch(msg);
    callback_unlock();

    return  ret;
}
#endif


/* Dispatch all message(s) */
int  in_message_dispatch_all(void)
{
    GUI_MESSAGE  msg = { 0 };


    while ( 1 )
    {
        if ( lmhead == lmtail )
            break;   /* Avoid compiliing warning */

        msg = lmqueu[lmhead].user_msg;
        lmqueu[lmhead].flag = 0;
        lmhead++;
   
        if ( lmhead >= MESSAGE_QUEUE_LEN )
            lmhead  = 0;

        if ( lmhead == lmtail )
        {
            lmhead = 0;
            lmtail = 0;
        }

        #ifdef  _LG_WINDOW_
        if (msg.to_hwnd == NULL)
        {
            msg.to_hwnd       = lhfocu;
            msg.callback_flag = HWND_APP_CALLBACK | HWND_APP_CALLBACK;
        }
        #endif  /* _LG_WINDOW_ */

        in_message_dispatch(&msg);
    }

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  message_dispatch_all(void)
{
    int ret = 0;

    callback_lock();
    ret = in_message_dispatch_all();
    callback_unlock();

    return  ret;
}
#endif

/* Clear message queue */  
int  in_message_clear_queue(void)
{
    lmhead = 0;
    lmtail = 0;
    memset((void *)lmqueu, 0, sizeof(lmqueu));

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  message_clear_queue(void)
{
    int  ret = 0;

    gui_lock( );
    ret = in_message_clear_queue( );
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_MESSAGE_ */
