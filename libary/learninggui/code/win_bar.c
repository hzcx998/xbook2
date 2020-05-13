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

#include  <lgconst.h>

#include  <lock.h>
#include  <cursor.h>

#include  <d2_line.h>
#include  <d2_rect.h>

#include  <win_type_widget.h>

#include  <win_tools.h>
#include  <win_invalidate.h>
#include  <win_interface.h>

#include  <win_desktop.h>

#include  "win_default_in.h"
#include  "win_arithmetic_in.h"



/* Window Bar color */
#ifndef  WINBAR_WIN_DISABLED_BCOLOR 
#define  WINBAR_WIN_DISABLED_BCOLOR         0x00606060
#endif
#ifndef  WINBAR_WIN_DISABLED_FCOLOR 
#define  WINBAR_WIN_DISABLED_FCOLOR         GUI_BLACK
#endif
#ifndef  WINBAR_WIN_DISABLED_TBCOLOR 
#define  WINBAR_WIN_DISABLED_TBCOLOR        0x00606060
#endif
#ifndef  WINBAR_WIN_DISABLED_TFCOLOR 
#define  WINBAR_WIN_DISABLED_TFCOLOR        GUI_BLACK
#endif

#ifndef  WINBAR_WIN_INACTIVE_BCOLOR 
#define  WINBAR_WIN_INACTIVE_BCOLOR         0x007A96DF
#endif
#ifndef  WINBAR_WIN_INACTIVE_FCOLOR 
#define  WINBAR_WIN_INACTIVE_FCOLOR         GUI_BLACK
#endif
#ifndef  WINBAR_WIN_INACTIVE_TBCOLOR 
#define  WINBAR_WIN_INACTIVE_TBCOLOR        0x007A96DF
#endif
#ifndef  WINBAR_WIN_INACTIVE_TFCOLOR 
#define  WINBAR_WIN_INACTIVE_TFCOLOR        GUI_BLACK
#endif

#ifndef  WINBAR_WIN_ACTIVE_BCOLOR 
#define  WINBAR_WIN_ACTIVE_BCOLOR           0x000055E5
#endif
#ifndef  WINBAR_WIN_ACTIVE_FCOLOR 
#define  WINBAR_WIN_ACTIVE_FCOLOR           GUI_BLACK
#endif
#ifndef  WINBAR_WIN_ACTIVE_TBCOLOR 
#define  WINBAR_WIN_ACTIVE_TBCOLOR          0x000055E5
#endif
#ifndef  WINBAR_WIN_ACTIVE_TFCOLOR 
#define  WINBAR_WIN_ACTIVE_TFCOLOR          GUI_BLACK
#endif

#ifndef  WINBAR_CLI_DISABLED_BCOLOR 
#define  WINBAR_CLI_DISABLED_BCOLOR         0x00606060
#endif
#ifndef  WINBAR_CLI_DISABLED_FCOLOR 
#define  WINBAR_CLI_DISABLED_FCOLOR         GUI_WHITE
#endif
#ifndef  WINBAR_CLI_DISABLED_TBCOLOR 
#define  WINBAR_CLI_DISABLED_TBCOLOR        0x00606060
#endif
#ifndef  WINBAR_CLI_DISABLED_TFCOLOR 
#define  WINBAR_CLI_DISABLED_TFCOLOR        GUI_BLACK
#endif

#ifndef  WINBAR_CLI_INACTIVE_BCOLOR 
#define  WINBAR_CLI_INACTIVE_BCOLOR         0x007A96DF
#endif
#ifndef  WINBAR_CLI_INACTIVE_FCOLOR 
#define  WINBAR_CLI_INACTIVE_FCOLOR         GUI_WHITE
#endif
#ifndef  WINBAR_CLI_INACTIVE_TBCOLOR 
#define  WINBAR_CLI_INACTIVE_TBCOLOR        0x007A96DF
#endif
#ifndef  WINBAR_CLI_INACTIVE_TFCOLOR 
#define  WINBAR_CLI_INACTIVE_TFCOLOR        GUI_BLACK
#endif

#ifndef  WINBAR_CLI_ACTIVE_BCOLOR 
#define  WINBAR_CLI_ACTIVE_BCOLOR           0x000055E5
#endif
#ifndef  WINBAR_CLI_ACTIVE_FCOLOR 
#define  WINBAR_CLI_ACTIVE_FCOLOR           GUI_WHITE
#endif
#ifndef  WINBAR_CLI_ACTIVE_TBCOLOR 
#define  WINBAR_CLI_ACTIVE_TBCOLOR          0x000055E5
#endif
#ifndef  WINBAR_CLI_ACTIVE_TFCOLOR 
#define  WINBAR_CLI_ACTIVE_TFCOLOR          GUI_BLACK
#endif

#ifndef  WINBAR_CLOSE_DISABLED_BCOLOR
#define  WINBAR_CLOSE_DISABLED_BCOLOR       0x00D0C0C0
#endif
#ifndef  WINBAR_CLOSE_INACTIVE_BCOLOR
#define  WINBAR_CLOSE_INACTIVE_BCOLOR       0x00D0A0A0
#endif
#ifndef  WINBAR_CLOSE_ACTIVE_BCOLOR
#define  WINBAR_CLOSE_ACTIVE_BCOLOR         0x00E15025
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_WINDOW_BAR_

/* Temp window bar buffer */
volatile  GUI_WINBAR   ltwbar;


GUI_WINBAR  *in_win_get_window_bar(/* HWND hwnd */ void *hwnd)
{ 
    HWND       p    = (HWND)hwnd; 

    if ( p == NULL )
        return  NULL;
    if ( p == HWND_DESKTOP )
        return  NULL;
    if ( in_win_has(p) < 1 )
        return  NULL;
    if ( IS_WINBAR_WIDGET(&(p->common)) < 1 )
        return  NULL;

    if ( ((p->common.style)&WINBAR_STYLE) != WINBAR_STYLE )
        return  NULL;

    return  p->common.winbar;
}

int  in_win_paint_window_bar(/* HWND hwnd */ void *hwnd)
{
    HWND         p  = (HWND)hwnd;
    HDC          hdc;
    GUI_RECT     old_rect;
    GUI_WINBAR  *wbar = NULL;
    GUI_COLOR    old_bcolor;
    GUI_COLOR    old_fcolor;
    GUI_RECT     rect;
    int          ret = 0;
    int          i   = 0;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( in_win_has(p) < 1 )
        return  -1;
    ret = in_win_is_visual(p);
    if ( ret < 0 )
        return  -1;
    if ( IS_WINBAR_WIDGET(&(p->common)) < 1 )
        return  -1;


    hdc = in_hdc_get_window(p);
    old_rect  = hdc->rect;

    /* Draw bar rect */
    wbar = in_win_get_window_bar(p);
    if (wbar == NULL)
    {
        in_hdc_release_win(p, hdc);
        return  -1;
    }
    rect = wbar->bar_rect;
    hdc->rect = rect;

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
    #endif

    in_win_set_current_color_group(p);

    old_bcolor = in_hdc_get_back_color(hdc);
    old_fcolor = in_hdc_get_fore_color(hdc);

    ret = in_hdc_get_current_group(hdc);
    if ( ret == DISABLED_GROUP )
        in_hdc_set_back_color(hdc, WINBAR_WIN_DISABLED_BCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_back_color(hdc, WINBAR_WIN_ACTIVE_BCOLOR);
    else
        in_hdc_set_back_color(hdc, WINBAR_WIN_INACTIVE_BCOLOR);
         
    in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);


    /* Draw window title */
    hdc->rect    = wbar->title_rect;
    rect.left    = 0;
    rect.top     = 0;
    rect.right   = GUI_RECTW(&(hdc->rect)) - 1;
    rect.bottom  = GUI_RECTH(&(hdc->rect)) - 1;


    if ( ret == DISABLED_GROUP )
        in_hdc_set_fore_color(hdc, WINBAR_WIN_DISABLED_FCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_fore_color(hdc, WINBAR_WIN_ACTIVE_FCOLOR);
    else
        in_hdc_set_fore_color(hdc, WINBAR_WIN_INACTIVE_FCOLOR);
         
    in_text_out_rect(hdc, &rect, wbar->text, -1, LG_TA_VCENTER);


    /* Draw window system close button */
    if (IS_CLOSE_BTN_WINBAR(&(p->common)) < 1 )
        goto  DRAW_WINBAR_MAX;


    rect = wbar->close_rect;
    hdc->rect = rect;

    if ( ret == DISABLED_GROUP )
        in_hdc_set_back_color(hdc, WINBAR_CLOSE_DISABLED_BCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_back_color(hdc, WINBAR_CLOSE_ACTIVE_BCOLOR);
    else
        in_hdc_set_back_color(hdc, WINBAR_CLOSE_INACTIVE_BCOLOR);
        

    in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    if ( ret == DISABLED_GROUP )
        in_hdc_set_fore_color(hdc, WINBAR_CLI_DISABLED_FCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_fore_color(hdc, WINBAR_CLI_ACTIVE_FCOLOR);
    else
        in_hdc_set_fore_color(hdc, WINBAR_CLI_INACTIVE_FCOLOR);


    in_rect_frame(hdc, 0, 0 , GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    /* \ */
    for (i = 0; i < IN_SYSTEM_LINE_WIDTH; i++)
    {
         in_line(hdc, IN_CLOSE_BUTTON_OFFSET*2-i-1, IN_CLOSE_BUTTON_OFFSET,
                      GUI_RECTW(&rect)-1-IN_CLOSE_BUTTON_OFFSET-i, GUI_RECTH(&rect)-2-IN_CLOSE_BUTTON_OFFSET);
    }
    /* / */
    for (i = 0; i < IN_SYSTEM_LINE_WIDTH; i++)
    {
        in_line(hdc, IN_CLOSE_BUTTON_OFFSET+i-1, GUI_RECTH(&rect)-1-IN_CLOSE_BUTTON_OFFSET,
                     GUI_RECTW(&rect)-1-IN_CLOSE_BUTTON_OFFSET*2+i, IN_CLOSE_BUTTON_OFFSET+1);
    }

    /* Draw window system max button */
    DRAW_WINBAR_MAX:
    if (IS_MAX_BTN_WINBAR(&(p->common)) < 1)
        goto  DRAW_WINBAR_MIN;
   
    rect = wbar->max_rect;
    hdc->rect = rect;

    if ( ret == DISABLED_GROUP )
        in_hdc_set_back_color(hdc, WINBAR_CLI_DISABLED_BCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_back_color(hdc, WINBAR_CLI_ACTIVE_BCOLOR);
    else
        in_hdc_set_back_color(hdc, WINBAR_CLI_INACTIVE_BCOLOR);

 
    in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
  
    if ( ret == DISABLED_GROUP )
        in_hdc_set_fore_color(hdc, WINBAR_CLI_DISABLED_FCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_fore_color(hdc, WINBAR_CLI_ACTIVE_FCOLOR);
    else
        in_hdc_set_fore_color(hdc, WINBAR_CLI_INACTIVE_FCOLOR);


    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    if ( (wbar->status) == WINDOW_MAX_STATUS )
    {
        in_rect_fill(hdc, GUI_RECTW(&rect)-1-5*IN_MAX_BUTTON_OFFSET, IN_MAX_BUTTON_OFFSET, 
                          GUI_RECTW(&rect)-1-IN_MAX_BUTTON_OFFSET, GUI_RECTH(&rect)-1-3*IN_MAX_BUTTON_OFFSET);
        in_rect_frame(hdc, GUI_RECTW(&rect)-1-5*IN_MAX_BUTTON_OFFSET, IN_MAX_BUTTON_OFFSET, 
                          GUI_RECTW(&rect)-1-IN_MAX_BUTTON_OFFSET, GUI_RECTH(&rect)-1-3*IN_MAX_BUTTON_OFFSET);

        in_rect_fill(hdc, IN_MAX_BUTTON_OFFSET, 3*IN_MAX_BUTTON_OFFSET, 
                          5*IN_MAX_BUTTON_OFFSET, GUI_RECTH(&rect)-1-IN_MAX_BUTTON_OFFSET);
        in_rect_frame(hdc, IN_MAX_BUTTON_OFFSET, 3*IN_MAX_BUTTON_OFFSET, 
                          5*IN_MAX_BUTTON_OFFSET, GUI_RECTH(&rect)-1-IN_MAX_BUTTON_OFFSET);

    } else {
        in_rect_fill(hdc, IN_MAX_BUTTON_OFFSET, IN_MAX_BUTTON_OFFSET, 
                          GUI_RECTW(&rect)-1-IN_MAX_BUTTON_OFFSET, GUI_RECTH(&rect)-1-IN_MAX_BUTTON_OFFSET); 
        in_rect_frame(hdc, IN_MAX_BUTTON_OFFSET, IN_MAX_BUTTON_OFFSET, 
                          GUI_RECTW(&rect)-1-IN_MAX_BUTTON_OFFSET, GUI_RECTH(&rect)-1-IN_MAX_BUTTON_OFFSET);
 
        for (i = 0; i < IN_SYSTEM_LINE_WIDTH; i++)
        {
            in_line(hdc, IN_MAX_BUTTON_OFFSET, IN_MAX_BUTTON_OFFSET+i,
                         GUI_RECTW(&rect)-1-IN_MAX_BUTTON_OFFSET, IN_MAX_BUTTON_OFFSET+i);
        }
    }

    /* Draw window system min button */
    DRAW_WINBAR_MIN:
    if (IS_MIN_BTN_WINBAR(&(p->common)) < 1)
        goto  DRAW_WINBAR_END;


    rect = wbar->min_rect;
    hdc->rect = rect;

    if ( ret == DISABLED_GROUP )
        in_hdc_set_back_color(hdc, WINBAR_CLI_DISABLED_BCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_back_color(hdc, WINBAR_CLI_ACTIVE_BCOLOR);
    else
        in_hdc_set_back_color(hdc, WINBAR_CLI_INACTIVE_BCOLOR);


    in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);


    if ( ret == DISABLED_GROUP )
        in_hdc_set_fore_color(hdc, WINBAR_CLI_DISABLED_FCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_fore_color(hdc, WINBAR_CLI_ACTIVE_FCOLOR);
    else
        in_hdc_set_fore_color(hdc, WINBAR_CLI_INACTIVE_FCOLOR);


    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    for (i = 0; i < IN_SYSTEM_LINE_WIDTH; i++)
    {
         in_line(hdc, IN_MIN_BUTTON_OFFSET, GUI_RECTH(&rect)-1-IN_MIN_BUTTON_OFFSET-i,
                      (GUI_RECTW(&rect))>>1, GUI_RECTH(&rect)-1-IN_MIN_BUTTON_OFFSET-i);
    }

    DRAW_WINBAR_END:
    hdc->rect = old_rect;

    in_hdc_set_back_color(hdc, old_bcolor);
    in_hdc_set_fore_color(hdc, old_fcolor);

    in_hdc_release_win(p, hdc);

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
 
    return  1;
}

int  in_win_normal(/* HWND hwnd */ void *hwnd)
{
    HWND        p = (HWND)hwnd;
    HWND        temp_p = NULL;
    HDC         hdc;
    GUI_RECT    rect;
    GUI_RECT    temp_rect;
    GUI_WINBAR *bar = NULL;
    int         ret = 0;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( in_win_has(p) < 1 )
        return  -1;
    ret = in_win_is_visual(p);
    if ( ret < 0 )
        return  -1;
    if ( !IS_WINBAR_WIDGET(&(p->common)) )
        return  -1;


    bar  = in_win_get_window_bar(p);
    if ( bar == NULL )
        return  -1;

    if ( (bar->status) == WINDOW_NORMAL_STATUS )
        return  0;

    rect = p->common.win_dc.rect;
    in_win_invalidate_below_area_abs(p, &(p->common.win_dc.rect));

    /* Update window bar rect */
    if ( win_bar_height < IN_MIN_WINDOW_BAR_HEIGHT )
        win_bar_height = IN_MIN_WINDOW_BAR_HEIGHT;

    if ( win_system_button_width < IN_MIN_SYSTEM_BUTTON_WIDTH )
        win_system_button_width = IN_MIN_SYSTEM_BUTTON_WIDTH;

    rect                                = bar->raw_rect;
    temp_rect                           = rect;

    bar->status                          = WINDOW_NORMAL_STATUS;
    if (IS_BORDER_WIDGET(&(p->common)))
    {
        rect.left                      += win_border_width;
        rect.right                     -= win_border_width;
        rect.top                       += win_border_width;
    }
    rect.bottom                         = rect.top + (win_bar_height-1);
    bar->bar_rect                       = rect;
    bar->title_rect                     = bar->bar_rect;
    bar->title_rect.right              -= IN_SYSTEM_BUTTON_H_DISTANCE;

    bar->close_rect.left                 = -1;
    bar->close_rect.right                = -1;
    bar->close_rect.top                  = -1;
    bar->close_rect.bottom               = -1;

    bar->max_rect.left                   = -1;
    bar->max_rect.right                  = -1;
    bar->max_rect.top                    = -1;
    bar->max_rect.bottom                 = -1;

    bar->min_rect.left                   = -1;
    bar->min_rect.right                  = -1;
    bar->min_rect.top                    = -1;
    bar->min_rect.bottom                 = -1;

    rect.right                         -= IN_SYSTEM_BUTTON_H_DISTANCE;
    rect.top                           += IN_SYSTEM_BUTTON_VT_DISTANCE;
    rect.bottom                        -= IN_SYSTEM_BUTTON_VB_DISTANCE;

    /* System close button rect */
    if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
    {
        bar->close_rect                  = rect;
        bar->close_rect.left             = bar->close_rect.right - (win_system_button_width-1);
    } 

    /* System max button rect */
    if ( IS_MAX_BTN_WINBAR(&(p->common)) )
    {
        bar->max_rect                    = rect;
        if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
            bar->max_rect.right         -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        bar->max_rect.left               = bar->max_rect.right - (win_system_button_width-1);
    }

    /* System mix button rect */
    if ( IS_MIN_BTN_WINBAR(&(p->common)) )
    {
        bar->min_rect                    = rect;
        if ( IS_CLOSE_BTN_WINBAR(&(p->common)) )
            bar->min_rect.right         -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        if ( IS_MAX_BTN_WINBAR(&(p->common)) )
            bar->min_rect.right         -= (win_system_button_width-1) + IN_SYSTEM_BUTTON_H_DISTANCE;

        bar->min_rect.left               = bar->min_rect.right - (win_system_button_width-1);
 
    }

    /* Update client rect */
    rect         = bar->bar_rect;
    rect.top    += win_bar_height;
    rect.bottom  = temp_rect.bottom;
    if (IS_BORDER_WIDGET(&(p->common)))
        rect.bottom  -= win_border_width;

    hdc  = in_hdc_get_client(p);
    hdc->rect = rect;
    in_hdc_release_win(p, hdc);

    /* Update window rect */
    hdc  = in_hdc_get_window(p);
    hdc->rect = temp_rect;
    in_hdc_release_win(p, hdc);

    /* Update all child window(s) rect */
    temp_p = p->head.fc;
    while (temp_p != NULL)
    {
        in_win_move(temp_p, bar->raw_rect.left, bar->raw_rect.top);
        temp_p = temp_p->head.next;
    }

    in_win_invalidate_below_area_abs(p, &(p->common.win_dc.rect));

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_normal(/* HWND hwnd */ void *hwnd)
{
    int   ret = 0;

    gui_lock( );
    ret = in_win_normal(hwnd);
    gui_unlock( );

    return  ret;
}
#endif


int  in_win_set_window_bar_text(/* HWND hwnd */ void *hwnd, TCHAR *text, unsigned int text_len)
{
    HWND         p    = (HWND)hwnd;
    GUI_WINBAR  *wbar = NULL;
    int          len  = 0;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;

    if ( !IS_WINBAR_WIDGET(&(p->common)) )
        return  -1;


    if ( text == NULL )
        return  -1;

    wbar  = in_win_get_window_bar(p);
    if ( wbar == NULL )
        return  -1;

    len = (MAX_WIN_TEXT_LEN > text_len) ? text_len : MAX_WIN_TEXT_LEN;
    memset(wbar->text, 0, MAX_WIN_TEXT_LEN);
    memcpy(wbar->text, text, len);
    wbar->len = len;

    if ( in_win_is_visual(p) )
        in_win_invalidate_rect(p, &(wbar->bar_rect));

    return  len;
}

#ifndef  _LG_ALONE_VERSION_
int  win_set_window_bar_text(/* HWND hwnd */ void *hwnd, TCHAR *text, unsigned int text_len)
{
    int   ret = 0;

    gui_lock( );
    ret = in_win_set_window_bar_text(hwnd, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_win_get_window_bar_text(/* HWND hwnd */ void *hwnd, TCHAR *text, unsigned int *text_len)
{
    HWND         p    = (HWND)hwnd;
    GUI_WINBAR  *wbar = NULL;
    int          len  = 0;


    if ( p == NULL )
        return  -1;
    if ( p == HWND_DESKTOP )
        return  -1;
    if ( !IS_WINBAR_WIDGET(&(p->common)) )
        return  -1;

    if ( text == NULL )
        return  -1;
    if ( text_len == NULL )
        return  -1;


    wbar = in_win_get_window_bar(p);
    if ( wbar == NULL )
        return  -1;

    len = ((wbar->len) > *text_len) ? *text_len : (wbar->len);
    memcpy(text, wbar->text, len);
   
    *text_len = len;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  win_get_window_bar_text(/* HWND hwnd */ void *hwnd, TCHAR *text, unsigned int *text_len)
{
    int   ret = 0;

    gui_lock( );
    ret = in_win_get_window_bar_text(hwnd, text, text_len);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_WINDOW_BAR_ */
#endif  /* _LG_WINDOW_ */
