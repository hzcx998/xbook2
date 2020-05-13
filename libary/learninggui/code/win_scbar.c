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

#include  <d2_line.h>
#include  <d2_rect.h>
#include  <d2_triangle.h>

#include  <win_type_widget.h>
#include  <win_desktop.h>
#include  <win_interface.h>

#include  <win_scbar.h>

#include  "win_default_in.h"



/* Scroll Bar color */
#ifndef  SCBAR_WIN_DISABLED_BCOLOR 
#define  SCBAR_WIN_DISABLED_BCOLOR         0x00606060
#endif
#ifndef  SCBAR_WIN_DISABLED_FCOLOR 
#define  SCBAR_WIN_DISABLED_FCOLOR         GUI_BLACK
#endif
#ifndef  SCBAR_WIN_DISABLED_TBCOLOR 
#define  SCBAR_WIN_DISABLED_TBCOLOR        0x00606060
#endif
#ifndef  SCBAR_WIN_DISABLED_TFCOLOR 
#define  SCBAR_WIN_DISABLED_TFCOLOR        GUI_BLACK
#endif

#ifndef  SCBAR_WIN_INACTIVE_BCOLOR 
#define  SCBAR_WIN_INACTIVE_BCOLOR         0x007A96DF
#endif
#ifndef  SCBAR_WIN_INACTIVE_FCOLOR 
#define  SCBAR_WIN_INACTIVE_FCOLOR         GUI_BLACK
#endif
#ifndef  SCBAR_WIN_INACTIVE_TBCOLOR 
#define  SCBAR_WIN_INACTIVE_TBCOLOR        0x007A96DF
#endif
#ifndef  SCBAR_WIN_INACTIVE_TFCOLOR 
#define  SCBAR_WIN_INACTIVE_TFCOLOR        GUI_BLACK
#endif

#ifndef  SCBAR_WIN_ACTIVE_BCOLOR 
#define  SCBAR_WIN_ACTIVE_BCOLOR           0x000055E5
#endif
#ifndef  SCBAR_WIN_ACTIVE_FCOLOR 
#define  SCBAR_WIN_ACTIVE_FCOLOR           GUI_BLACK
#endif
#ifndef  SCBAR_WIN_ACTIVE_TBCOLOR 
#define  SCBAR_WIN_ACTIVE_TBCOLOR          0x000055E5
#endif
#ifndef  SCBAR_WIN_ACTIVE_TFCOLOR 
#define  SCBAR_WIN_ACTIVE_TFCOLOR          GUI_BLACK
#endif

#ifndef  SCBAR_CLI_DISABLED_BCOLOR 
#define  SCBAR_CLI_DISABLED_BCOLOR         0x00606060
#endif
#ifndef  SCBAR_CLI_DISABLED_FCOLOR 
#define  SCBAR_CLI_DISABLED_FCOLOR         GUI_WHITE
#endif
#ifndef  SCBAR_CLI_DISABLED_TBCOLOR 
#define  SCBAR_CLI_DISABLED_TBCOLOR        0x00606060
#endif
#ifndef  SCBAR_CLI_DISABLED_TFCOLOR 
#define  SCBAR_CLI_DISABLED_TFCOLOR        GUI_BLACK
#endif

#ifndef  SCBAR_CLI_INACTIVE_BCOLOR 
#define  SCBAR_CLI_INACTIVE_BCOLOR         0x007A96DF
#endif
#ifndef  SCBAR_CLI_INACTIVE_FCOLOR 
#define  SCBAR_CLI_INACTIVE_FCOLOR         GUI_WHITE
#endif
#ifndef  SBAR_CLI_INACTIVE_TBCOLOR 
#define  SBAR_CLI_INACTIVE_TBCOLOR        0x007A96DF
#endif
#ifndef  SCBAR_CLI_INACTIVE_TFCOLOR 
#define  SCBAR_CLI_INACTIVE_TFCOLOR        GUI_BLACK
#endif

#ifndef  SCBAR_CLI_ACTIVE_BCOLOR 
#define  SCBAR_CLI_ACTIVE_BCOLOR           0x000055E5
#endif
#ifndef  SCBAR_CLI_ACTIVE_FCOLOR 
#define  SCBAR_CLI_ACTIVE_FCOLOR           GUI_WHITE
#endif
#ifndef  SCBAR_CLI_ACTIVE_TBCOLOR 
#define  SCBAR_CLI_ACTIVE_TBCOLOR          0x000055E5
#endif
#ifndef  SCBAR_CLI_ACTIVE_TFCOLOR 
#define  SCBAR_CLI_ACTIVE_TFCOLOR          GUI_BLACK
#endif

#ifndef  SCBAR_CLOSE_DISABLED_BCOLOR
#define  SCBAR_CLOSE_DISABLED_BCOLOR       0x00D0C0C0
#endif
#ifndef  SCBAR_CLOSE_INACTIVE_BCOLOR
#define  SCBAR_CLOSE_INACTIVE_BCOLOR       0x00D0A0A0
#endif
#ifndef  SCBAR_CLOSE_ACTIVE_BCOLOR
#define  SCBAR_CLOSE_ACTIVE_BCOLOR         0x00E15025
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_SCROLL_BAR_

/* Temp scroll bar buffer */
volatile  GUI_SCBAR   lscbar;


GUI_SCBAR  *in_get_schbar(/* HWND hwnd */ void *hwnd)
{ 
    HWND   p = (HWND)hwnd; 


    if ( p == NULL )
        return  NULL;
    if ( p == HWND_DESKTOP )
        return  NULL;
    if ( IS_HBAR_SCBAR(&(p->common)) < 1 )
        return  NULL;

    return  p->common.schbar;
}

int  in_paint_schbar(/* HWND hwnd */ void *hwnd)
{
    HWND          p = (HWND)hwnd;
    HDC           hdc;
    GUI_RECT      old_rect;
    GUI_SCBAR    *scbar = NULL;
    GUI_COLOR     old_bcolor;
    GUI_COLOR     old_fcolor;
    unsigned int  old_mode;
    GUI_RECT      rect;
    int           ret = 0;
    int           i = 0;


    /* Draw sbar rect */
    scbar = in_get_schbar(p);
    if (scbar == NULL)
        return  -1;

    hdc = in_hdc_get_window(p);
    if ( hdc == NULL )
        return  -1;

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
    #endif

    old_rect   = hdc->rect;
    old_bcolor = in_hdc_get_back_color(hdc);
    old_fcolor = in_hdc_get_fore_color(hdc);
    old_mode   = in_hdc_get_back_mode(hdc);

    rect = scbar->bar_rect;
    hdc->rect = rect;

    /* Sbar border */
    in_win_set_current_color_group(p);

    ret = in_hdc_get_current_group(hdc);
    if ( ret == DISABLED_GROUP )
        in_hdc_set_back_color(hdc, SCBAR_WIN_DISABLED_BCOLOR);
    else if ( ret == ACTIVE_GROUP )
        in_hdc_set_back_color(hdc, SCBAR_WIN_ACTIVE_BCOLOR);
    else
        in_hdc_set_back_color(hdc, SCBAR_WIN_INACTIVE_BCOLOR);


    in_hdc_set_back_mode(hdc, MODE_COPY);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_BORDER_FCOLOR);
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_MID_FCOLOR);
    for ( i= 0; i < 2; i++)
        in_line(hdc, 1, 1+i, GUI_RECTW(&rect)-2, 1+i);
    for ( i= 0; i < 2; i++)
        in_line(hdc, 1+i, 2, 1+i, GUI_RECTH(&rect)-2);
    
    in_hdc_set_fore_color(hdc, GUI_3D_UP_IN_FCOLOR);
    for ( i= 0; i < 2; i++)
        in_line(hdc, 1, GUI_RECTH(&rect)-1-i, GUI_RECTW(&rect)-2, GUI_RECTH(&rect)-1-i);
    for ( i= 0; i < 2; i++)
        in_line(hdc, GUI_RECTW(&rect)-1-i, 2, GUI_RECTW(&rect)-1-i, GUI_RECTH(&rect)-1);

    /* Sbar fbtn rect */ 
    rect = scbar->fbtn_rect;
    hdc->rect = rect;
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
    in_fill_triangle(hdc, 1, GUI_RECTH(&rect)/2, GUI_RECTW(&rect)-1-3, 3, TRIANGLE_LEFT);
    in_fill_triangle(hdc, 1, GUI_RECTH(&rect)/2, GUI_RECTW(&rect)-1-3, GUI_RECTH(&rect)-1-3, TRIANGLE_LEFT);

    /* Sbar lftn rect */
    rect = scbar->lbtn_rect;
    hdc->rect = rect;
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
    in_fill_triangle(hdc, 3, 3, GUI_RECTW(&rect)-1-1, GUI_RECTH(&rect)/2, TRIANGLE_RIGHT);
    in_fill_triangle(hdc, 3, GUI_RECTH(&rect)-1-3, GUI_RECTW(&rect)-1-1, GUI_RECTH(&rect)/2, TRIANGLE_RIGHT);


    /* Sbar scroll rect */
    rect.left  = scbar->fbtn_rect.right+1;
    rect.right = scbar->lbtn_rect.left -1;
    hdc->rect = rect;
    in_hdc_set_back_color(hdc, GUI_GRAY);

    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    if ( (p->common.bimage_flag) < 1 )
    #endif
        in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    rect = scbar->mid_rect;
    hdc->rect = rect;
    in_hdc_set_back_color(hdc, GUI_LIGHT_GRAY);
    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    if ( (p->common.bimage_flag) < 1 )
    #endif 
        in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    /* Restor the old value */
    hdc->rect = old_rect;
    in_hdc_set_back_color(hdc, old_bcolor);
    in_hdc_set_fore_color(hdc, old_fcolor);
    in_hdc_set_back_mode(hdc, old_mode);

    in_hdc_release_win(p, hdc);

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
 
    return  1;
}

GUI_SCBAR  *in_get_scvbar(/* HWND hwnd */ void *hwnd)
{ 
    HWND   p = (HWND)hwnd; 


    if ( p == NULL )
        return  NULL;
    if ( p == HWND_DESKTOP )
        return  NULL;

    if ( IS_VBAR_SCBAR(&(p->common)) < 1 )
        return  NULL;

    return  p->common.scvbar;
}

int  in_paint_scvbar(/* HWND hwnd */ void *hwnd)
{
    HWND          p = (HWND)hwnd;
    HDC           hdc;
    GUI_RECT      old_rect;
    GUI_SCBAR    *scbar = NULL;
    GUI_COLOR     old_bcolor;
    GUI_COLOR     old_fcolor;
    GUI_COLOR     tmp_color;
    unsigned int  old_mode;
    GUI_RECT      rect;
    int           ret = 0;
    int           i = 0;


    in_win_set_current_color_group(p);

    /* Draw sbar rect */
    scbar = in_get_scvbar(p);
    if (scbar == NULL)
        return  -1;

    hdc = in_hdc_get_window(p);
    if ( hdc == NULL )
        return  -1;

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
    #endif

    old_rect   = hdc->rect;
    old_bcolor = in_hdc_get_back_color(hdc);
    old_fcolor = in_hdc_get_fore_color(hdc);
    old_mode   = in_hdc_get_back_mode(hdc);

    rect = scbar->bar_rect;
    hdc->rect = rect;

    /* Sbar border */
    ret = in_hdc_get_current_group(hdc);
    if ( ret == DISABLED_GROUP )
        tmp_color = SCBAR_WIN_DISABLED_BCOLOR;
    else if ( ret == ACTIVE_GROUP )
        tmp_color = SCBAR_WIN_ACTIVE_BCOLOR;
    else
        tmp_color = SCBAR_WIN_INACTIVE_BCOLOR;

    in_hdc_set_back_mode(hdc, MODE_COPY);
    in_hdc_set_back_color(hdc, tmp_color);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_BORDER_FCOLOR);
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_MID_FCOLOR);
    for ( i= 0; i < 2; i++)
        in_line(hdc, 1, 1+i, GUI_RECTW(&rect)-2, 1+i);
    for ( i= 0; i < 2; i++)
        in_line(hdc, 1+i, 2, 1+i, GUI_RECTH(&rect)-2);
    
    in_hdc_set_fore_color(hdc, GUI_3D_UP_IN_FCOLOR);
    for ( i= 0; i < 2; i++)
        in_line(hdc, 1, GUI_RECTH(&rect)-1-i, GUI_RECTW(&rect)-2, GUI_RECTH(&rect)-1-i);
    for ( i= 0; i < 2; i++)
        in_line(hdc, GUI_RECTW(&rect)-1-i, 2, GUI_RECTW(&rect)-1-i, GUI_RECTH(&rect)-1);


    /* Sbar fbtn rect */ 
    rect = scbar->fbtn_rect;
    hdc->rect = rect;
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
    in_fill_triangle(hdc, 3, GUI_RECTH(&rect)-1-3, GUI_RECTW(&rect)/2, 3, TRIANGLE_UP);
    in_fill_triangle(hdc, GUI_RECTW(&rect)-1-3, GUI_RECTH(&rect)-1-3, GUI_RECTW(&rect)/2, 3, TRIANGLE_UP);


    /* Sbar lbtn rect */ 
    rect = scbar->lbtn_rect;
    hdc->rect = rect;
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
    in_fill_triangle(hdc, 3, 3, GUI_RECTW(&rect)/2, GUI_RECTH(&rect)-1-3, TRIANGLE_DOWN);
    in_fill_triangle(hdc, GUI_RECTW(&rect)-1-3, 3, GUI_RECTW(&rect)/2, GUI_RECTH(&rect)-1-3, TRIANGLE_DOWN);


    /* Sbar scroll rect */ 
    rect.top    = scbar->fbtn_rect.bottom+1;
    rect.bottom = scbar->lbtn_rect.top-1;
    hdc->rect = rect;
    in_hdc_set_back_color(hdc, GUI_GRAY);
    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    if ( (p->common.bimage_flag) < 1 )
    #endif 
        in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

    rect = scbar->mid_rect;
    hdc->rect = rect;
    in_hdc_set_back_color(hdc, GUI_LIGHT_GRAY);
    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    if ( (p->common.bimage_flag) < 1 )
    #endif 
        in_rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
    in_rect_frame(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);


    /* Restore the old value */
    hdc->rect = old_rect;
    in_hdc_set_back_color(hdc, old_bcolor);
    in_hdc_set_fore_color(hdc, old_fcolor);
    in_hdc_set_back_mode(hdc, old_mode);

    in_hdc_release_win(p, hdc);

    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
 
    return  1;
}

#endif  /* _LG_SCROLL_BAR_ */
#endif  /* _LG_WINDOW_ */
