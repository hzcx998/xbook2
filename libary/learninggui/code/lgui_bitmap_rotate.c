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

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif



DOUBLE  in_caliculate_overlap(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT  *center, DOUBLE *fsin, DOUBLE *fcos)
{
    GUI_DOUBLE_POINT  corner[4] = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};

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
        if (in_is_corner_inside_square((corner + i), center, fsin, fcos))
	{
            lap_point[in_size++] = corner[i];
	}

        /* Search for line intersections */
        j = ja[i];
        minx = GUI_MIN(p[i].x, p[j].x);
        miny = GUI_MIN(p[i].y, p[j].y);
        maxx = GUI_MAX(p[i].x, p[j].x);
        maxy = GUI_MAX(p[i].y, p[j].y);


        if (minx < 0.0 && 0.0 < maxx)
        {/* Cross left */
            z = p[i].y - p[i].x * (p[i].y - p[j].y) / (p[i].x - p[j].x);
            if (z >= 0.0 && z <= 1.0)
            {
                lap_point[in_size].x = 0.0;
                lap_point[in_size++].y = z;
            }
        }
        else if (minx < 1.0 && 1.0 < maxx)
        {/* Cross right */
            z = p[i].y + (1 - p[i].x) * (p[i].y - p[j].y) / (p[i].x - p[j].x);
            if (z >= 0.0 && z <= 1.0)
            {
                lap_point[in_size].x = 1.0;
                lap_point[in_size++].y = z;
            }
        }

        if (miny < 0.0 && 0.0 < maxy)
        {/* Cross bottom */
            z = p[i].x - p[i].y * (p[i].x - p[j].x) / (p[i].y - p[j].y);
            if (z >= 0.0 && z <= 1.0)
            {
                lap_point[in_size].x = z;
                lap_point[in_size++].y = 0.0;
            }
        }
        else if (miny < 1.0 && 1.0 < maxy)
        {/* Cross top */
            z = p[i].x + (1 - p[i].y) * (p[i].x - p[j].x) / (p[i].y - p[j].y);
            if (z >= 0.0 && z <= 1.0)
            {
                lap_point[in_size].x = z;
                lap_point[in_size++].y = 1.0;
            }
        }
    }        

    /* Rao */
    lap_size = in_size;


    /* Sort the points and return the area */
    in_sort_point(lap_point, lap_size, sort_point, &sort_size);

    ret =  in_caliculate_area(sort_point, sort_size);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
DOUBLE  caliculate_overlap(GUI_DOUBLE_POINT *p, GUI_DOUBLE_POINT *center,DOUBLE *fsin,DOUBLE *fcos)
{
    DOUBLE  ret = 0.0;

    gui_lock( );
    ret = in_caliculate_overlap(p, center, fsin, fcos);
    gui_unlock( );

    return  ret;
}
#endif

int  in_is_corner_inside_square(GUI_DOUBLE_POINT *corner, GUI_DOUBLE_POINT *square, DOUBLE *fsin, DOUBLE *fcos)
{
    GUI_DOUBLE_POINT   tmp_corner;
    GUI_DOUBLE_POINT   tmp_point;


    if ( corner == NULL )
        return  -1;
    if ( square == NULL )
        return  -1;


    tmp_corner = *corner;


    /* Offset point */
    tmp_corner.x -= square->x;
    tmp_corner.y -= square->y;

    /* rotate point */
    tmp_point.x = (tmp_corner.x) * (*fcos) + tmp_corner.y * (*fsin);
    tmp_point.y = (tmp_corner.y) * (*fcos) - tmp_corner.x * (*fsin);

    /* Find if the rotated polygon is within the square of size 1 centerd on the origin */
    tmp_point.x = GUI_ABS(tmp_point.x);
    tmp_point.y = GUI_ABS(tmp_point.y);

    return    ( ((tmp_point.x) < 0.5) && ((tmp_point.y) < 0.5) );
}

#ifndef  _LG_ALONE_VERSION_
int  is_corner_inside_square(GUI_DOUBLE_POINT *corner, GUI_DOUBLE_POINT *square, DOUBLE *fsin, DOUBLE *fcos)
{
    int  ret = 0;

    gui_lock( );
    ret = in_is_corner_inside_square(corner, square, fsin, fcos);
    gui_unlock( );

    return  ret;
}
#endif


int  in_sort_point(GUI_DOUBLE_POINT *in_point, unsigned int in_size, GUI_DOUBLE_POINT *out_point, unsigned int *out_size)
{
    #define   INDEX_LIST_SIZE    16


    INDEX_LIST  *root     = NULL;
    INDEX_LIST  *node     = NULL;
    INDEX_LIST  *tmp_node = NULL;
    unsigned int  size    = 0;
    int          i    = 0;
   

    INDEX_LIST   list[INDEX_LIST_SIZE] = { 0 };
    unsigned int counter = 0;


    if ( in_point == NULL )
        return  -1;
   if ( out_point == NULL )
	return  -1;
    if ( out_size == NULL )
	return  -1;



    if ( in_size < 3 )
	return  1;
 
    *out_size = 0;


    if ( size == 3 )
    {
        *out_size = 2;
        out_point[0].x = in_point[1].x - in_point[0].x;
        out_point[0].y = in_point[1].y - in_point[0].y;

        out_point[1].x = in_point[2].x - in_point[0].x;
        out_point[1].y = in_point[2].y - in_point[0].y;

        return  *out_size;
    }


    /* Add */
    memset(&list, 0, sizeof(list));
    counter = 0;


    list[0].used = 1;
    root = &list[0];
    root->index = 0;
    root->next = NULL;
    counter++;


    for (i = 1; i < in_size; i++)
    {
        in_point[i].x = in_point[i].x - in_point[0].x;
        in_point[i].y = in_point[i].y - in_point[0].y;

        node = root;
        while ( 1 )
        {
            if (node->next)
            {
                if (in_point[i].x * in_point[node->next->index].y - in_point[node->next->index].x * in_point[i].y < 0)
                {
                    tmp_node = node->next;

		    list[counter].used = 1;
                    node->next = &list[counter];
                    node->next->index = i;
                    node->next->next = tmp_node;
 
		    counter++;

                    break;
                }
            } else {
		list[counter].used = 1;
                node->next = &list[counter];
                node->next->index = i;
                node->next->next = NULL;

		counter++;

                break;
            }
            node = node->next;
        }
    }


    /* We can leave out the first point because it's offset position is going to be (0, 0) */
    size = 0;

    /* ?? */
    node = root;
    while ( node )
    {
        tmp_node = node;
        node = node->next;
        if (node)
	{
            out_point[size++] = in_point[node->index];
	}
    }

    *out_size = size;

    return  size;
}


#ifndef  _LG_ALONE_VERSION_
int  sort_point(GUI_DOUBLE_POINT *in_point, unsigned int in_size, GUI_DOUBLE_POINT *out_point, unsigned int *out_size)
{
    int  ret = 0;

    gui_lock( );	
    ret = in_sort_point(in_point, in_size, out_point, out_size);
    gui_unlock( );

    return  ret;
}
#endif


    
DOUBLE  in_caliculate_area(GUI_DOUBLE_POINT *point, unsigned  int size)
{
    DOUBLE  value = 0.0;
    int     i     = 0;


    if ( point == NULL )
        return  0.0;


    if ( size < 2 )
	  return  0.0;


    /* Loop through each triangle with respect to (0, 0) and add the cross multiplication */
    value = 0.0;
    for (i = 0; i < (size - 1); i++)
        value += GUI_ABS(point[i].x * point[i + 1].y - point[i + 1].x * point[i].y);

    return  ((GUI_ABS(value))/2.0);
}


#ifndef  _LG_ALONE_VERSION_
DOUBLE  caliculate_area(GUI_DOUBLE_POINT *point, unsigned int size)
{
    DOUBLE  ret = 0.0;

    gui_lock( );
    ret = in_caliculate_area(point, size);
    gui_unlock( );

    return  ret;
}
#endif


#ifdef  _LG_BITMAP_

int  in_gui_bitmap_rotate_area(HDC hdc, int x, int y, const void *bitmap, void *rotate)
{
    const GUI_BITMAP  *pbitmap = (GUI_BITMAP *)bitmap;
    GUI_ROTATE  *rotate_s      = (GUI_ROTATE *)rotate;
    
    GUI_DOUBLE_POINT  p[4] = {{0, 0}};
    GUI_DOUBLE_POINT  poffset[4] = {{0, 0}};
    GUI_DOUBLE_POINT  c = {0, 0};
    GUI_DOUBLE_POINT  new_c = {0, 0};

    GUI_DOUBLE_POINT  plt     = {0, 0};
    GUI_DOUBLE_POINT  plb     = {0, 0};
    GUI_DOUBLE_POINT  prt     = {0, 0};
    GUI_DOUBLE_POINT  prb     = {0, 0};

    GUI_RECT          rect    = {0, 0, 0, 0};

    GUI_DOUBLE_RECT   frect    = {0, 0, 0, 0};

    DOUBLE      fsin    = 0;
    DOUBLE      fcos    = 0;
    DOUBLE      fvalue1 = 0;
    DOUBLE      fvalue2 = 0;

    int  theta   = 0;

    int  raw_width  = 0;
    int  raw_height = 0;
    int  new_width  = 0;
    int  new_height = 0;

    int  i  = 0;
    int  j  = 0;

    int  x0 = 0;
    int  y0 = 0;


    /* Find the scan area on the destination's pixels */
    int min_dx = 0;
    int max_dx = 0;
    int min_dy = 0;
    int max_dy = 0;
            

    /* loop through the scan area to find where source(x, y) overlaps with the destination pixels */    int  sub_x = 0;
    int  sub_y = 0;


    int  offset = 0;
    int  index  = 0;


    GUI_COLOR         gui_color =  GUI_WHITE;
    SCREEN_COLOR      screen_color;
    unsigned char     red_color    = 0;
    unsigned char     green_color  = 0;
    unsigned char     blue_color   = 0;

    char   transparent_flag = 0;
    unsigned char  bit = 0;

    DOUBLE  overlap = 0.0; 
    DOUBLE  alpha   = 0.0;

    int    ret = 0;




    if ( rotate_s ==  NULL )
        return  -1;

    if ( IMAGE_ROTATE_FACTOR == 0 )
        return  -1;


    x0    = ((GUI_ROTATE *)rotate)->x;
    y0    = ((GUI_ROTATE *)rotate)->y;
    theta = ((GUI_ROTATE *)rotate)->theta;

    raw_width  = pbitmap->width;
    raw_height = pbitmap->height;

    fsin = sin((theta*PI)/IMAGE_ROTATE_180);
    fcos = cos((theta*PI)/IMAGE_ROTATE_180);

    /* LeftTop point */
    fvalue1 =  (x-x0)*fcos;
    fvalue2 =  0 - ((y-y0)*fsin);
    plt.x   =  fvalue1 + fvalue2 + x0;
    fvalue1 =  (x-x0)*fsin;
    fvalue2 =  (y-y0)*fcos;
    plt.y   =  fvalue1 + fvalue2 + y0;

    /* LeftBottom point */
    fvalue1 =  (x-x0)*fcos;
    fvalue2 =  0 - ((y+raw_height-1-y0)*fsin);
    plb.x   =  fvalue1 + fvalue2 + x0;

    fvalue1 =  (x-x0)*fsin;
    fvalue2 =  (y+raw_height-1-y0)*fcos;
    plb.y   =  fvalue1 + fvalue2 + y0;

    /* RightBottom point */
    fvalue1 =  (x+raw_width-1-x0)*fcos;
    fvalue2 =  0 - ((y+raw_height-1-y0)*fsin);
    prb.x   =  fvalue1 + fvalue2 + x0;
    fvalue1 =  (x+raw_width-1-x0)*fsin;
    fvalue2 =  (y+raw_height-1-y0)*fcos;
    prb.y   =   fvalue1 + fvalue2 + y0;

    /* RightTop point */
    fvalue1 =  (x+raw_width-1-x0)*fcos;
    fvalue2 =  0 - ((y-y0)*fsin); 
    prt.x   =  fvalue1 + fvalue2 + x0;
    fvalue1 =  (x+raw_width-1-x0)*fsin;
    fvalue2 =  (y-y0)*fcos;
    prt.y   =  fvalue1 + fvalue2 + y0;

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

    rect.left   = (int)(frect.left - 1);
    rect.top    = (int)(frect.top - 1);
    rect.right  = in_gui_round_up(frect.right);
    rect.bottom = in_gui_round_up(frect.bottom);

    /* New width and height */
    new_width  = GUI_RECTW(&rect);
    new_height = GUI_RECTH(&rect);

    /* For app */
    hdc->paint_rect = rect;
    hdc->is_paint_rect = 1;

    /* Init */
    if ( (rotate_s->buf_size) < (new_width*new_height*sizeof(GUI_DOUBLE_RGB)) )
        return  -1;
    memset(rotate_s->buffer, 0, rotate_s->buf_size);


    /* Loop through the source's pixels */
    for (y0 = y; y0 < ((pbitmap->height) + y); y0++)
    {
        for (x0 = x; x0 < ((pbitmap->width) + x); x0++)
        {
            fvalue1 =  (x0-rotate_s->x)*fcos;
            fvalue2 =  0 - ((y0-rotate_s->y)*fsin);
            p[0].x   =  fvalue1 + fvalue2 + rotate_s->x;
            fvalue1 =  (x0-rotate_s->x)*fsin;
            fvalue2 =  (y0-rotate_s->y)*fcos;
            p[0].y   =  fvalue1 + fvalue2 + rotate_s->y;

            fvalue1 =  (x0+1.0-rotate_s->x)*fcos;
            fvalue2 =  0 - ((y0-rotate_s->y)*fsin);
            p[1].x   =  fvalue1 + fvalue2 + rotate_s->x;
            fvalue1 =  (x0+1.0-rotate_s->x)*fsin;
            fvalue2 =  (y0-rotate_s->y)*fcos;
            p[1].y   =  fvalue1 + fvalue2 + rotate_s->y;


            fvalue1 =  (x0+1.0-rotate_s->x)*fcos;
            fvalue2 =  0 - ((y0+1.0-rotate_s->y)*fsin);
            p[2].x   =  fvalue1 + fvalue2 + rotate_s->x;
            fvalue1 =  (x0+1.0-rotate_s->x)*fsin;
            fvalue2 =  (y0+1.0-rotate_s->y)*fcos;
            p[2].y   =  fvalue1 + fvalue2 + rotate_s->y;


            fvalue1 =  (x0-rotate_s->x)*fcos;
            fvalue2 =  0 - ((y0+1.0-rotate_s->y)*fsin);
            p[3].x   =  fvalue1 + fvalue2 + rotate_s->x;
            fvalue1 =  (x0-rotate_s->x)*fsin;
            fvalue2 =  (y0+1.0-rotate_s->y)*fcos;
            p[3].y   =  fvalue1 + fvalue2 + rotate_s->y;


            /* Caculate center of the polygon */
            c.x = 0;
            c.y = 0;
            for (i = 0; i < 4; i++)
            {
                c.x += p[i].x;
                c.y += p[i].y;

            }
            c.x = c.x / 4.0;
            c.y = c.y / 4.0;


	    /* Find the scan area on the destination's pixels */
            fvalue1 = GUI_MIN(p[0].x, p[1].x);
            fvalue2 = GUI_MIN(p[2].x, p[3].x);
            min_dx  = (int)(GUI_MIN(fvalue1,fvalue2));

            fvalue1 = GUI_MAX(p[0].x, p[1].x);
            fvalue2 = GUI_MAX(p[2].x, p[3].x);
            alpha   = GUI_MAX(fvalue1,fvalue2);
            max_dx  = in_gui_round_up(alpha);
         
            fvalue1 = GUI_MIN(p[0].y, p[1].y);
            fvalue2 = GUI_MIN(p[2].y, p[3].y);
            min_dy  = (int)(GUI_MIN(fvalue1,fvalue2));

            fvalue1 = GUI_MAX(p[0].y, p[1].y);
            fvalue2 = GUI_MAX(p[2].y, p[3].y);
            alpha   = GUI_MAX(fvalue1,fvalue2);
            max_dy  = in_gui_round_up(alpha);
 

            /* 
	     * loop through the scan area to find where source(x, y) 
	     * overlaps with the destination pixels 
	     */

	    /* printf("mindx,maxdx,mindy,maxdy: %d, %d, %d, %d\n\r",mindx,maxdx,mindy,maxdy); */

            for (sub_y = min_dy; sub_y < max_dy; sub_y++)
            {  
                for (sub_x = min_dx; sub_x < max_dx; sub_x++)
                {
                    for (i = 0; i < 4; i++)
                    {
                        poffset[i].x = p[i].x - sub_x;
                        poffset[i].y = p[i].y - sub_y;
                    }

		    new_c.x = c.x - sub_x;
		    new_c.y = c.y - sub_y;

		    overlap = in_caliculate_overlap(poffset, &new_c, &fsin, &fcos);
                    if (!overlap)
		        continue;

                    index = (sub_x - rect.left) + (sub_y - rect.top) * new_width;
		    if ( index < 0 )
			continue;


		    if ((pbitmap->bits_per_pixel) == 1)
		    {
                        offset = ((x0-x)/8) + ((pbitmap->height) - (y0-y) - 1) * (pbitmap->bytes_per_line);
                        bit    = ((x0-x)%8);
		    } else if ((pbitmap->bits_per_pixel) == 2) {
                        offset = ((x0-x)/4) + ((pbitmap->height) - (y0-y) - 1) * (pbitmap->bytes_per_line);
                        bit    = ((x0-x)%4);
		    } else if ((pbitmap->bits_per_pixel) == 4) {
                        offset = ((x0-x)/2) + ((pbitmap->height) - (y0-y) - 1) * (pbitmap->bytes_per_line);
                        bit    = ((x0-x)%2);
		    } else {
                        offset = (x0-x) + ((pbitmap->height) - (y0-y) - 1) * (pbitmap->width);
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

                    rotate_s->buffer[index].red   += (red_color * overlap);
                    rotate_s->buffer[index].green += (green_color * overlap);
                    rotate_s->buffer[index].blue  += (blue_color * overlap);
                    rotate_s->buffer[index].alpha += overlap;
                }
            }
        }
    }


    for (j = 0; j < new_height; j++)
    {
        for (i = 0; i < new_width; i++)
        {
            index = i + j*new_width;
	 
            alpha =  rotate_s->buffer[index].alpha;
            if ( alpha == 0 )
                continue;

	    if ( alpha < 0.95)
	    {
                ret = in_point_input_pixel_abs(hdc,i+rect.left+hdc->rect.left,j+rect.top+hdc->rect.top, &gui_color); 
		if ( ret < 0 )
		    continue;

                blue_color  = in_gui_byte((rotate_s->buffer[index].blue+(1-alpha)*(gui_color&0xFF)) );
                green_color = in_gui_byte((rotate_s->buffer[index].green + (1-alpha)*((gui_color&0xFF00)>>8)));
                red_color   = in_gui_byte((rotate_s->buffer[index].red + (1-alpha)*((gui_color&0xFF0000)>>16)));
	    } else {
                blue_color  = in_gui_byte(rotate_s->buffer[index].blue);
                green_color = in_gui_byte(rotate_s->buffer[index].green);
                red_color   = in_gui_byte(rotate_s->buffer[index].red);
	    }
	   
            gui_color = (red_color << 16) | (green_color << 8) | blue_color;
            screen_color = (lscrn.gui_to_screen_color)(gui_color);

            in_output_screen_pixel_abs(hdc, i+rect.left+hdc->rect.left, j+rect.top+hdc->rect.top, screen_color);
	}
    }

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  gui_bitmap_rotate_area(HDC hdc, int x, int y, const void *bitmap, void *rotate)
{
    int  ret = 0;

    gui_lock( );
    ret = in_gui_bitmap_rotate_area(hdc, x, y, bitmap, rotate);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif    /* _LG_BITMAP_ */
