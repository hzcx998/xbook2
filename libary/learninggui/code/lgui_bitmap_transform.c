/*   
 *  Copyright (C) 2011- 2020 Rao Youkun(960747373@qq.com)
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
#include  <stdlib.h>
#include  <math.h>
#include  <string.h>


#include  <lgmacro.h>
#include  <type_gui.h>

#include  <lock.h>
#include  <cursor.h>

#include  <d2_pixel.h>
#include  <screen.h>

#include  <image_comm.h>
#include  <image_bitmap.h>
#include  <lgui_bitmap_rotate.h>
#include  <lgui_bitmap_transform.h>


#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif




int  in_make_blank_grid(unsigned int width, unsigned int height, void *transform)
{
    const  int ja[] = {1, 2, 3, 0};

    GUI_TRANSFORM   *transform_s = (GUI_TRANSFORM *)transform;
    GUI_DOUBLE_POINT        corner[4] = { 0 };

    DOUBLE  sideradius[4] = { 0 };
    DOUBLE  sidecos[4]    = { 0 };
    DOUBLE  sidesin[4]    = { 0 };

    int     i = 0;
    int     j = 0;
    int     x = 0;
    int     y = 0;
    
    int     index = 0;



    for ( i = 0; i < 4; i++)
        corner[i] = (transform_s->corner[i]);


    /* First we find the radius, cos, and sin of each side of the polygon created by xcorner and ycorner */
    for (i = 0; i < 4; i++)
    {
        j = ja[i];
        sideradius[i] = sqrt((corner[i].x - corner[j].x) * (corner[i].x - corner[j].x) + (corner[i].y - corner[j].y) * (corner[i].y - corner[j].y));
        sidecos[i] = (corner[j].x - corner[i].x) / sideradius[i];
        sidesin[i] = (corner[j].y - corner[i].y) / sideradius[i];
    }


    /* Next we create two lines in Ax + By = C form */
    for (y = 0; y < (height + 1); y++)
    {
        DOUBLE leftdist = (1.0 - ((DOUBLE)y)/height) * sideradius[3];
        DOUBLE ptxleft  = corner[3].x + leftdist * sidecos[3];
        DOUBLE ptyleft  = corner[3].y + leftdist * sidesin[3];
    
        DOUBLE rightdist = (((DOUBLE)y) / height) * sideradius[1];
        DOUBLE ptxright  = corner[1].x + rightdist * sidecos[1];
        DOUBLE ptyright  = corner[1].y + rightdist * sidesin[1];

        DOUBLE Av = ptyright - ptyleft;
        DOUBLE Bv = ptxleft - ptxright;
        DOUBLE Cv = Av * ptxleft + Bv * ptyleft;
 

        for (x = 0; x < (width + 1); x++)
        {
            DOUBLE topdist = (((DOUBLE)x) / width) * sideradius[0];
            DOUBLE ptxtop  = corner[0].x + topdist * sidecos[0];
            DOUBLE ptytop  = corner[0].y + topdist * sidesin[0];

            DOUBLE botdist = (1.0 - ((DOUBLE)x)/width) * sideradius[2];
            DOUBLE ptxbot  = corner[2].x + botdist * sidecos[2];
            DOUBLE ptybot  = corner[2].y + botdist * sidesin[2];

            DOUBLE Ah = ptybot - ptytop;
            DOUBLE Bh = ptxtop - ptxbot;
            DOUBLE Ch = Ah * ptxtop + Bh * ptytop;


            /* Find where the lines intersect and store that point in the pixelgrid array */
            DOUBLE det = Ah * Bv - Av * Bh;
            if (GUI_ABS(det) < 1e-9)
            {
                return  0;
            }  else {
                index  =  x + y * (width + 1);
                transform_s->raw_buffer[index].x = (Bv * Ch - Bh * Cv) / det;
                transform_s->raw_buffer[index].y = (Ah * Cv - Av * Ch) / det;
            }
        }
    }

    return  1;
}


#ifndef  _LG_ALONE_VERSION_
int  make_blank_grid(unsigned int width, unsigned int height, void *transform)
{
    int  ret = 0;

    gui_lock( );
    ret = in_make_blank_grid(width, height, transform);
    gui_unlock( );

    return  ret;
}
#endif



DOUBLE  in_transform_caliculate_overlap(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT  *corner)
{
    GUI_DOUBLE_POINT  lap_point[16]  = {0};
    GUI_DOUBLE_POINT  sort_point[16] = {0};

    unsigned int  lap_size  = 16;
    unsigned int  sort_size = 0;
    unsigned int  in_size   = 0;

    DOUBLE  ret = 0.0;

    int ja[] = {1, 2, 3, 0};
    DOUBLE minx, maxx, miny, maxy;

    DOUBLE z = 0;

    int  i = 0;
    int  j = 0;



    if ( p == NULL )
        return  -1;


    in_size = 0;
    sort_size = 0;


    for (i = 0; i < 4; i++)
    {
        /* Search for source points within the destination square */
        if (p[i].x >= 0 && p[i].x <= 1 && p[i].y >= 0 && p[i].y <= 1)
	{
            lap_point[in_size++] = p[i];
	}

        /* Search for destination points within the source square */
        if (in_is_point_inside_convex_polygon(&corner[i], p))
	{
            lap_point[in_size++] = corner[i];
	}

        /* Search for line intersections */
        j = ja[i];
        minx = GUI_MIN(p[i].x, p[j].x);
        miny = GUI_MIN(p[i].y, p[j].y);
        maxx = GUI_MAX(p[i].x, p[j].x);
        maxy = GUI_MAX(p[i].y, p[j].y);


        /* Cross left */
        if (minx < 0.0 && 0.0 < maxx)   
        {
            z = p[i].y - p[i].x * (p[i].y - p[j].y) / (p[i].x - p[j].x);
            if (z >= 0.0 && z <= 1.0)
            {
                lap_point[in_size].x = 0.0;
                lap_point[in_size++].y = z;
            }
        }
        /* Cross right */
        else if (minx < 1.0 && 1.0 < maxx)
        {
            z = p[i].y + (1 - p[i].x) * (p[i].y - p[j].y) / (p[i].x - p[j].x);
            if (z >= 0.0 && z <= 1.0)
            {
                lap_point[in_size].x = 1.0;
                lap_point[in_size++].y = z;
            }
        }


        /* Cross bottom */
        if (miny < 0.0 && 0.0 < maxy)
        {
            z = p[i].x - p[i].y * (p[i].x - p[j].x) / (p[i].y - p[j].y);
            if (z >= 0.0 && z <= 1.0)
            {
                lap_point[in_size].x = z;
                lap_point[in_size++].y = 0.0;
            }
        }
        /* Cross top */
        else if (miny < 1.0 && 1.0 < maxy)
        {
            z = p[i].x + (1 - p[i].y) * (p[i].x - p[j].x) / (p[i].y - p[j].y);
            if (z >= 0.0 && z <= 1.0)
            {
                lap_point[in_size].x = z;
                lap_point[in_size++].y = 1.0;
            }
        }
    }        

    lap_size = in_size;


    /* Sort the points and return the area */
    in_sort_point(lap_point, lap_size, sort_point, &sort_size);

    ret = in_caliculate_area(sort_point, sort_size);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
DOUBLE  transform_caliculate_overlap(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT  *corner)
{
    DOUBLE  ret = 0.0;

    gui_lock( );
    ret = in_transform_caliculate_overlap(p, corner);
    gui_unlock( );

    return  ret;
}
#endif



int  in_is_point_inside_convex_polygon(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT *corner)
{
    int ja[] = {1, 2, 3, 0};

    int dir = 0;
    int i   = 0;
    int j   = 0;

    DOUBLE  cross = 0.0;



    for (i = 0; i < 4; i++)
    {
        j = ja[i];

        cross = (corner[i].x - p->x) * (corner[j].y - p->y) - (corner[j].x - p->x) * (corner[i].y - p->y);

        if (cross == 0)
            continue;

        if (cross > 0)
        {
            if (dir == -1)
                return  0;

            dir = 1;
        } else {
            if (dir == 1)
                return  0;

            dir = -1;
        }
    }

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  is_point_inside_convex_polygon(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT *corner)
{
    int  ret = 0;

    gui_lock( );
    ret = in_is_point_inside_convex_polygon(p, corner);
    gui_unlock( );

    return  ret;
}
#endif



#ifdef  _LG_BITMAP_

int  in_gui_bitmap_transform_area(HDC hdc, const void *bitmap, void *transform)
{
    const GUI_BITMAP  *pbitmap     = (GUI_BITMAP *)bitmap;
    GUI_TRANSFORM     *ptransform  = (GUI_TRANSFORM *)transform;
    
    GUI_DOUBLE_POINT  in_corners[4] = {{0.0, 0.0}, {1.0, 0.0}, {1.0,1.0},{0.0,1.0}};

    

    GUI_DOUBLE_POINT  p[4] = {{0, 0}};
    GUI_DOUBLE_POINT  poffset[4] = {{0, 0}};

    GUI_DOUBLE_POINT  plt     = {0, 0};
    GUI_DOUBLE_POINT  plb     = {0, 0};
    GUI_DOUBLE_POINT  prt     = {0, 0};
    GUI_DOUBLE_POINT  prb     = {0, 0};

    GUI_RECT          rect    = {0, 0, 0, 0};
    GUI_DOUBLE_RECT   frect   = {0, 0, 0, 0};

    DOUBLE      fvalue1 = 0;
    DOUBLE      fvalue2 = 0;


    int  new_width  = 0;
    int  new_height = 0;

    int  i  = 0;
    int  j  = 0;

    int  x = 0;
    int  y = 0;


    /* Find the scan area on the destination's pixels */
    int min_dx  = 0;
    int max_dx  = 0;
    int min_dy  = 0;
    int max_dy  = 0;
            

    /* loop through the scan area to find where source(x, y) overlaps with the destination pixels */
    int  sub_x  = 0;
    int  sub_y  = 0;


    int  offset = 0;
    int  index  = 0;


    GUI_COLOR         gui_color    =  GUI_WHITE;
    SCREEN_COLOR      screen_color;
    unsigned char     red_color    = 0;
    unsigned char     green_color  = 0;
    unsigned char     blue_color   = 0;

    char   transparent_flag = 0;
    unsigned char  bit = 0;

    DOUBLE  overlap = 0.0; 
    DOUBLE  alpha   = 0.0;

    int    ret = 0;



    if ( ptransform ==  NULL )
        return  -1;


    /* Get offset */
    ptransform->offset.x = ptransform->corner[0].x;
    ptransform->offset.y = ptransform->corner[0].y;
    for ( i = 0; i < 4; i++ )
    {
        if ( (ptransform->corner[i].x) < (ptransform->offset.x))
            ptransform->offset.x = ptransform->corner[i].x;

	if ( (ptransform->corner[i].y) < (ptransform->offset.y))
            ptransform->offset.y = ptransform->corner[i].y;
    }
    for ( i = 0; i < 4; i++ )
    {
        ptransform->corner[i].x  -=  ptransform->offset.x;
        ptransform->corner[i].y  -=  ptransform->offset.y;
    }



    /* LeftTop point */
    plt.x     =  ptransform->corner[0].x;
    plt.y     =  ptransform->corner[0].y;


    /* LeftBottom point */
    plb.x     =  ptransform->corner[3].x;
    plb.y     =  ptransform->corner[3].y;


    /* RightBottom point */
    prb.x     =  ptransform->corner[2].x;
    prb.y     =  ptransform->corner[2].y;


    /* RightTop point */
    prt.x     =  ptransform->corner[1].x;
    prt.y     =  ptransform->corner[1].y;


    /* Calculate rect */
    fvalue1 = GUI_MIN(plt.x, plb.x);
    fvalue2 = GUI_MIN(prt.x, prb.x);
    frect.left = GUI_MIN(fvalue1,fvalue2);

    fvalue1 = GUI_MIN(plt.y, plb.y);
    fvalue2 = GUI_MIN(prt.y, prb.y);
    frect.top = GUI_MIN(fvalue1,fvalue2);

    fvalue1 = GUI_MAX(plt.x, plb.x);
    fvalue2 = GUI_MAX(prt.x, prb.x);
    frect.right = GUI_MAX(fvalue1,fvalue2);

    fvalue1 = GUI_MAX(plt.y, plb.y);
    fvalue2 = GUI_MAX(prt.y, prb.y);
    frect.bottom = GUI_MAX(fvalue1,fvalue2);

    /* Adjust frect */
    if ( (frect.left) > (frect.right) )
    {
        fvalue1      = frect.left;
	frect.left   = frect.right;
	frect.right  = fvalue1;
    }

    if ( (frect.top) > (frect.bottom) )
    {
        fvalue1      = frect.top;
	frect.top    = frect.bottom;
	frect.bottom = fvalue1;
    }


    /* ?? */
    /*
    rect.left   = (int)(frect.left - 1);
    rect.top    = (int)(frect.top - 1);
    */
    rect.left   = (int)(frect.left);
    rect.top    = (int)(frect.top);

    rect.right  = in_gui_round_up(frect.right);
    rect.bottom = in_gui_round_up(frect.bottom);


    new_width  = GUI_RECTW(&rect) - 1;
    new_height = GUI_RECTH(&rect) - 1;



    /* Init */
    if ( (ptransform->raw_size) < (((pbitmap->width)+1)*((pbitmap->height)+1)*sizeof(GUI_DOUBLE_POINT)) )
        return  -1;
    memset(ptransform->raw_buffer, 0, ptransform->raw_size);


    if ( (ptransform->transform_size) < (new_width*new_height*sizeof(GUI_DOUBLE_RGB)) )
        return  -1;
    memset(ptransform->transform_buffer, 0, ptransform->transform_size);


    ret = in_make_blank_grid(pbitmap->width, pbitmap->height, ptransform);
    if ( ret < 1 )
        return  -1;


    /* For app */
    hdc->paint_rect         = rect;
    hdc->paint_rect.left   += ptransform->offset.x;
    hdc->paint_rect.top    += ptransform->offset.y;
    hdc->paint_rect.right  += ptransform->offset.x;
    hdc->paint_rect.bottom += ptransform->offset.y;

    hdc->is_paint_rect = 1;



    /* Loop through the source's pixels */
    for (y = 0; y < (pbitmap->height); y++)
    {
        for (x = 0; x < (pbitmap->width); x++)
        {
            /* Construct the source pixel's rotated polygon from pixelgrid */
            p[0] = ptransform->raw_buffer[x + y * ((pbitmap->width) + 1)];
            p[1] = ptransform->raw_buffer[(x + 1) + y * ((pbitmap->width) + 1)];
            p[2] = ptransform->raw_buffer[(x + 1) + (y + 1) * ((pbitmap->width) + 1)];
            p[3] = ptransform->raw_buffer[x + (y + 1) * ((pbitmap->width) + 1)];
           
            /* Find the scan area on the destination's pixels */
            min_dx = 0x7FFFFFFF;
            min_dy = 0x7FFFFFFF;
            max_dx = 0x80000000;
            max_dy = 0x80000000;

            for (i = 0; i < 4; i++)
            {
		 /* ?? */
	        if (in_gui_round(p[i].x) < min_dx) 
		    min_dx = in_gui_round(p[i].x);

                if (in_gui_round_up(p[i].x) > max_dx) 
	            max_dx = in_gui_round_up(p[i].x);
		
		/* ?? */
		if (in_gui_round(p[i].y) < min_dy) 
	            min_dy = in_gui_round(p[i].y);

	        if (in_gui_round_up(p[i].y) > max_dy) 
	            max_dy = in_gui_round_up(p[i].y);
            }

            for (sub_y = min_dy - 1; sub_y <= max_dy; sub_y++)
            {  
                if ( (sub_y < 0) || ( sub_y >= new_height) )
	            continue ;


                for (sub_x = (min_dx - 1); sub_x <= max_dx; sub_x++)
                {
	            if ( (sub_x < 0) || ( sub_x >= new_width) )
		        continue ;

                    for (i = 0; i < 4; i++)
                    {
                        poffset[i].x = p[i].x - sub_x;
                        poffset[i].y = p[i].y - sub_y;
                    }

	   
                    overlap =  in_transform_caliculate_overlap(poffset, in_corners);
                    if (!overlap)
		        continue;


		    /* ?? */
                    index = (sub_x - rect.left) + (sub_y - rect.top) * new_width;
		    if ( index < 0 )
			continue;


		    if ((pbitmap->bits_per_pixel) == 1)
		    {
                        offset = ((x)/8) + ((pbitmap->height) - (y) - 1) * (pbitmap->bytes_per_line);
                        bit    = ((x)%8);
		    } else if ((pbitmap->bits_per_pixel) == 2) {
                        offset = ((x)/4) + ((pbitmap->height) - (y) - 1) * (pbitmap->bytes_per_line);
                        bit    = ((x)%4);
		    } else if ((pbitmap->bits_per_pixel) == 4) {
                        offset = ((x)/2) + ((pbitmap->height) - (y) - 1) * (pbitmap->bytes_per_line);
                        bit    = ((x)%2);
		    } else {
                        offset = (x) + ((pbitmap->height) - (y) - 1) * (pbitmap->width);

                        offset = x + ((pbitmap->height) - y - 1 ) * (pbitmap->width);

		    }

                    ret = in_gui_bitmap_get_color(pbitmap,offset,bit,&transparent_flag,&gui_color);
                    if ( ret < 0 )
		        continue;

		    if ( transparent_flag > 0 )
		    {
	                ret = in_point_input_pixel_abs(hdc,sub_x+hdc->rect.left,sub_y+ hdc->rect.top, &gui_color); 
		        if ( ret < 0 )
		            continue;
		    }

                    blue_color  = (gui_color&0xFF);
                    green_color = ((gui_color&0xFF00) >> 8);
                    red_color   = ((gui_color&0xFF0000) >> 16);

                    ptransform->transform_buffer[index].red   += (red_color * overlap);
                    ptransform->transform_buffer[index].green += (green_color * overlap);
                    ptransform->transform_buffer[index].blue  += (blue_color * overlap);
                    ptransform->transform_buffer[index].alpha += overlap;
                }
            }
        }
    }


    for (j = 0; j < new_height; j++)
    {
        for (i = 0; i < new_width; i++)
        {
            index = i + j*new_width;
	 
            alpha =  ptransform->transform_buffer[index].alpha;
            if ( alpha == 0 )
                continue;

	    if ( alpha < 0.95)
	    {
                ret = in_point_input_pixel_abs(hdc,i+rect.left+hdc->rect.left,j+rect.top+hdc->rect.top, &gui_color); 
		if ( ret < 0 )
		    continue;

                blue_color  = in_gui_byte((ptransform->transform_buffer[index].blue+(1-alpha)*(gui_color&0xFF)) );
                green_color = in_gui_byte((ptransform->transform_buffer[index].green + (1-alpha)*((gui_color&0xFF00)>>8)));
                red_color   = in_gui_byte((ptransform->transform_buffer[index].red + (1-alpha)*((gui_color&0xFF0000)>>16)));
	    } else {
                blue_color  = in_gui_byte(ptransform->transform_buffer[index].blue);
                green_color = in_gui_byte(ptransform->transform_buffer[index].green);
                red_color   = in_gui_byte(ptransform->transform_buffer[index].red);
	    }
	   
            gui_color = (red_color << 16) | (green_color << 8) | blue_color;
            screen_color = (lscrn.gui_to_screen_color)(gui_color);

            in_output_screen_pixel_abs(hdc, i+rect.left+hdc->rect.left + ptransform->offset.x, j+rect.top+hdc->rect.top + ptransform->offset.y, screen_color);
	}
    }

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  gui_bitmap_transform_area(HDC hdc, const void *bitmap, void *transform)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_bitmap_transform_area(hdc, bitmap, transform);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif    /* _LG_BITMAP_ */

