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

#include  <rect_ops.h>
#include  <lgmacro.h>


/* Calc the smallest rectangle containing both rects */
int   in_merge_rect(GUI_RECT *pdest, const GUI_RECT *pr0, const GUI_RECT *pr1)
{
    if ( (pdest == NULL) || (pr0 == NULL) || (pr1 == NULL) )
        return  -1;

    pdest->left   = GUI_MIN(pr0->left, pr1->left);
    pdest->top    = GUI_MIN(pr0->top, pr1->top);
    pdest->right  = GUI_MAX(pr0->right, pr1->right);
    pdest->bottom = GUI_MAX(pr0->bottom, pr1->bottom);

    if ( ((pdest->left)<=(pdest->right)) && ((pdest->top)<=(pdest->bottom)) )
        return  1;

    return  -1;
}

/* Calc intersection of rectangles */
int  in_intersect_rect(GUI_RECT *pdest, const GUI_RECT *pr0, const GUI_RECT *pr1) 
{
    if ( (pdest == NULL) || (pr0 == NULL) || (pr1 == NULL) )
        return  -1;

    pdest->left   = GUI_MAX(pr0->left, pr1->left);
    pdest->top    = GUI_MAX(pr0->top, pr1->top); 
    pdest->right  = GUI_MIN(pr0->right, pr1->right);
    pdest->bottom = GUI_MIN(pr0->bottom, pr1->bottom);

    if ( ((pdest->left)<=(pdest->right)) && ((pdest->top)<=(pdest->bottom)) )
        return  1;

    return  -1;
}

/* Reduce rectangle borders */
int  in_reduce_rect(GUI_RECT *pdest, const GUI_RECT *pr0, int dist) 
{
    if ( (pdest == NULL) || (pr0 == NULL) )
        return  -1;

    pdest->left   = pr0->left + dist;
    pdest->top    = pr0->top + dist;
    pdest->right  = pr0->right - dist;
    pdest->bottom = pr0->bottom - dist;

    if ( ((pdest->left)<=(pdest->right)) && ((pdest->top)<=(pdest->bottom)) )
        return  1;

    return  -1;
}

/* Move rectangle borders (delta distance) */
int  in_move_delta_rect(GUI_RECT *pr0, int dx, int dy) 
{
    if (pr0 == NULL)
        return  -1;

    pr0->left   += dx;
    pr0->top    += dy;
    pr0->right  += dx;
    pr0->bottom += dy;
 
    if ( ((pr0->left)<=(pr0->right)) && ((pr0->top)<=(pr0->bottom)) )
        return  1;

    return  -1;
}


/* Check if both rectangles do intersect */
int  in_is_intersect_rect(const GUI_RECT *pr0, const GUI_RECT *pr1) 
{
    if ( (pr0 == NULL) || (pr1 == NULL) )
        return  -1;

    if ( pr0->right < pr1->left )
        return  -1;
    if ( pr1->right < pr0->left )
        return  -1;

    if ( pr0->bottom < pr1->top )
        return  -1;
    if ( pr1->bottom < pr0->top )
        return  -1;

    return  1;
}

/* Check if the rectangle has content */
int  in_is_zero_rect(const GUI_RECT *pr) 
{
    if ( pr == NULL )
        return  -1;

    if (pr->left > pr->right)
        return 1; 

    if (pr->top > pr->bottom)
        return 1;
    
    return  -1;
}

/* Check if the rectangle has content */
int  in_is_none_zero_rect(const GUI_RECT *pr) 
{
    if ( pr == NULL )
        return  -1;

    if (pr->left > pr->right)
        return -1;
 
    if (pr->top > pr->bottom)
        return -1;
    
    return  1;
}
