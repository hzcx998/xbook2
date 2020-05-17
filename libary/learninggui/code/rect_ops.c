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


/* Divide rect function */
int  in_gui_divide_rect(GUI_RECT *in_r, unsigned int power, GUI_RECT *out_r, unsigned int out_num, unsigned int *ret_num)
{
    #define   DELTA_SPACE      5
    #define   DELTA_OFFSET     1


    GUI_RECT      temp_rect = {0};
    unsigned int  value     = 0;
    unsigned int  ret_num1  = 0;
    unsigned int  ret_num2  = 0;
             int  mid_left  = 0;
	     int  mid_top   = 0;
             int  i         = 0;


    if ( in_r == NULL )
        return  -1;
    if ( out_r == NULL )
        return  -1;
    if ( ret_num == NULL )
        return  -1;

    if ( power < 1 )
        return  -1;
    if ( out_num < 1 )
        return  -1;


    /* Horizonal line */
    if ( (in_r->top) == (in_r->bottom) )
    {
        *out_r   = *in_r;
	*ret_num = 1;

        return  1;
    }

    /* Vertical line */
    if ( (in_r->left) == (in_r->right) )
    {
        *out_r   = *in_r;
	*ret_num = 1;

        return  1;
    }


    /* Horizonal adjust  */
    if ( GUI_ABS((in_r->top) - (in_r->bottom)) < DELTA_SPACE ) 
    {
        *out_r   = *in_r;
	*ret_num = 1;

        return  1;
    }


    /* Vertical adjust  */
    if ( GUI_ABS((in_r->left) - (in_r->right)) < DELTA_SPACE ) 
    {
        *out_r   = *in_r;
	*ret_num = 1;

        return  1;
    }



    /* Need to be divided */
    value = 1; 
    for ( i = 0; i < power; i++ )
        value *= 2;
    if ( out_num < value )
        return  -1;


    mid_left = (((in_r->left) + (in_r->right))/2);
    mid_top  = (((in_r->top) + (in_r->bottom))/2);

    if ( power == 1 )
    {
        temp_rect = *in_r;
        temp_rect.right  = mid_left;
        temp_rect.bottom = mid_top;
	/* Adjust .right */
        if ( (temp_rect.left) < (temp_rect.right) )
	{
	    temp_rect.right += DELTA_OFFSET;
	} else if ( (temp_rect.left) > (temp_rect.right) ) {
	    temp_rect.right -= DELTA_OFFSET;
	}
	/* Adjust .bottom */
        if ( (temp_rect.top) < (temp_rect.bottom) )
	{
	    temp_rect.bottom += DELTA_OFFSET;
	} else if ( (temp_rect.top) > (temp_rect.bottom) ) {
	    temp_rect.bottom -= DELTA_OFFSET;
	}
        *out_r = temp_rect;


        temp_rect = *in_r;
        temp_rect.left = mid_left;
        temp_rect.top  = mid_top; 
	/* Adjust .left */
        if ( (temp_rect.left) < (temp_rect.right) )
	{
	    temp_rect.left -= DELTA_OFFSET;
	} else if ( (temp_rect.left) > (temp_rect.right) ) {
	    temp_rect.left += DELTA_OFFSET;
	}
	/* Adjust .top */
        if ( (temp_rect.top) < (temp_rect.bottom) )
	{
	    temp_rect.top -= DELTA_OFFSET;
	} else if ( (temp_rect.top) > (temp_rect.bottom) ) {
	    temp_rect.top += DELTA_OFFSET;
	}
        *(out_r+1) = temp_rect;


	*ret_num = 2;


        return  1;	
    } 

    temp_rect = *in_r;
    temp_rect.right  = mid_left;
    temp_rect.bottom = mid_top;
    /* Adjust .right */
    if ( (temp_rect.left) < (temp_rect.right) )
    {
        temp_rect.right += DELTA_OFFSET;
    } else if ( (temp_rect.left) > (temp_rect.right) ) {
	temp_rect.right -= DELTA_OFFSET;
    }
    /* Adjust .bottom */
    if ( (temp_rect.top) < (temp_rect.bottom) )
    {
	temp_rect.bottom += DELTA_OFFSET;
    } else if ( (temp_rect.top) > (temp_rect.bottom) ) {
	temp_rect.bottom -= DELTA_OFFSET;
    }
    in_gui_divide_rect(&temp_rect, power-1, out_r, out_num, &ret_num1);


    temp_rect = *in_r;
    temp_rect.left = mid_left;
    temp_rect.top  = mid_top;
    /* Adjust .left */
    if ( (temp_rect.left) < (temp_rect.right) )
    {
	temp_rect.left -= DELTA_OFFSET;
    } else if ( (temp_rect.left) > (temp_rect.right) ) {
	temp_rect.left += DELTA_OFFSET;
    }
    /* Adjust .top */
    if ( (temp_rect.top) < (temp_rect.bottom) )
    {
	temp_rect.top -= DELTA_OFFSET;
    } else if ( (temp_rect.top) > (temp_rect.bottom) ) {
	temp_rect.top += DELTA_OFFSET;
    }

    if ( ret_num1 > out_num )
        ret_num1 = out_num;
    in_gui_divide_rect(&temp_rect, power-1, out_r + ret_num1, out_num-ret_num1, &ret_num2);

    *ret_num = (ret_num1 + ret_num2);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  gui_divide_rect(GUI_RECT *in_r, unsigned int power, GUI_RECT *out_r, unsigned int out_num, unsigned int *ret_num)
{
    int  ret = 0;

    gui_lock( );
    ret =  in_gui_divide_rect(in_r, power, out_r, out_num, ret_num);
    gui_unlock( );

    return  ret;
}
#endif
