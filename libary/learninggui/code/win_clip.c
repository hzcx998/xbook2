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

#include  <rect_ops.h>

#include  <win_interface.h>
#include  <win_desktop.h>

#include  <win_clip.h>


#ifdef  _LG_WINDOW_

/* Clip rect  */
static  volatile  CLIP_RECT   lclipr;


int  in_init_max_output_rect(HDC  hdc)
{
    HWND      p      = NULL;
    HWND      parent = NULL;
    GUI_RECT  rect;
    GUI_RECT  temp_rect;
    int       ret    = 0;


    if ( hdc == NULL )
        return  0;

    memset((void *)(&lclipr), 0, sizeof(CLIP_RECT));

    rect = hdc->rect;
    lclipr.output_rect = rect;
    lclipr.output_flag = 1;


    p = hdc->hwnd;
    if ( p == NULL )
        return  -1;


    if ( ((p->common.style)&VISUAL_STYLE) != VISUAL_STYLE )
    {
        lclipr.output_flag = 0;
        return  -1;
    }


    ret = in_win_is_ghost(p);
    if ( ret > 0 )
        return  -1;

    if ( (p->common.invalidate_flag) > 0 )
    {
        rect = p->common.invalidate_rect;
        lclipr.output_rect = rect;
        return  1;
    }

    /* ?? */
    if ( p == HWND_DESKTOP )
        return  1;


    temp_rect = rect;
    while ( 1 )
    {
        parent = p->head.parent;
        if ( parent == HWND_DESKTOP )
            break;
        if ( parent == NULL )
            break;

        if ( ((parent->common.style)&VISUAL_STYLE) != VISUAL_STYLE )
        {
            lclipr.output_flag = 0;
            return  -1;
        }

        /* 20170622 */
        if ( in_win_is_ghost(parent) > 0 )
        {
            lclipr.output_flag = 0;
            return  -1;
        }

        /* Importance ?? */
        ret = in_intersect_rect(&temp_rect, &(parent->common.client_dc.rect), &rect);
        if ( ret < 1 )
        {
            lclipr.output_flag = 0;
            return  -1;
        }

        p    = parent;
        rect = temp_rect;
    }

    lclipr.output_rect = rect;

    return  1;
}

int  in_get_current_clip_rect(HDC hdc)
{
    GUI_RECT    rect;
    GUI_RECT    temp_rect;
    HWND        p         = NULL;
    HWND        parent    = NULL;
    HWND        temp_hwnd = NULL;


    if (hdc == NULL)
        return  -1;

    if ( (lclipr.output_flag) < 1 )
        return  -1;

    /* Init */
    if ( (lclipr.clip_num) < 1 )
    {
        rect        = lclipr.output_rect;
    } else {
        rect.left   = lclipr.cur_clip_rect.right + 1;
        rect.top    = lclipr.cur_clip_rect.top;
        rect.right  = lclipr.output_rect.right;
        rect.bottom = lclipr.cur_clip_rect.bottom;
    }

    p = (HWND)(hdc->hwnd);
    if ( p == NULL )
    {
        lclipr.output_flag = 0;
        return  -1;
    }

    /* Looking for the top main parent hwnd */
    while ( 1 )
    {
        parent = p->head.parent;
        if ( parent == NULL )
            break;
        if ( parent == HWND_DESKTOP )
            break;

        if ( ((parent->common.style)&VISUAL_STYLE) != VISUAL_STYLE )
        {
            lclipr.output_flag = 0;
            return  -1;
        }

        /* 20170622 */
        if ( in_win_is_ghost(parent) > 0 )
        {
            lclipr.output_flag = 0;
            return  -1;
        }

        p = parent;
    }

    if ( (p->head.next) == NULL )
    {
        lclipr.output_flag = 0;
        goto  FIND_CUR_CLIP_RECT;
    }

    NEW_BOUNDING:
    if ( (rect.left) > (lclipr.output_rect.right) )
    {
        rect.left   = lclipr.output_rect.left;
        rect.top    = lclipr.cur_clip_rect.bottom + 1;
        /* ?? */
        rect.right  = lclipr.output_rect.right;
        rect.bottom = lclipr.output_rect.bottom;
    }

    /* Finish to clip rect */
    if ( rect.top > lclipr.output_rect.bottom )
    {
        lclipr.output_flag = 0;
        return  0;
    }

    /* New bounding */
    temp_hwnd = p;
    if ( (rect.left) == (lclipr.output_rect.left) )
    {
        while  ( 1 )
        {
            temp_hwnd = temp_hwnd->head.next;
            if ( temp_hwnd == NULL )
                break;

            if ( ((temp_hwnd->common.style)&VISUAL_STYLE) != VISUAL_STYLE )
                continue;

            /* 20170622 */
            if ( in_win_is_ghost(temp_hwnd) > 0 )
                continue;

            temp_rect = temp_hwnd->common.win_dc.rect;
            if ( in_is_intersect_rect(&rect, &temp_rect) < 1 )
                continue;

            if ( rect.top < temp_rect.top )
            {
                if ( rect.bottom > (temp_rect.top - 1) )
                    rect.bottom = temp_rect.top - 1;
            } else {
                if ( rect.bottom > temp_rect.bottom )
                    rect.bottom = temp_rect.bottom;
            }
        }
    }

    FIND_RECT_LEFT:
    temp_hwnd  = p;
    rect.right = rect.left;
    while  ( 1 )
    {
        temp_hwnd = temp_hwnd->head.next;
        if ( temp_hwnd == NULL )
            break;

        if ( ((temp_hwnd->common.style)&VISUAL_STYLE) != VISUAL_STYLE )
            continue;

        /* 20170622 */
        if ( in_win_is_ghost(temp_hwnd) > 0 )
            continue;

        temp_rect = temp_hwnd->common.win_dc.rect;
        if ( in_is_intersect_rect(&rect, &temp_rect) < 1 )
            continue;

        rect.left = temp_rect.right + 1;
        goto  FIND_RECT_LEFT;
    }

    rect.right = lclipr.output_rect.right;
    if ( rect.left > rect.right )
    {
        lclipr.cur_clip_rect = rect;
        goto  NEW_BOUNDING;
    }

    /* rect.right */
    temp_hwnd = p;
    while  ( 1 )
    {
        temp_hwnd = temp_hwnd->head.next;
        if ( temp_hwnd == NULL )
            break;

        if ( ((temp_hwnd->common.style)&VISUAL_STYLE) != VISUAL_STYLE )
            continue;

        /* 20170622 */
        if ( in_win_is_ghost(temp_hwnd) > 0 )
            continue;

        temp_rect = temp_hwnd->common.win_dc.rect;
        if ( in_is_intersect_rect(&rect, &temp_rect) < 1 )
            continue;

        rect.right = temp_rect.left - 1;
    }

    /* Find a clip rect */
    FIND_CUR_CLIP_RECT:
    lclipr.cur_clip_rect = rect; 
    ((HWND)(hdc->hwnd))->common.cur_clip_rect = lclipr.cur_clip_rect;
    lclipr.clip_num++;
    /*
    printf("lclipr.cur_clip_rect: %d, %d, %d, %d\n", lclipr.cur_clip_rect.left, lclipr.cur_clip_rect.top, lclipr.cur_clip_rect.right, lclipr.cur_clip_rect.bottom);
    */

    return  1;
}
#endif  /* _LG_WINDOW_ */
