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

#include  <learninggui.h>


int  gui_open(void)
{
    static  unsigned int  is_gui_init_flag = 0;
                     int  ret              = 1;


    if ( is_gui_init_flag )
        return  1;

    is_gui_init_flag  = 1;


    #ifdef  _LG_SCREEN_
    if ( (lscrn.open) == NULL )
        return  -ESCREEN_OPEN;

    if ( (lscrn.close) == NULL )
        return  -ESCREEN_CLOSE;

    if ( (lscrn.gui_to_screen_color) == NULL )
        return  -ESCREEN_GUI_TO_SCREEN_COLOR;

    if ( (lscrn.screen_to_gui_color) == NULL )
        return  -ESCREEN_SCREEN_TO_GUI_COLOR;

    if ( (lscrn.output_pixel) == NULL )
        return  -ESCREEN_OUTPUT_PIXEL;

    /* ?? */
    if ( (lscrn.input_pixel) == NULL )
        return  -ESCREEN_OUTPUT_PIXEL;
    #endif  /* _LG_SCREEN_ */


    #ifdef  _LG_KEYBOARD_
    if ( (lkbd.open) == NULL )
        return  -EKEYBOARD_OPEN;

    if ( (lkbd.close) == NULL )
        return  -EKEYBOARD_CLOSE;

    if ( (lkbd.read) == NULL )
        return  -EKEYBOARD_READ;

    /* ?? */
    if ( (lkbd.write) == NULL )
        return  -EKEYBOARD_WRITE;
    #endif  /* _LG_KEYBOARD_ */


    #ifdef  _LG_MTJT_
    if ( (lmtjt.open) == NULL )
        return  -EMTJT_OPEN;

    if ( (lmtjt.close) == NULL )
        return  -EMTJT_CLOSE;

    if ( (lmtjt.read) == NULL )
        return  -EMTJT_READ;

    /* ?? */
    if ( (lmtjt.write) == NULL )
        return  -EMTJT_WRITE;
    #endif  /* _LG_MTJT_ */


    #ifdef  _LG_MULTI_THREAD_
    if ( (lglckr.init) == NULL )
        return  -EGUI_LOCKER_INIT;
    if ( (lglckr.destroy) == NULL )
        return  -EGUI_LOCKER_DESTROY;

    if ( (lglckr.lock) == NULL )
        return  -EGUI_LOCKER_LOCK;
    if ( (lglckr.unlock) == NULL )
        return  -EGUI_LOCKER_UNLOCK;


    if ( (lclckr.init) == NULL )
        return  -ECALLBACK_LOCKER_INIT;
    if ( (lclckr.destroy) == NULL )
        return  -ECALLBACK_LOCKER_DESTROY;

    if ( (lclckr.lock) == NULL )
        return  -ECALLBACK_LOCKER_LOCK;
    if ( (lclckr.unlock) == NULL )
        return  -ECALLBACK_LOCKER_UNLOCK;
    #endif  /* _LG_MULTI_THREAD_ */


    #ifdef  _LG_MULTI_THREAD_
    /* Init locker */
    gui_locker_init( );
    callback_locker_init( );
    #endif  /* _LG_MULTI_THREAD_ */

    /* Start to lock */
    gui_lock( );
    callback_lock( );

    #ifdef  _LG_MESSAGE_
    /* Init message */
    in_message_init( );

    #ifdef  _LG_COUNTER_
    /* Init counter */
    ret = in_gui_counter_init();
    if ( ret < 0 )
    {
        callback_unlock( );
        gui_unlock();
        return  -ECOUNTER_INIT;
    }
    #endif  /* _LG_COUNTER_ */

    #ifdef  _LG_TIMER_
    /* Init timer */
    ret = in_gui_timer_init();
    if ( ret < 0 )
    {
        callback_unlock( );
        gui_unlock();
        return  -ETIMER_INIT;
    }
    #endif  /* _LG_TIMER_ */
    #endif /* _LG_MESSAGE_ */


    #ifdef  _LG_CURSOR_
    /* Init cursor */
    ret = in_cursor_init();
    if ( ret < 0 )
    {
        callback_unlock( );
        gui_unlock();
        return  -ECURSOR_INIT;
    }
    #endif  /* _LG_CURSOR_ */


    #ifdef  _LG_SCREEN_ 
    /* Open screen */
    ret = in_screen_open( );
    if ( ret < 0 )
    {
        callback_unlock( );
        gui_unlock();
        return  -ESCREEN_OPEN_RETURN;
    }
    #endif  /* _LG_SCREEN_ */


    #ifdef  _LG_DC_
    /* Init dc */
    /* Should be after in_screen_open */
    ret = in_hdc_basic_init( );
    if ( ret < 0 )
    {
        callback_unlock( );
        gui_unlock();
        return  -EDC_INIT;
    }
    #endif  /* _LG_DC_ */


    #ifdef  _LG_WINDOW_
    /* Init window */
    ret = in_init_window();
    if ( ret < 0 )
    {
        callback_unlock( );
        gui_unlock();
        return  -EWINDOW_INIT;
    }
    #endif  /* _LG_WINDOW_ */


    #ifdef  _LG_KEYBOARD_ 
    /* Open keyboard */
    ret = in_keyboard_open( );
    if ( ret < 0 )
    {
        callback_unlock( );
        gui_unlock();
        return  -EKEYBOARD_OPEN_RETURN;
    }
    #endif  /* _LG_KEYBOARD_ */


    #ifdef  _LG_MTJT_
    /* Open mtjt */
    ret = in_mtjt_open( );
    if ( ret < 0 )
    {
        callback_unlock( );
        gui_unlock();
        return  -EMTJT_OPEN_RETURN;
    }
    #endif  /* _LG_MTJT_ */


    callback_unlock( );
    gui_unlock( );

    return  1;
}

int  gui_close( void )
{
    gui_lock( );
    callback_lock( );

    #ifdef  _LG_KEYBOARD_ 
    in_keyboard_close( );
    #endif

    #ifdef  _LG_MTJT_ 
    in_mtjt_close( );
    #endif

    #ifdef  _LG_SCREEN_
    in_screen_close();
    #endif

    #ifdef  _LG_WINDOW_
    in_win_close_all();
    #endif

    #if  ( defined(_LG_BITMAP_) || defined(_LG_ICON_) || defined(_LG_GIF_) )
    in_image_decode_free_buffer();
    #endif
 
    callback_unlock( );
    gui_unlock( );

    #ifdef  _LG_MULTI_THREAD_
    gui_locker_destroy( );
    callback_locker_destroy( );
    #endif  /* _LG_MULTI_THREAD_ */

    return  1;
} 
