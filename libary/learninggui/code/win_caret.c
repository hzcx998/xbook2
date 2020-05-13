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

#include  <lock.h>
#include  <cursor.h>

#include  <win_tools.h>
#include  <win_interface.h>
#include  <win_widget.h>

#include  <win_desktop.h>

#include  <win_caret.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"



#ifndef  CARET_BLINK_DEFAULT_INTERVAL
#define  CARET_BLINK_DEFAULT_INTERVAL        3000
#endif
#ifndef  CARET_DEFAULT_WIDTH
#define  CARET_DEFAULT_WIDTH                 4
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_CARET_

/* Caret blink flag */        
volatile  unsigned int    lcarfl = 1;

/* Caret index */        
volatile  unsigned int    lcarin = 0;

/* Caret widht */
volatile  unsigned int    lcarwi = CARET_DEFAULT_WIDTH;

/* Caret rect */        
volatile  GUI_RECT        lcarre = {-1, -1, -1, -1};



/* Show caret flag */
static  volatile  unsigned int    lcarsf = 1;

/* caret blink value */
static  volatile  unsigned int    lcarbv = CARET_BLINK_DEFAULT_INTERVAL;

/* Carete blink counter */
static  volatile  unsigned int    lcarbc = 0;


int  in_caret_deal(void)
{
    if ( lcarsf < 1 )
        return  0;

    lcarbc++;
    if ( lcarbc < lcarbv )
        return  0;

    lcarbc = 0;
    in_win_message_send_ext(lhfocu, MSG_CARET, HWND_IN_CALLBACK);

    return  1;
}

int  in_caret_init_blink(void)
{
    lcarbc = lcarbv;
    lcarfl    = 1;
    return  1;
}

int  in_caret_show(void)
{
    lcarbc = 0;
    lcarfl    = 1;
    lcarsf     = 1;
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  caret_show(void)
{
    int  ret = 0;

    gui_lock();
    ret = in_caret_show();
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_caret_hide(void)
{
    lcarsf     = 0;
    lcarfl    = 0;
    lcarbc = 0;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  caret_hide(void)
{
    int  ret = 0;

    gui_lock();
    ret = in_caret_hide();
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_caret_get_position(void)
{
    return  lcarin;
}

#ifndef  _LG_ALONE_VERSION_
int  caret_get_position(void)
{
    int  ret = 0;

    gui_lock();
    ret = in_caret_get_position();
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_caret_set_position(unsigned int index)
{
    lcarin = index;

    return  lcarin;
}

#ifndef  _LG_ALONE_VERSION_
int  caret_set_position(unsigned int index)
{
    int  ret = 0;

    gui_lock();
    ret = in_caret_set_position(index);
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_caret_get_blink_interval(void)
{
    return  lcarbv;
}

#ifndef  _LG_ALONE_VERSION_
int  caret_get_blink_interval(void)
{
    int  ret = 0;

    gui_lock();
    ret = in_caret_get_blink_interval();
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_caret_set_blink_interval(unsigned int interval)
{
    lcarbv = interval;
    return  lcarbv;
}

#ifndef  _LG_ALONE_VERSION_
int  caret_set_blink_interval(unsigned int interval)
{
    int  ret = 0;

    gui_lock();
    ret = in_caret_set_blink_interval(interval);
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_caret_get_width(void)
{
    return  lcarwi;
}

#ifndef  _LG_ALONE_VERSION_
int  caret_get_width(void)
{
    int  ret = 0;

    gui_lock();
    ret = in_caret_get_width();
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_caret_set_width(unsigned int width)
{
    lcarwi = width;
    return  lcarwi;
}

#ifndef  _LG_ALONE_VERSION_
int  caret_set_width(unsigned int width)
{
    int  ret = 0;

    gui_lock();
    ret = in_caret_set_width(width);
    gui_unlock();

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_CARET_ */
#endif  /* _LG_WINDOW_ */
