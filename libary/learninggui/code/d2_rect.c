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

#include  <stdio.h>

#include  <lock.h>
#include  <cursor.h>

#include  <dc.h>
#include  <d2_pixel.h>
#include  <d2_line.h>
#include  <screen.h>

#include  <d2_rect.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif


#ifndef  CHECK_CROSS_WIDTH
#define  CHECK_CROSS_WIDTH    3
#endif
#if  (CHECK_CROSS_WIDTH < 1)
#undef   CHECK_CROSS_WIDTH
#define  CHECK_CROSS_WIDTH    1
#endif

#ifndef  CHECK_CROSS_DELTA
#define  CHECK_CROSS_DELTA    2
#endif
#if (CHECK_CROSS_DELTA < 1)
#undef   CHECK_CROSS_DELTA
#define  CHECK_CROSS_DELTA    1
#endif


#ifdef	_LG_RECTANGLE_

/* Absolute coordinate */    
static  int  in_output_screen_rect_fill_abs(HDC hdc, int left, int top, int right, int bottom, SCREEN_COLOR  color)
{
    GUI_RECT    rect = { 0 };
    #ifdef  _LG_ALPHA_BLEND_
    int  i = 0;
    int  j = 0;
    #endif


    if ( hdc == NULL ) 
        return  -1;


    #ifdef  _LG_WINDOW_
    rect = ((HWND)(hdc->hwnd))->common.cur_clip_rect;
    #else
    rect = hdc->rect;
    #endif


    if ( left > rect.right )
        return  -1;
    if ( right < rect.left )
        return  -1;
    if ( top > rect.bottom )
        return  -1;
    if ( bottom < rect.top )
        return  -1;


    if ( left < rect.left )
        left = rect.left;
    if ( left > rect.right )
        left = rect.right;

    if ( right < rect.left )
        right = rect.left;
    if ( right > rect.right )
        right = rect.right;


    if ( top < rect.top )
        top = rect.top;
    if ( top > rect.bottom )
        top = rect.bottom;

    if ( bottom < rect.top )
        bottom = rect.top;
    if ( bottom > rect.bottom )
        bottom = rect.bottom;


    #ifdef  _LG_ALPHA_BLEND_
    if ( (hdc->is_alpha_blend) > 0 )
    {
        for ( j = 0; j < (bottom - top) + 1; j++ )
        {
            for ( i = 0; i < (right - left) + 1; i++ )
            {
                in_output_screen_pixel_abs(hdc, left + i, top + j, color);
            }
        }

        return  1;
    }
    #endif   


    if ( (lscrn.output_rect_fill) == NULL )
        return  -1;

    return  (lscrn.output_rect_fill)(left, top, right, bottom, color);
}
 

int  in_rect_frame(HDC hdc, int left, int top, int right, int bottom)
{
    int  temp = 0;


    if ( left > right )
    {
        temp  = left;
        left  = right;
        right = temp;
    }

    if ( top > bottom )
    {
        temp  = top;
        top   = bottom;
        bottom = temp;
    }

    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_
    if ( in_init_max_output_rect(hdc) < 1 )
    {
        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif
        (lscrn.output_sequence_end)();
        return  -1;
    }

    while ( 1 )
    {
        if ( in_get_current_clip_rect(hdc) < 1 )
            break;
    #endif

    in_paint_h_line_no_clip(hdc, left, right, top);
    in_paint_h_line_no_clip(hdc, left, right, bottom);
    in_paint_v_line_no_clip(hdc, top, bottom, left);
    in_paint_v_line_no_clip(hdc, top, bottom, right);

    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();
	
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  rect_frame(HDC hdc, int left, int top, int right, int bottom)
{
    int  ret = 0;

    gui_lock( );
    ret = in_rect_frame(hdc, left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif

int  in_rect_fill(HDC hdc, int left, int top, int right, int bottom)
{
    GUI_COLOR       gui_color    = GUI_BLACK;
    SCREEN_COLOR    screen_color = GUI_BLACK;
    GUI_COLOR       old_color    = GUI_BLACK;
    int             i    = 0; 
    int             temp = 0;


    if ( left > right )
    {
        temp  = left;
        left  = right;
        right = temp;
    }

    if ( top > bottom )
    {
        temp  = top;
        top   = bottom;
        bottom = temp;
    }

    gui_color    = in_hdc_get_back_color(hdc);
    screen_color = (lscrn.gui_to_screen_color)(gui_color);

    old_color    = in_hdc_get_fore_color(hdc);
    in_hdc_set_fore_color(hdc, gui_color);

    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_
    if ( in_init_max_output_rect(hdc) < 1 )
    {
        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif
        (lscrn.output_sequence_end)();

        in_hdc_set_fore_color(hdc, old_color);

        return  -1;
    }

    while ( 1 )
    {
        if ( in_get_current_clip_rect(hdc) < 1 )
            break;
    #endif


    if ( (lscrn.is_rect_fill_accelerate) > 0 )
    {
        in_output_screen_rect_fill_abs(hdc, left + hdc->rect.left, top + hdc->rect.top, right + hdc->rect.left, bottom + hdc->rect.top, screen_color);
    } else {
        for (i = top; i < (bottom+1); i++)
            in_paint_h_line_no_clip(hdc, left, right, i);
    }


    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();
    
    in_hdc_set_fore_color(hdc, old_color);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  rect_fill(HDC hdc, int left, int top, int right, int bottom)
{
    int  ret = 0;

    gui_lock( );
    ret =  in_rect_fill(hdc, left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif


int  in_frame_3d_up_rect(HDC hdc, int left, int top, int right, int bottom)
{
    GUI_COLOR  old_color = GUI_BLACK;
    int        temp      = 0;


    if ( left > right )
    {
        temp  = left;
        left  = right;
        right = temp;
    }

    if ( top > bottom )
    {
        temp  = top;
        top   = bottom;
        bottom = temp;
    }

    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_
    if ( in_init_max_output_rect(hdc) < 1 )
    {
        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif
        (lscrn.output_sequence_end)();
        return  -1;
    }

    while ( 1 )
    {
        if ( in_get_current_clip_rect(hdc) < 1 )
            break;
    #endif

    old_color = in_hdc_get_fore_color(hdc);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_BORDER_FCOLOR);
    in_paint_h_line_no_clip(hdc, left, right, top);
    in_paint_h_line_no_clip(hdc, left, right, bottom);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_MID_FCOLOR);
    in_paint_h_line_no_clip(hdc, left+1, right-2, top+1);
    in_paint_h_line_no_clip(hdc, left+1, right-2, top+2);

    in_hdc_set_fore_color(hdc, GUI_3D_UP_IN_FCOLOR);
    in_paint_h_line_no_clip(hdc, left+1, right-1, bottom-1);
    in_paint_h_line_no_clip(hdc, left+1, right-1, bottom-2);

    in_hdc_set_fore_color(hdc, old_color);

    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();
	
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  frame_3d_up_rect(HDC hdc, int left, int top, int right, int bottom)
{
    int  ret = 0;

    gui_lock( );
    ret = in_frame_3d_up_rect(hdc, left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif

int  in_frame_3d_down_rect(HDC hdc, int left, int top, int right, int bottom)
{
    int  temp = 0;


    if ( left > right )
    {
        temp  = left;
        left  = right;
        right = temp;
    }

    if ( top > bottom )
    {
        temp  = top;
        top   = bottom;
        bottom = temp;
    }

    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_
    if ( in_init_max_output_rect(hdc) < 1 )
    {
        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif
        (lscrn.output_sequence_end)();
        return  -1;
    }

    while ( 1 )
    {
        if ( in_get_current_clip_rect(hdc) < 1 )
            break;
    #endif

    in_paint_h_line_no_clip(hdc, left, right, top);
    in_paint_h_line_no_clip(hdc, left, right, bottom);
    in_paint_v_line_no_clip(hdc, top, bottom, left);
    in_paint_v_line_no_clip(hdc, top, bottom, right);

    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();
	
    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  frame_3d_down_rect(HDC hdc, int left, int top, int right, int bottom)
{
    int  ret = 0;

    gui_lock( );
    ret = in_frame_3d_down_rect(hdc, left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif

int  in_paint_check(HDC hdc, int left, int top, int right, int bottom)
{
    int    i       = 0; 
    int    temp    = 0;
    int    left0   = 0;
    int    top0    = 0;
    int    right0  = 0;
    int    bottom0 = 0;


    if ( left > right )
    {
        temp  = left;
        left  = right;
        right = temp;
    }

    if ( top > bottom )
    {
        temp  = top;
        top   = bottom;
        bottom = temp;
    }

    left0   = left + ((right-left)/6);
    top0    = top  + ((bottom-top)>>1);
    right0  = left + ((right-left)>>1);
    bottom0 = bottom - ((bottom-top)/4);

    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_
    if ( in_init_max_output_rect(hdc) < 1 )
    {
        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif
        (lscrn.output_sequence_end)();
        return  -1;
    }

    while ( 1 )
    {
        if ( in_get_current_clip_rect(hdc) < 1 )
            break;
    #endif

    for (i = 0; i < CHECK_CROSS_WIDTH; i++)
    {
        in_line(hdc, left0+i, top0, right0, bottom0-i);
        in_line(hdc, right0, bottom0-i, right - CHECK_CROSS_DELTA - i, top + CHECK_CROSS_DELTA );
    }

    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  paint_check(HDC hdc, int left, int top, int right, int bottom)
{
    int  ret = 0;

    gui_lock( );
    ret =  in_paint_check(hdc, left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif

int  in_paint_cross(HDC hdc, int left, int top, int right, int bottom)
{
    int    i       = 0; 
    int    temp    = 0;


    if ( left > right )
    {
        temp  = left;
        left  = right;
        right = temp;
    }

    if ( top > bottom )
    {
        temp  = top;
        top   = bottom;
        bottom = temp;
    }

    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + left, hdc->rect.top + top, hdc->rect.left + right, hdc->rect.top + bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_
    if ( in_init_max_output_rect(hdc) < 1 )
    {
        #ifndef  _LG_WINDOW_
        #ifdef  _LG_CURSOR_
        in_cursor_maybe_refresh( );
        #endif
        #endif
        (lscrn.output_sequence_end)();
        return  -1;
    }

    while ( 1 )
    {
        if ( in_get_current_clip_rect(hdc) < 1 )
            break;
    #endif

    for (i = 0; i < CHECK_CROSS_WIDTH; i++)
    {
        in_line(hdc, left + CHECK_CROSS_DELTA + i, bottom - CHECK_CROSS_DELTA, right - CHECK_CROSS_DELTA - 2*CHECK_CROSS_WIDTH + i , top + CHECK_CROSS_DELTA);
        in_line(hdc, left + CHECK_CROSS_DELTA + i, top + CHECK_CROSS_DELTA, right - CHECK_CROSS_DELTA - 2*CHECK_CROSS_WIDTH + i , bottom - CHECK_CROSS_DELTA);
    }

    #ifdef  _LG_WINDOW_
    }
    #endif

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif
    (lscrn.output_sequence_end)();

    return  1;  
}

#ifndef  _LG_ALONE_VERSION_
int  paint_cross(HDC hdc, int left, int top, int right, int bottom)
{
    int  ret = 0;

    gui_lock( );
    ret =  in_paint_cross(hdc, left, top, right, bottom);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_RECTANGLE_ */
