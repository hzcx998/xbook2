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
#include  <string.h>
#include  <math.h>

#include  <lgmacro.h>

#include  <lock.h>
#include  <cursor.h>

#include  <dc.h>
#include  <d2_pixel.h>
#include  <screen.h>

#include  <text_ops.h>
#include  <image_comm.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif



#ifdef  _LG_FONT_

static  int    lg_start_x         = 0;
static  int    lg_start_y         = 0;

static  int    lg_current_x       = 0;
static  int    lg_current_y       = 0;

static  BINT   lg_start_new_line  = 0;

static  UCHAR  start_data[288+1]  = {0};
static  unsigned int  data_len    = 0;

#define  in_text_out_blank(pdc)       (lg_current_x += MISSING_CHAR_BLANK_WIDTH)
#define  in_text_out_tab(pdc)         (lg_current_x += MISSING_CHAR_BLANK_WIDTH*4)



#ifdef  _LG_MONO_CHARSET_FONT_
static  int  in_mono_charset_text_out(HDC hdc, const void *font, const TCHAR *str, int code_counter, unsigned int *position, GUI_SYMMETRY_ROTATE *s_r)
{
    MONO_CHARSET_FONT  *first_mono_charset_font  = (MONO_CHARSET_FONT *)font;
    MONO_CHARSET_FONT  *start_mono_charset_font  = (MONO_CHARSET_FONT *)font;
    MONO_CHARSET_FONT  *mono_charset_font        = (MONO_CHARSET_FONT *)font;

    unsigned int    str_counter = code_counter;
    TCHAR           *pchar = 0;

    #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
    unsigned char multi_byte_flag = 0;
    #endif

    unsigned int  width        = 0;
    unsigned int  height       = 0;
    unsigned int  index        = 0;
    unsigned int  n            = 0;
    unsigned int  pos          = 0;
    unsigned int  start_index  = 0;

    GUI_COLOR     gui_color = 0;
    SCREEN_COLOR  screen_foreground_color = 0;  
    SCREEN_COLOR  screen_back_color       = 0;
    SCREEN_COLOR  screen_color            = 0;

    UCHAR           tchar   = 0;
    int             itemp   = 0;

    int             x       = 0;
    int             y       = 0;  

    int             x0      = 0;
    int             y0      = 0;  
    int         new_x       = 0;
    int         new_y       = 0;  

    double      fsin        = 0;
    double      fcos        = 0;
    double      fvalue1     = 0;
    double      fvalue2     = 0;


    unsigned char   lf_flag = 0;
    unsigned char   cr_flag = 0;

    int             ret     = 0;

             int    j       = 0;
             int    k       = 0;
 
             int    shift = 0;



    if ( hdc == NULL )
        return  -1;
    if ( mono_charset_font == NULL )
        return  -1;
    if ( s_r == NULL )
	return  -1;
    if ( position == NULL)
        return  -1;


    if ( (s_r->rotate.is_rotate) )
    {
        fsin = sin(((s_r->rotate.theta)*PI)/IMAGE_ROTATE_180);
        fcos = cos(((s_r->rotate.theta)*PI)/IMAGE_ROTATE_180);
        x0   = s_r->rotate.x + hdc->rect.left;
        y0   = s_r->rotate.y + hdc->rect.top;
    }

    gui_color = in_hdc_get_text_fore_color(hdc);
    screen_foreground_color = (lscrn.gui_to_screen_color)(gui_color);

    gui_color = in_hdc_get_text_back_color(hdc);
    screen_back_color = (lscrn.gui_to_screen_color)(gui_color);

    width  = mono_charset_font->width;
    height = mono_charset_font->height;

    index     = 0;
    pos       = 0;
    *position = 0;

    for ( n = 0; n < str_counter; n++ )
    {
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        multi_byte_flag = 0;
        #endif

        start_mono_charset_font = mono_charset_font;
        pchar = (TCHAR *)(&str[n]);
        if ( *pchar == GUI_LF )
        {   
            if  ( cr_flag == 0 )
            {
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  = hdc->text_metric.space_left;
                lg_current_y += height + hdc->text_metric.space_top + hdc->text_metric.space_bottom;
                #else
                lg_current_x  = 0;
                lg_current_y += height;
                #endif
 
                lg_start_new_line = 0; 

                lf_flag = 1;
            } else {
                lf_flag = 0;
                cr_flag = 0;
            }

            pos++;
            continue; 
        }

        if ( *pchar == GUI_CR )
        { 
            if (lf_flag == 0 )
            {     
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  = hdc->text_metric.space_left;
                lg_current_y += height + hdc->text_metric.space_top + hdc->text_metric.space_bottom;
                #else
                lg_current_x  = 0;
                lg_current_y += height;
                #endif
 
                lg_start_new_line = 0; 
                cr_flag = 1;
            } else {
                cr_flag = 0;
                lf_flag = 0;
            }

            pos++;
            continue; 
        }

        if ( *pchar == GUI_TAB )
        {
            in_text_out_tab(pdc); 
            pos++;
            continue; 
        }

        if ( *pchar == NULL_CHAR )
        {
            pos++;
            continue; 
        }

        lf_flag = 0;
        cr_flag = 0;

        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( (unsigned char)(*pchar) >= MULTI_BYTE_START_CODE  )
        {
            multi_byte_flag = 1;
            n++;
        } 
        #endif

        MONO_CHARSET_FONT_START:
        ret = mono_charset_font->is_in_this_charset_block(pchar);
        if ( ret < 0 )
            goto  MONO_CHARSET_FONT_NEXT;

        #ifdef  _LG_TEXT_METRIC_
        lg_current_x  += hdc->text_metric.space_left;
        #endif

        if ( lg_start_new_line )
        {
            #ifdef  _LG_TEXT_METRIC_
            lg_current_y  += hdc->text_metric.space_top;
            #endif
            lg_start_new_line = 0; 
        }

        #ifdef  _LG_TEXT_METRIC_
        if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_TOP) == TEXT_FRAME_TOP )
        {
            y = hdc->rect.top + lg_current_y;
            for ( k = 0; k < width; k++ )
            {
                x = hdc->rect.left + lg_current_x + k;

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
		    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}


		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
                    new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
	        {
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			

		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		         new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			 new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		         continue;
		    } else {
	                 new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			 new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;


	            x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
	        }


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

	        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		     continue;
	        } else {
	             new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
	             new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
	        new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }

        if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_LEFT) == TEXT_FRAME_LEFT )
        {
            x = hdc->rect.left + lg_current_x;

            for ( j = 0; j < height; j++ )
            {
                y = hdc->rect.top + lg_current_y + j;

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
		    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}


		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
                    new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
	        {
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			

		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		         new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			 new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		         continue;
		    } else {
	                 new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			 new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;


	            x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
	        }


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

	        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		     continue;
	        } else {
	             new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
	             new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
	        new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }


        if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_RIGHT) == TEXT_FRAME_RIGHT )
        {
            y = hdc->rect.top + lg_current_y;

            for ( j = 0; j < height; j++ )
            {
                x = hdc->rect.left + lg_current_x + k;

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
		    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}


		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
                    new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
	        {
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			

		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		         new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			 new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		         continue;
		    } else {
	                 new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			 new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;


	            x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
	        }


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

	        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		     continue;
	        } else {
	             new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
	             new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
	        new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }

        if ( hdc->text_metric.is_strike_out )
        {
            itemp = height/2;
            for ( k = 0; k < width; k++ )
            {
                x = hdc->rect.left + lg_current_x + k;
                y = hdc->rect.top + lg_current_y +itemp;

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{
		    /* ?? y+2 or y-2 */	
		    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}


		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
                    new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
	        {
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			

		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		         new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			 new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		         continue;
		    } else {
	                 new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			 new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;


	            x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
	        }


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

	        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		     continue;
	        } else {
	             new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
	             new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
	        new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }
        #endif

	if ( (mono_charset_font->is_get_serial_data) > 0 )
	{
            data_len = sizeof(start_data);
	    memset(start_data, 0, sizeof(start_data));
            ret = mono_charset_font->get_serial_data(pchar, start_data, &data_len);
            if ( ret < 0 )
                goto  MONO_CHARSET_FONT_NEXT;

	    if ( data_len > sizeof(start_data) )
                goto  MONO_CHARSET_FONT_NEXT;

	    itemp = (width*height)/8;
	    if ( ((width*height)%8) > 0 )
	        itemp += 1;
            if ( data_len < itemp )
                goto  MONO_CHARSET_FONT_NEXT;
	} else {
            start_index = mono_charset_font->get_data_start_index(pchar);
	}
        for ( j = 0; j < height; j++ )
        {
            for ( k = 0; k < width; k++ )
            {
                itemp   =  width*j + k;
                index   =  itemp/8;
                shift   =  itemp%8;

                /* tchar = *((UCHAR *)((void *)(mono_charset_font->data) + start_index + index)); */
	        if ( (mono_charset_font->is_get_serial_data) > 0 )
                    tchar = start_data[index];
		else
                    tchar = *((UCHAR *)((UCHAR *)(mono_charset_font->data) + start_index + index));
               
                x = hdc->rect.left + lg_current_x;
                y = hdc->rect.top + lg_current_y;


                /* */
                if ( (tchar << shift )&0x80 )
                    screen_color = screen_foreground_color; 
		else if ( (hdc->back_mode) == MODE_COPY ) {
                    screen_color = screen_back_color; 
		} else {
                    lg_current_x++;  
		    continue;
	        }


		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
	            in_output_screen_pixel_abs(hdc,x,y,screen_color);

	            lg_current_x++;  
	            continue;
		}


		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                        lg_current_x++;  
		        continue;
	            } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

                    lg_current_x++;  
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		{
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

	            lg_current_x++;  
	            continue;

		} 
			

		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                        lg_current_x++;  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;


	            x       = new_x;
	            y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

                    lg_current_x++;  
	            continue;

		}


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

	        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                    lg_current_x++;  
		    continue;
		} else {
	            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
	            new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
		new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

		lg_current_x++;  
		/* continue; */

		/* */
            }
         
            #ifdef  _LG_TEXT_METRIC_
            lg_current_x -= width + hdc->text_metric.offset_italic;
            #else
            lg_current_x -= width;
            #endif

            lg_current_y++;

        }

        #ifdef  _LG_TEXT_METRIC_
        if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_BOTTOM) == TEXT_FRAME_BOTTOM )
        {
            y = hdc->rect.top + lg_current_y;
            for ( k = 0; k < width; k++ )
            {
                x = hdc->rect.left + lg_current_x + k;

	        /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
		    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}


		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
                    new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
	        {
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			

		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		         new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		         new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			 new_x = (x - hdc->rect.left);
	            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		         continue;
		    } else {
	                 new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			 new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;


	            x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
	        }


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

	        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		     continue;
	        } else {
	             new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
	             new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
	        new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }
        #endif 

        #ifdef  _LG_TEXT_METRIC_
        lg_current_x += width+hdc->text_metric.offset_italic*height + hdc->text_metric.space_right;
        lg_current_y -= height+hdc->text_metric.offset_escapement;
        #else
        lg_current_x += width;
        lg_current_y -= height;
        #endif               
 
        pos++;
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( multi_byte_flag )
            pos++;
        #endif

        continue;

        MONO_CHARSET_FONT_NEXT: 
        mono_charset_font = mono_charset_font->next;
        if ( mono_charset_font == start_mono_charset_font )
            goto  MONO_CHARSET_FONT_END;

        if ( mono_charset_font == 0 )
        {
            mono_charset_font = first_mono_charset_font;
            if ( mono_charset_font == start_mono_charset_font )
                goto  MONO_CHARSET_FONT_END;
            
            width   = mono_charset_font->width;
            height  = mono_charset_font->height;
            index  = 0;

            goto  MONO_CHARSET_FONT_START;
        }
 
        width        = mono_charset_font->width;
        height       = mono_charset_font->height;

        index = 0;

        goto  MONO_CHARSET_FONT_START;
    }

    MONO_CHARSET_FONT_END:
    *position = pos; 

    return  1;
}
#endif  /* _LG_MONO_CHARSET_FONT_ */


#ifdef  _LG_MONO_DISCRETE_FONT_
static  int  in_mono_discrete_text_out(HDC hdc, const void *font, const TCHAR *str, int code_counter, unsigned int *position, GUI_SYMMETRY_ROTATE *s_r)
{
    MONO_DISCRETE_FONT  *first_mono_discrete_font  = (MONO_DISCRETE_FONT *)font;
    MONO_DISCRETE_FONT  *start_mono_discrete_font  = (MONO_DISCRETE_FONT *)font;
    MONO_DISCRETE_FONT  *mono_discrete_font        = (MONO_DISCRETE_FONT *)font;

    unsigned int    str_counter = code_counter;

    UCHAR     *pcharset = 0;
    TCHAR     *pchar    = 0;
    unsigned int  char_counter = 0;

    #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
    unsigned char multi_byte_flag = 0;
    #endif


    unsigned int  width        = 0;
    unsigned int  height       = 0;
    unsigned int  totall_bytes = 0;
    unsigned int  index        = 0;
    unsigned int  n            = 0;
    unsigned int  pos          = 0;

    GUI_COLOR     gui_color = 0;
    SCREEN_COLOR  screen_foreground_color = 0;  
    SCREEN_COLOR  screen_back_color = 0;
    SCREEN_COLOR  screen_color = 0;


    UCHAR           tchar   = 0;
    int             itemp   = 0;
    int             x       = 0;
    int             y       = 0;  

    int             x0      = 0;
    int             y0      = 0;  
    int         new_x       = 0;
    int         new_y       = 0;  

    double      fsin        = 0;
    double      fcos        = 0;
    double      fvalue1     = 0;
    double      fvalue2     = 0;


    unsigned char   lf_flag = 0;
    unsigned char   cr_flag = 0;

             int    i = 0; 
             int    j = 0;
             int    k = 0;

             int    shift = 0;



    if ( hdc == NULL )
        return  -1;
    if ( mono_discrete_font == NULL )
        return  -1;
    if ( s_r == NULL )
	return -1;
    if ( position == NULL )
        return  -1;



    if ( s_r->rotate.is_rotate )
    {
        fsin = sin(((s_r->rotate.theta)*PI)/IMAGE_ROTATE_180);
        fcos = cos(((s_r->rotate.theta)*PI)/IMAGE_ROTATE_180);
        x0   = s_r->rotate.x + hdc->rect.left;
        y0   = s_r->rotate.y + hdc->rect.top;
    }


    gui_color = in_hdc_get_text_fore_color(hdc);
    screen_foreground_color = (lscrn.gui_to_screen_color)(gui_color);

    gui_color = in_hdc_get_text_back_color(hdc);
    screen_back_color = (lscrn.gui_to_screen_color)(gui_color);

    char_counter = mono_discrete_font->counter;
    width        = mono_discrete_font->width;
    height       = mono_discrete_font->height;

    totall_bytes = (width*height)/8;
    if ( ((width*height)%8) > 0 )
        totall_bytes += 1;

    index     = 0;
    pos       = 0;
    *position = 0;


    for ( n = 0; n < str_counter; n++ )
    {
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        multi_byte_flag = 0;
        #endif

        start_mono_discrete_font = mono_discrete_font;

        pchar =(TCHAR *)(&str[n]);

        if ( *pchar == GUI_LF )
        {   
            if  ( cr_flag == 0 )
            {
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  = hdc->text_metric.space_left;
                lg_current_y += height + hdc->text_metric.space_top + hdc->text_metric.space_bottom;
                #else
                lg_current_x  = 0;
                lg_current_y += height;
                #endif
 
                lg_start_new_line = 0; 

                lf_flag = 1;
            } else {
                lf_flag = 0;
                cr_flag = 0;
            }

            pos++;
            continue; 
        }

        if ( *pchar == GUI_CR )
        { 
            if (lf_flag == 0 )
            {     
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  = hdc->text_metric.space_left;
                lg_current_y += height + hdc->text_metric.space_top + hdc->text_metric.space_bottom;
                #else
                lg_current_x  = 0;
                lg_current_y += height;
                #endif
 
                lg_start_new_line = 0; 

                cr_flag = 1;
            } else {
                cr_flag = 0;
                lf_flag = 0;
            }

            pos++;
            continue; 
        }

        if ( *pchar == GUI_TAB )
        { 
            in_text_out_tab(pdc);
            pos++;
            continue; 
        }

        if ( *pchar == NULL_CHAR )
        {
            pos++;
            continue; 
        }

        lf_flag = 0;
        cr_flag = 0;

        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( (unsigned char)(*pchar) >= MULTI_BYTE_START_CODE  )
        {
            multi_byte_flag = 1;
            n++;
        } 
        #endif

        MONO_DISCRETE_FONT_START:
        pcharset = mono_discrete_font->mono_char; 
        for (i = 0; i < char_counter; i++)
        {
            #ifdef  _LG_UNICODE_VERSION_
                itemp = *((UINT16 *)pcharset) - *pchar;
            #else
                #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
                if ( multi_byte_flag )
                    itemp = strncmp((const char *)pcharset, pchar, 2*sizeof(UCHAR));
                else
                    itemp = strncmp((const char *)pcharset, pchar, sizeof(UCHAR));
                #else
                itemp = strncmp((const char *)pcharset, pchar, sizeof(UCHAR));
                #endif
            #endif

            if ( itemp == 0 )
            {
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  += hdc->text_metric.space_left;
                #endif

                if ( lg_start_new_line )
                {
                    #ifdef  _LG_TEXT_METRIC_
                    lg_current_y  += hdc->text_metric.space_top;
                    #endif
                    lg_start_new_line = 0; 
                }

                #ifdef  _LG_TEXT_METRIC_
                if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_TOP) == TEXT_FRAME_TOP )
                {
                    y = hdc->rect.top + lg_current_y;
                    for ( k = 0; k < width; k++ )
                    {
                        x = hdc->rect.left + lg_current_x + k;
			   
                        /* */
			if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
			{	
			    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
			    continue;
			}


			if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;
			       
                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
				    		    
		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
			{
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
			

		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;


			    x       = new_x;
			    y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			}


			/* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

			if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			{
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    new_x = (x - hdc->rect.left);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
			} else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			    new_y = y - hdc->rect.top;
			}     

		        new_x += hdc->rect.left;
			new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			/* continue; */
			/* */
                    }
                }

                if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_LEFT) == TEXT_FRAME_LEFT )
                {
                    x = hdc->rect.left + lg_current_x;

                    for ( j = 0; j < height; j++ )
                    {
                        y = hdc->rect.top + lg_current_y + j;


			if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
			{	
			    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
			    continue;
			}


			if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;
			       
                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
				    		    
		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
			{
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
			

		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;


			    x       = new_x;
			    y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			}

			/* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

			if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			{
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    new_x = (x - hdc->rect.left);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
			} else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			    new_y = y - hdc->rect.top;
			}     

		        new_x += hdc->rect.left;
			new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			/* continue; */
			/* */
                    }
                }



                if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_RIGHT) == TEXT_FRAME_RIGHT )
                {
                    y = hdc->rect.top + lg_current_y;

                    for ( j = 0; j < height; j++ )
                    {
                        x = hdc->rect.left + lg_current_x + k;


			if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
			{	
			    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
			    continue;
			}


			if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;
			       
                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
				    		    
		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
			{
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
			

		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;


			    x       = new_x;
			    y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			}

			/* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

			if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			{
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    new_x = (x - hdc->rect.left);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
			} else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			    new_y = y - hdc->rect.top;
			}     

		        new_x += hdc->rect.left;
			new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			/* continue; */
			/* */
                    }
                }


                if ( hdc->text_metric.is_strike_out )
                {
                    itemp = height/2;
                    for ( k = 0; k < width; k++ )
                    {
                        x = hdc->rect.left + lg_current_x + k;
                        y = hdc->rect.top + lg_current_y +itemp;


			if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
			{	
			    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
			    continue;
			}


			if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;
			       
                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
				    		    
		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
			{
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
			

		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;


			    x       = new_x;
			    y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			}

			/* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

			if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			{
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    new_x = (x - hdc->rect.left);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
			} else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			    new_y = y - hdc->rect.top;
			}     

		        new_x += hdc->rect.left;
			new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			/* continue; */
			/* */
                    }
                }
                #endif

                for ( j = 0; j < height; j++ )
                {
                    for ( k = 0; k < width; k++ )
                    {
                        itemp  = (width*j + k);
                        index  = (itemp/8);
                        shift  = (itemp%8);
	
                        /* tchar = *((void *)(pcharset+2*sizeof(UCHAR)+index)); */
                        tchar = *((UCHAR *)(pcharset+2*sizeof(UCHAR)+index));

                        x = hdc->rect.left + lg_current_x;
                        y = hdc->rect.top + lg_current_y;

	
                        if ( (tchar << shift )&0x80 )
                            screen_color = screen_foreground_color; 
			else if ( (hdc->back_mode) == MODE_COPY ) {
                            screen_color = screen_back_color; 
			} else {
	                    lg_current_x++;  
			    continue;
			}


			if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
			{	
			    in_output_screen_pixel_abs(hdc,x,y,screen_color);

	                    lg_current_x++;  
			    continue;
			}


			if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                                lg_current_x++;  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;
			       
                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

                            lg_current_x++;  
			    continue;
			} 
				    		    
		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
			{
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

			    lg_current_x++;  
			    continue;

			} 
			

		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                                lg_current_x++;  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;


			    x       = new_x;
			    y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

                            lg_current_x++;  
			    continue;

			}


			/* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

			if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			{
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    new_x = (x - hdc->rect.left);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                            lg_current_x++;  
		            continue;
			} else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			    new_y = y - hdc->rect.top;
			}     

		        new_x += hdc->rect.left;
			new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

			lg_current_x++;  
			/* continue; */
                    }
          
                    #ifdef  _LG_TEXT_METRIC_
                    lg_current_x -= width + hdc->text_metric.offset_italic;
                    #else
                    lg_current_x -= width;
                    #endif

                    lg_current_y++;
                }

                #ifdef  _LG_TEXT_METRIC_
                if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_BOTTOM) == TEXT_FRAME_BOTTOM )
                {
                    y = hdc->rect.top + lg_current_y;
                    for ( k = 0; k < width; k++ )
                    {
                        x = hdc->rect.left + lg_current_x + k;


			if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
			{	
			    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
			    continue;
			}


			if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;
			       
                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
				    		    
		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
			{
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			} 
			

		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
			{
			    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			    {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
			    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
				continue;
			    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
			    }     

		            new_x += hdc->rect.left;
			    new_y += hdc->rect.top;


			    x       = new_x;
			    y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			    continue;
			}

			/* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

			if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
			{
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			    new_x = (x - hdc->rect.left);
			}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
			} else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			    new_y = y - hdc->rect.top;
			}     

		        new_x += hdc->rect.left;
			new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
			/* continue; */
                    }
                }
                #endif 

                #ifdef  _LG_TEXT_METRIC_
                lg_current_x += width + hdc->text_metric.offset_italic*height + hdc->text_metric.space_right;
                lg_current_y -= height + hdc->text_metric.offset_escapement;
                #else
                lg_current_x += width;
                lg_current_y -= height;
                #endif               
 
                pos++;
                #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
                if ( multi_byte_flag )
                    pos++;
                #endif

                break;

            }
            pcharset += 2*sizeof(UCHAR) + totall_bytes;
        }

        if ( i >= char_counter ) 
        {
            mono_discrete_font = mono_discrete_font->next;
            if ( mono_discrete_font == start_mono_discrete_font )
                goto  MONO_DISCRETE_FONT_END;

            if ( mono_discrete_font == 0 )
            {
                mono_discrete_font = first_mono_discrete_font;
                if ( mono_discrete_font == start_mono_discrete_font )
                    goto  MONO_DISCRETE_FONT_END;

                char_counter = mono_discrete_font->counter;
                width        = mono_discrete_font->width;
                height       = mono_discrete_font->height;

                totall_bytes = (width*height)/8;
                if ( ((width*height)%8) > 0 )
                    totall_bytes += 1;

                index = 0;

                goto  MONO_DISCRETE_FONT_START;
            }
 
            char_counter = mono_discrete_font->counter;
            width        = mono_discrete_font->width;
            height       = mono_discrete_font->height;

            totall_bytes = (width*height)/8;
            if ( ((width*height)%8) > 0 )
                totall_bytes += 1;

            index = 0;

            goto  MONO_DISCRETE_FONT_START;
        }
    }

    MONO_DISCRETE_FONT_END:
    *position = pos; 

    return  1;
}
#endif  /* _LG_MONO_DISCRETE_FONT_ */


#ifdef  _LG_MIXED_CHARSET_FONT_
static  int  in_mixed_charset_text_out(HDC hdc, const void *font, const TCHAR *str, int code_counter, unsigned int *position, GUI_SYMMETRY_ROTATE *s_r)
{
    MIXED_CHARSET_FONT  *first_mixed_charset_font  = (MIXED_CHARSET_FONT *)font;
    MIXED_CHARSET_FONT  *start_mixed_charset_font  = (MIXED_CHARSET_FONT *)font;
    MIXED_CHARSET_FONT  *prop_charset_font         = (MIXED_CHARSET_FONT *)font;

    unsigned int    str_counter = code_counter;

    MIXED_CHARSET_CHAR  *pcharset = 0;
    TCHAR               *pchar    = 0;

    #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
    unsigned char multi_byte_flag = 0;
    #endif

    unsigned int  width             = 0;
    unsigned int  height            = 0;
    unsigned int  totall_bytes      = 0;
    unsigned int  index             = 0;
    unsigned int  n                 = 0;
    unsigned int  pos               = 0;
    unsigned int  start_char_index  = 0;

    GUI_COLOR       gui_color;
    SCREEN_COLOR    screen_foreground_color;  
    SCREEN_COLOR    screen_back_color;
    SCREEN_COLOR    screen_color;

    UCHAR           tchar   = 0;
    int             itemp   = 0;
    int             x       = 0;
    int             y       = 0;
    int             x0      = 0;
    int             y0      = 0;  
    int         new_x       = 0;
    int         new_y       = 0;  

    double      fsin        = 0;
    double      fcos        = 0;
    double      fvalue1     = 0;
    double      fvalue2     = 0;

 

    unsigned char   lf_flag = 0;
    unsigned char   cr_flag = 0;

             int    ret = 0;

             int    j;

             int    k = 0; 

             int    shift = 0;


    if ( hdc == NULL )
        return  -1;
    if ( prop_charset_font == NULL )
        return  -1;
    if ( s_r == NULL )
	return  -1;
    if ( position == NULL)
        return  -1;


    if ( (s_r->rotate.is_rotate) )
    {
        fsin = sin(((s_r->rotate.theta)*PI)/IMAGE_ROTATE_180);
        fcos = cos(((s_r->rotate.theta)*PI)/IMAGE_ROTATE_180);
        x0   = s_r->rotate.x + hdc->rect.left;
        y0   = s_r->rotate.y + hdc->rect.top;
    }

    gui_color = in_hdc_get_text_fore_color(hdc);
    screen_foreground_color = (lscrn.gui_to_screen_color)(gui_color);

    gui_color = in_hdc_get_text_back_color(hdc);
    screen_back_color = (lscrn.gui_to_screen_color)(gui_color);

    index     = 0;
    pos       = 0;
    *position = 0;

    for ( n = 0; n < str_counter; n++ )
    {
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        multi_byte_flag = 0;
        #endif

        start_mixed_charset_font = prop_charset_font;

        pchar = (TCHAR *)(&str[n]);

        if ( *pchar == GUI_LF )
        {   
            if  ( cr_flag == 0 )
            {
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  = hdc->text_metric.space_left;
                lg_current_y += height + hdc->text_metric.space_top + hdc->text_metric.space_bottom;
                #else
                lg_current_x  = 0;
                lg_current_y += height;
                #endif
 
                lg_start_new_line = 0; 

                lf_flag = 1;
            } else {
                lf_flag = 0;
                cr_flag = 0;
            }

            pos++;
            continue; 
        }

        if ( *pchar == GUI_CR )
        { 
            if (lf_flag == 0 )
            {     
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  = hdc->text_metric.space_left;
                lg_current_y += height + hdc->text_metric.space_top + hdc->text_metric.space_bottom;
                #else
                lg_current_x  = 0;
                lg_current_y += height;
                #endif
 
                lg_start_new_line = 0; 

                cr_flag = 1;
            } else {
                cr_flag = 0;
                lf_flag = 0;
            }

            pos++;
            continue; 
        }

        if ( *pchar == GUI_TAB )
        {
            in_text_out_tab(pdc);
 
            pos++;
            continue; 
        }

        if ( *pchar == NULL_CHAR )
        {
            pos++;
            continue; 
        }

        lf_flag = 0;
        cr_flag = 0;

        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( (unsigned char)(*pchar) >= MULTI_BYTE_START_CODE )
        {
            multi_byte_flag = 1;
            n++;
        } 
        #endif


        MIXED_CHARSET_FONT_START:
        ret = prop_charset_font->is_in_this_charset_block(pchar);
        if ( ret < 0 )
            goto  MIXED_CHARSET_FONT_NEXT;


        #ifdef  _LG_TEXT_METRIC_
        lg_current_x  += hdc->text_metric.space_left;
        #endif

        if ( lg_start_new_line )
        {
            #ifdef  _LG_TEXT_METRIC_
            lg_current_y  += hdc->text_metric.space_top;
            #endif
            lg_start_new_line = 0; 
        }

        #ifdef  _LG_TEXT_METRIC_
        if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_TOP) == TEXT_FRAME_TOP )
        {
            y = hdc->rect.top + lg_current_y;
            for ( k = 0; k < width; k++ )
            {
                x = hdc->rect.left + lg_current_x + k;

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
	            in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}

		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
	            } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		{
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			
		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
		    }     

		    new_x += hdc->rect.left;
		    new_y += hdc->rect.top;


		    x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		    continue;
		}


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

		if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		    continue;
		} else {
	            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
		new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }

        if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_LEFT) == TEXT_FRAME_LEFT )
        {
            x = hdc->rect.left + lg_current_x;
            for ( j = 0; j < height; j++ )
            {
                y = hdc->rect.top + lg_current_y + j;

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
	            in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}

		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
	            } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		{
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			
		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
		    }     

		    new_x += hdc->rect.left;
		    new_y += hdc->rect.top;


		    x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		    continue;
		}


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

		if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		    continue;
		} else {
	            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
		new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }


        if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_RIGHT) == TEXT_FRAME_RIGHT )
        {
            y = hdc->rect.top + lg_current_y;

            for ( j = 0; j < height; j++ )
            {
                /* ??: + k  */
                x = hdc->rect.left + lg_current_x + k;

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
	            in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}

		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
	            } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		{
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			
		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
		    }     

		    new_x += hdc->rect.left;
		    new_y += hdc->rect.top;


		    x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		    continue;
		}


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

		if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		    continue;
		} else {
	            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
		new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
           }
        }


        if ( hdc->text_metric.is_strike_out )
        {
            itemp = height/2;
            for ( k = 0; k < width; k++ )
            {
                x = hdc->rect.left + lg_current_x + k;
                y = hdc->rect.top + lg_current_y +itemp;

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
	            in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}

		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
	            } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		{
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			
		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
		    }     

		    new_x += hdc->rect.left;
		    new_y += hdc->rect.top;


		    x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		    continue;
		}


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

		if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		    continue;
		} else {
	            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
		new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }
        #endif
 
        start_char_index = prop_charset_font->get_code_index(pchar);

        pcharset = (prop_charset_font)->mixed_char;
        pcharset  += start_char_index;

        width    = pcharset->width;
        height   = pcharset->height;
    
        totall_bytes = (width*height)/8;
        if ( ((width*height)%8) > 0 )
            totall_bytes += 1;

        for ( j = 0; j < height; j++ )
        {
            for ( k = 0; k < width; k++ )
            {
                itemp   =  width*j + k;
                index   =  itemp/8;
                shift   =  itemp%8;

                /* tchar =  *((UCHAR *)((pcharset->data) + index)); */
                tchar =  *((UCHAR *)((pcharset->data) + index));

                x = hdc->rect.left + lg_current_x;
                y = hdc->rect.top + lg_current_y;


                if ( (tchar << shift )&0x80 )
                    screen_color = screen_foreground_color; 
		else if ( (hdc->back_mode) == MODE_COPY ) {
                    screen_color = screen_back_color; 
		} else {
	            lg_current_x++;  
		    continue;
		}



	        if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
		    in_output_screen_pixel_abs(hdc,x,y,screen_color);

	            lg_current_x++;  
	            continue;
		}


		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                        lg_current_x++;  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     


		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

                    lg_current_x++;  
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		{
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

	            lg_current_x++;  
		    continue;

		} 
			
		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                        lg_current_x++;  
		        continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
		    }     

		    new_x += hdc->rect.left;
		    new_y += hdc->rect.top;


	            x       = new_x;
	            y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

                    lg_current_x++;  
	            continue;

		}


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

		if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	            new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                    lg_current_x++;  
		    continue;
		} else {
	            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
	            new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
		new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

		lg_current_x++;  
		/* continue; */
            }
          
            #ifdef  _LG_TEXT_METRIC_
            lg_current_x -= width + hdc->text_metric.offset_italic;
            #else
            lg_current_x -= width;
            #endif

            lg_current_y++;
        }

        #ifdef  _LG_TEXT_METRIC_
        if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_BOTTOM) == TEXT_FRAME_BOTTOM )
        {
            y = hdc->rect.top + lg_current_y;
            for ( k = 0; k < width; k++ )
            {
                x = hdc->rect.left + lg_current_x + k;	    

                /* */
		if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		{	
	            in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		    continue;
		}

		if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		{
		    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
	            } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
	            }     

		    new_x += hdc->rect.left;
	            new_y += hdc->rect.top;
			       
                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
				    		    
		if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		{
                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;

                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	            continue;
		} 
			
		if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		{
	            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		    {
		        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		        new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			new_x = (x - hdc->rect.left);
		    }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			continue;
		    } else {
	                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			new_y = y - hdc->rect.top;
		    }     

		    new_x += hdc->rect.left;
		    new_y += hdc->rect.top;


		    x       = new_x;
		    y       = new_y;

                    fvalue1 =  ((int)(x-x0))*fcos;
                    fvalue2 = -((int)(y-y0))*fsin;
                    new_x   =  fvalue1 + fvalue2 + (int)x0;

                    fvalue1 =  ((int)(x-x0))*fsin;
                    fvalue2 =  ((int)(y-y0))*fcos;
                    new_y   =  fvalue1 + fvalue2 + (int)y0;


                    in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		    continue;
		}


		/* GUI_FIRST_ROTATE */
                fvalue1 =  ((int)(x-x0))*fcos;
                fvalue2 = -((int)(y-y0))*fsin;
                new_x   =  fvalue1 + fvalue2 + (int)x0;

                fvalue1 =  ((int)(x-x0))*fsin;
                fvalue2 =  ((int)(y-y0))*fcos;
                new_y   =  fvalue1 + fvalue2 + (int)y0;


		x  = new_x;
		y  = new_y;	

		if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		{
		    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		    new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		    new_x = (x - hdc->rect.left);
		}  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		    continue;
		} else {
	            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		    new_y = y - hdc->rect.top;
		}     

		new_x += hdc->rect.left;
		new_y += hdc->rect.top;
			       
                in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		/* continue; */
		/* */
            }
        }
        #endif 

        #ifdef  _LG_TEXT_METRIC_
        lg_current_x += width + hdc->text_metric.offset_italic*height + hdc->text_metric.space_right;
        lg_current_y -= height + hdc->text_metric.offset_escapement;
        #else
        lg_current_x += width;
        lg_current_y -= height;
        #endif               
 
        pos++;
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( multi_byte_flag )
            pos++;
        #endif

        continue;

   
        MIXED_CHARSET_FONT_NEXT: 
        prop_charset_font = prop_charset_font->next;
        if ( prop_charset_font == start_mixed_charset_font )
            goto  MIXED_CHARSET_FONT_END;

        if ( prop_charset_font == 0 )
        {
            prop_charset_font = first_mixed_charset_font;
            if ( prop_charset_font == start_mixed_charset_font )
                goto  MIXED_CHARSET_FONT_END;
            
            index  = 0;
            goto  MIXED_CHARSET_FONT_START;
        }

        index = 0;
        goto  MIXED_CHARSET_FONT_START;
   
    }

    MIXED_CHARSET_FONT_END:
    *position = pos; 

    return  1;

}
#endif  /* _LG_MIXED_CHARSET_FONT_ */


#ifdef  _LG_MIXED_DISCRETE_FONT_
static  int  in_mixed_discrete_text_out(HDC hdc, const void *font, const TCHAR *str, int code_counter, unsigned int *position, GUI_SYMMETRY_ROTATE *s_r)
{
    MIXED_DISCRETE_FONT  *first_mixed_discrete_font  = (MIXED_DISCRETE_FONT *)font;
    MIXED_DISCRETE_FONT  *start_mixed_discrete_font  = (MIXED_DISCRETE_FONT *)font;
    MIXED_DISCRETE_FONT  *mixed_discrete_font        = (MIXED_DISCRETE_FONT *)font;


    unsigned int   str_counter = code_counter;

    MIXED_DISCRETE_UINT16  *puint16 = 0;
    MIXED_DISCRETE_UCHAR2  *puchar2 = 0;

    UCHAR          *pdata       = 0;

    TCHAR          *ptchar      = 0;

    unsigned int   char_counter = 0;

    #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
    unsigned char multi_byte_flag = 0;
    #endif

    unsigned int  width        = 0;
    unsigned int  height       = 0;
    unsigned int  totall_bytes = 0;
    unsigned int  index        = 0;
    unsigned int  n            = 0;
    unsigned int  pos          = 0;

    GUI_COLOR       gui_color = 0;
    SCREEN_COLOR    screen_foreground_color = 0;  
    SCREEN_COLOR    screen_back_color = 0;
    SCREEN_COLOR    screen_color = 0;

    UCHAR           tchar   = 0;
    int             itemp   = 0;
    int             x       = 0;
    int             y       = 0;  
    int             x0      = 0;
    int             y0      = 0;  
    int         new_x       = 0;
    int         new_y       = 0;  

    double      fsin        = 0;
    double      fcos        = 0;
    double      fvalue1     = 0;
    double      fvalue2     = 0;



    unsigned char   lf_flag = 0;
    unsigned char   cr_flag = 0;

             int    i = 0; 
             int    j = 0;

             int    k = 0;

             int    shift = 0;


    if ( hdc == NULL )
        return  -1;
    if ( mixed_discrete_font == NULL )
        return  -1;
    if ( s_r == NULL )
	return  -1;
    if ( position == NULL)
        return  -1;


    if ( (s_r->rotate.is_rotate) )
    {
        fsin = sin(((s_r->rotate.theta)*PI)/IMAGE_ROTATE_180);
        fcos = cos(((s_r->rotate.theta)*PI)/IMAGE_ROTATE_180);
        x0   = s_r->rotate.x + hdc->rect.left;
        y0   = s_r->rotate.y + hdc->rect.top;
    }

    gui_color = in_hdc_get_text_fore_color(hdc);
    screen_foreground_color = (lscrn.gui_to_screen_color)(gui_color);

    gui_color = in_hdc_get_text_back_color(hdc);
    screen_back_color = (lscrn.gui_to_screen_color)(gui_color);

    char_counter = mixed_discrete_font->counter;

    index     = 0;
    pos       = 0;
    *position = 0;


    for ( n = 0; n < str_counter; n++ )
    {
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        multi_byte_flag = 0;
        #endif

        start_mixed_discrete_font = mixed_discrete_font;
        ptchar = (TCHAR *)(&str[n]);
        if ( *ptchar == GUI_LF )
        {   
            if  ( cr_flag == 0 )
            {
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  = hdc->text_metric.space_left;
                lg_current_y += height + hdc->text_metric.space_top + hdc->text_metric.space_bottom;
                #else
                lg_current_x  = 0;
                lg_current_y += height;
                #endif

                lg_start_new_line = 0; 

                lf_flag = 1;
            } else {
                lf_flag = 0;
                cr_flag = 0;
            }

            pos++;
            continue; 
        }

        if ( *ptchar == GUI_CR )
        { 
            if (lf_flag == 0 )
            {  
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  = hdc->text_metric.space_left;
                lg_current_y += height + hdc->text_metric.space_top + hdc->text_metric.space_bottom;
                #else
                lg_current_x  = 0;
                lg_current_y += hdc->rect.top + height;
                #endif

                lg_start_new_line = 0; 

                cr_flag = 1;
            } else {
                cr_flag = 0;
                lf_flag = 0;
            }

            pos++;
            continue; 
        }

        if ( *ptchar == GUI_TAB )
        { 
            in_text_out_tab(pdc);
            pos++;
            continue; 
        }

        if ( *ptchar == NULL_CHAR )
        {
            pos++;
            continue; 
        }

        lf_flag = 0;
        cr_flag = 0;

        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( (unsigned char)(*ptchar) >= MULTI_BYTE_START_CODE  )
        {
            multi_byte_flag = 1;
            n++;
        } 
        #endif

        MIXED_DISCRETE_FONT_START:
        puint16  = (MIXED_DISCRETE_UINT16 *)(mixed_discrete_font->list);
        puchar2  = (MIXED_DISCRETE_UCHAR2 *)(mixed_discrete_font->list);
        for (i = 0; i < char_counter; i++)
        {          
            if ( (mixed_discrete_font->type) == MIXED_DISCRETE_UCHAR2_TYPE )
            {
                #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
                if ( multi_byte_flag )
                    itemp = strncmp((const char *)(puchar2->code), (const char *)ptchar, 2*sizeof(UCHAR));
                else
                    itemp = strncmp((const char *)(puchar2->code), (const char *)ptchar, sizeof(UCHAR));
                #else
                    itemp = strncmp((const char *)(puchar2->code), (const char *)ptchar, sizeof(UCHAR));
                #endif
            } else {
                itemp = puint16->code - *((UINT16 *)ptchar);
            }

            if ( itemp == 0 )
            {
                if ( (mixed_discrete_font->type) == MIXED_DISCRETE_UCHAR2_TYPE )
                {
                    width   = puchar2->width;
                    height  = puchar2->height;
                    pdata   = puchar2->data;
                } else {
                    width   = puint16->width;
                    height  = puint16->height;
                    pdata   = puint16->data;
                }

                totall_bytes = (width*height)/8;
                if ( ((width*height)%8) > 0 )
                    totall_bytes += 1;
   
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x  += hdc->text_metric.space_left;
                #endif

                if ( lg_start_new_line )
                {
                    #ifdef  _LG_TEXT_METRIC_
                    lg_current_y  += hdc->text_metric.space_top;
                    #endif

                    lg_start_new_line = 0; 
                }

                #ifdef  _LG_TEXT_METRIC_
                if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_TOP) == TEXT_FRAME_TOP )
                {
                    y = hdc->rect.top + lg_current_y;
                    for ( k = 0; k < width; k++ )
                    {
                        x = hdc->rect.left + lg_current_x + k;

                        /* */
		        if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		        {	
	                    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		            continue;
		        }

		        if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		        {
		            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
	                    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
	                    }     

		            new_x += hdc->rect.left;
	                    new_y += hdc->rect.top;
			       
                           in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                   continue;
		        } 
			

		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		        {
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                    continue;
		        } 
			
		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		        {
	                    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
		            } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
		            }     

		            new_x += hdc->rect.left;
		            new_y += hdc->rect.top;


		            x       = new_x;
		            y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		            continue;
		        }


		        /* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

		        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		        {
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            new_x = (x - hdc->rect.left);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
		        } else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = y - hdc->rect.top;
		        }     

		        new_x += hdc->rect.left;
		        new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		        /* continue; */
		        /* */
                    }
                }


                if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_LEFT) == TEXT_FRAME_LEFT )
                {
                    x = hdc->rect.left + lg_current_x;

                    for ( j = 0; j < height; j++ )
                    {
                        y = hdc->rect.top + lg_current_y + j;

                        /* */
		        if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		        {	
	                    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		            continue;
		        }

		        if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		        {
		            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
	                    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
	                    }     

		            new_x += hdc->rect.left;
	                    new_y += hdc->rect.top;
			       
                           in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                   continue;
		        } 
			

		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		        {
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                    continue;
		        } 
			
		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		        {
	                    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
		            } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
		            }     

		            new_x += hdc->rect.left;
		            new_y += hdc->rect.top;


		            x       = new_x;
		            y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		            continue;
		        }


		        /* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

		        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		        {
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            new_x = (x - hdc->rect.left);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
		        } else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = y - hdc->rect.top;
		        }     

		        new_x += hdc->rect.left;
		        new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		        /* continue; */
		        /* */
                    }
                }


                if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_RIGHT) == TEXT_FRAME_RIGHT )
                {
                    y = hdc->rect.top + lg_current_y;

                    for ( j = 0; j < height; j++ )
                    {
                        x = hdc->rect.left + lg_current_x + k;

                        /* */
		        if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		        {	
	                    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		            continue;
		        }

		        if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		        {
		            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
	                    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
	                    }     

		            new_x += hdc->rect.left;
	                    new_y += hdc->rect.top;
			       
                           in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                   continue;
		        } 
			

		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		        {
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                    continue;
		        } 
			
		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		        {
	                    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
		            } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
		            }     

		            new_x += hdc->rect.left;
		            new_y += hdc->rect.top;


		            x       = new_x;
		            y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		            continue;
		        }


		        /* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

		        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		        {
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            new_x = (x - hdc->rect.left);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
		        } else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = y - hdc->rect.top;
		        }     

		        new_x += hdc->rect.left;
		        new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		        /* continue; */
		        /* */
                    }
               }


               if ( hdc->text_metric.is_strike_out )
               {
                    itemp = height/2;
                    for ( k = 0; k < width; k++ )
                    {
                        x = hdc->rect.left + lg_current_x + k;
                        y = hdc->rect.top + lg_current_y +itemp;

                        /* */
		        if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		        {	
	                    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		            continue;
		        }

		        if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		        {
		            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
	                    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
	                    }     

		            new_x += hdc->rect.left;
	                    new_y += hdc->rect.top;
			       
                           in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                   continue;
		        } 
			

		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		        {
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                    continue;
		        } 
			
		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		        {
	                    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
		            } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
		            }     

		            new_x += hdc->rect.left;
		            new_y += hdc->rect.top;


		            x       = new_x;
		            y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		            continue;
		        }


		        /* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

		        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		        {
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            new_x = (x - hdc->rect.left);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
		        } else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = y - hdc->rect.top;
		        }     

		        new_x += hdc->rect.left;
		        new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		        /* continue; */
		        /* */
		    }
                }
                #endif

                for ( j = 0; j < height; j++ )
                {
                    for ( k = 0; k < width; k++ )
                    {
                        itemp   =  width*j + k;
                        index   =  itemp/8;
                        shift   =  itemp%8;
  
                        tchar  = *(pdata + index);

                        x = hdc->rect.left + lg_current_x;
                        y = hdc->rect.top + lg_current_y;



                        if ( (tchar << shift )&0x80 )
                            screen_color = screen_foreground_color; 
		        else if ( (hdc->back_mode) == MODE_COPY ) {
                            screen_color = screen_back_color; 
		        } else {
	                    lg_current_x++;  
		            continue;
		        }



	                if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		        {	
		            in_output_screen_pixel_abs(hdc,x,y,screen_color);

	                    lg_current_x++;  
	                    continue;
		        }


		        if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		        {
		            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                                lg_current_x++;  
			        continue;
		            } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
	                    }     


		            new_x += hdc->rect.left;
	                    new_y += hdc->rect.top;
			       
                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

                            lg_current_x++;  
	                    continue;
	 	        } 
				    		    
		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		        {
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

	                    lg_current_x++;  
		            continue;
		        } 
			
		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		        {
		            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                                lg_current_x++;  
		                continue;
		            } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
		            }     

		            new_x += hdc->rect.left;
		            new_y += hdc->rect.top;


	                    x       = new_x;
	                    y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

                            lg_current_x++;  
	                    continue;

		        }


		        /* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

		        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		        {
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
	                    new_x = (x - hdc->rect.left);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
                            lg_current_x++;  
		            continue;
		        } else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
	                    new_y = y - hdc->rect.top;
		        }     

		        new_x += hdc->rect.left;
		        new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_color);

		        lg_current_x++;  
		        /* continue; */
                    }

                    #ifdef  _LG_TEXT_METRIC_
                    lg_current_x -= width + hdc->text_metric.offset_italic; 
                    #else
                    lg_current_x -= width; 
                    #endif

                    lg_current_y++;
                }

                #ifdef  _LG_TEXT_METRIC_
                if ( ((hdc->text_metric.frame_style)&TEXT_FRAME_BOTTOM) == TEXT_FRAME_BOTTOM )
                {
                    y = hdc->rect.top + lg_current_y;
                    for ( k = 0; k < width; k++ )
                    {
	   	        x = hdc->rect.left + lg_current_x + k;

                        /* */
		        if (((s_r->symmetry.is_symmetry) == 0)&&( (s_r->rotate.is_rotate) == 0 ))
		        {	
	                    in_output_screen_pixel_abs(hdc,x,y,screen_foreground_color);
		            continue;
		        }

		        if ( (s_r->symmetry.is_symmetry) && ( (s_r->rotate.is_rotate) == 0 ) )
		        {
		            if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
	                    } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
	                    }     

		            new_x += hdc->rect.left;
	                    new_y += hdc->rect.top;
			       
                           in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                   continue;
		        } 
			

		        if ( (s_r->rotate.is_rotate) && ( (s_r->symmetry.is_symmetry) == 0 ) ) 
		        {
                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;

                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
	                    continue;
		        } 
			
		        if ( ((s_r->first) == GUI_FIRST_SYMMETRY) )
		        {
	                    if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		            {
		                new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		                new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
			        new_x = (x - hdc->rect.left);
		            }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
			        continue;
		            } else {
	                        new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
			        new_y = y - hdc->rect.top;
		            }     

		            new_x += hdc->rect.left;
		            new_y += hdc->rect.top;


		            x       = new_x;
		            y       = new_y;

                            fvalue1 =  ((int)(x-x0))*fcos;
                            fvalue2 = -((int)(y-y0))*fsin;
                            new_x   =  fvalue1 + fvalue2 + (int)x0;

                            fvalue1 =  ((int)(x-x0))*fsin;
                            fvalue2 =  ((int)(y-y0))*fcos;
                            new_y   =  fvalue1 + fvalue2 + (int)y0;


                            in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		            continue;
		        }


		        /* GUI_FIRST_ROTATE */
                        fvalue1 =  ((int)(x-x0))*fcos;
                        fvalue2 = -((int)(y-y0))*fsin;
                        new_x   =  fvalue1 + fvalue2 + (int)x0;

                        fvalue1 =  ((int)(x-x0))*fsin;
                        fvalue2 =  ((int)(y-y0))*fcos;
                        new_y   =  fvalue1 + fvalue2 + (int)y0;


		        x  = new_x;
		        y  = new_y;	

		        if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_POINT )
		        {
		            new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_HLINE ) {
		            new_y = (s_r->symmetry.point[0].y)*2 - (y - hdc->rect.top);
		            new_x = (x - hdc->rect.left);
		        }  else if ( (s_r->symmetry.symmetry_type) == GUI_SYMMETRY_LINE ) {  
		            continue;
		        } else {
	                    new_x = (s_r->symmetry.point[0].x)*2 - (x - hdc->rect.left);
		            new_y = y - hdc->rect.top;
		        }     

		        new_x += hdc->rect.left;
		        new_y += hdc->rect.top;
			       
                        in_output_screen_pixel_abs(hdc,new_x,new_y,screen_foreground_color);
		        /* continue; */
		        /* */
                    }
                }
                #endif
 
                #ifdef  _LG_TEXT_METRIC_
                lg_current_x += width + hdc->text_metric.offset_italic*height + hdc->text_metric.space_right;
                lg_current_y -= height + hdc->text_metric.offset_escapement;
                #else
                lg_current_x += width;
                lg_current_y -= height;
                #endif
                
                pos++;

                #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
                if ( multi_byte_flag )
                    pos++;
                #endif

                break;

            }
           
            if ( (mixed_discrete_font->type) == MIXED_DISCRETE_UCHAR2_TYPE )
                puchar2++;
            else
                puint16++;


        }

        if ( i >= char_counter ) 
        {
            mixed_discrete_font = mixed_discrete_font->next;
            if ( mixed_discrete_font == start_mixed_discrete_font )
                goto  MIXED_DISCRETE_FONT_END;

            if ( mixed_discrete_font == 0 )
            {
                mixed_discrete_font = first_mixed_discrete_font;
                if ( mixed_discrete_font == start_mixed_discrete_font )
                    goto  MIXED_DISCRETE_FONT_END;

                char_counter = mixed_discrete_font->counter;
                index = 0;

                goto  MIXED_DISCRETE_FONT_START;
            }
 
            char_counter = mixed_discrete_font->counter;

            index = 0;

            goto  MIXED_DISCRETE_FONT_START;
        }
    }

    MIXED_DISCRETE_FONT_END:
    *position = pos; 

    return  1;
}
#endif  /* _LG_MIXED_DISCRETE_FONT_ */


int  in_text_out(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter)
{
    GUI_FONT  *font          = NULL;
    GUI_FONT  *start_font    = NULL;
    TCHAR     *pstr          = NULL;  
             int  raw_len    = code_counter;
             int  cur_len    = code_counter;
             int  out_len    = 0;

    unsigned int  position   = 0;
    GUI_SYMMETRY_ROTATE  s_r = {0};



    if ( code_counter == 0 )
        return  0;


    (lscrn.output_sequence_start)();


    #ifndef  _LG_WINDOW_ 
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(((HDC)hdc)->rect.left + x, ((HDC)hdc)->rect.top + y, ((HDC)hdc)->rect.right, ((HDC)hdc)->rect.bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_ 
    if (in_init_max_output_rect(hdc) < 1)
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


    if ( code_counter < 0 )
    {
        #ifdef  _LG_UNICODE_VERSION_
            int  temp_len = 0;
            int  i = 0;

            raw_len = 0;
            temp_len = sizeof(*str)/2 + 1;
            for (i = 0; i < temp_len; i++)
            {
               if ( ((*(str+i))=='\0')&& ((*(str+i+1))=='\0'))
               {
                   raw_len = i;
                   break;
               }
            }
        #else
            raw_len = strlen(str);
        #endif

        cur_len = raw_len;
    }

    font       = ((HDC)hdc)->font;
    start_font = font;
    if ( font == 0 )
        goto  TEXT_OUT_EXIT;

    lg_start_x   = x;
    lg_start_y   = y;

    lg_current_x = lg_start_x;
    lg_current_y = lg_start_y;

    pstr = (TCHAR *)str;
    out_len = 0;

    lg_start_new_line = 1;


    TEXT_OUT_START:
    /* ?? */
    /*
    if ( ((font->type) < MIN_FONT_TYPE ) || ((font->type) > MAX_FONT_TYPE) )
        goto  TEXT_OUT_EXIT;
    */
    /* Avoid compilng warning */
    if ( (font->type) > MAX_FONT_TYPE )
        goto  TEXT_OUT_EXIT;


    position = 0;

    #ifdef  _LG_MONO_CHARSET_FONT_
    if ( (font->type) == MONO_CHARSET_FONT_TYPE )
        in_mono_charset_text_out(hdc, font->font, pstr, cur_len, &position, &s_r);
    #endif

    #ifdef  _LG_MONO_DISCRETE_FONT_
    if ( (font->type) == MONO_DISCRETE_FONT_TYPE )
        in_mono_discrete_text_out(hdc, font->font, pstr, cur_len, &position, &s_r);
    #endif

    #ifdef  _LG_MIXED_CHARSET_FONT_
    if ( (font->type) == MIXED_CHARSET_FONT_TYPE )
        in_mixed_charset_text_out(hdc, font->font, pstr, cur_len, &position, &s_r);
    #endif

    #ifdef  _LG_MIXED_DISCRETE_FONT_
    if ( (font->type) == MIXED_DISCRETE_FONT_TYPE )
        in_mixed_discrete_text_out(hdc, font->font, pstr, cur_len, &position, &s_r);
    #endif

    pstr    += position;
    out_len += position;
    cur_len  = raw_len - out_len;

    if ( out_len >= raw_len )
        goto  TEXT_OUT_EXIT;

    if ( position > 0 )
        start_font = font;
 
    font = font->next;
    if ( font == start_font )
        goto  TEXT_OUT_BLANK;

    if ( font != 0 )
        goto  TEXT_OUT_START;

    font = ((HDC)hdc)->font;
    if ( font == start_font )
        goto  TEXT_OUT_BLANK;

    goto  TEXT_OUT_START;


    TEXT_OUT_BLANK:
    in_text_out_blank(pdc);
    pstr++;

    font = ((HDC)hdc)->font;
    start_font = font;

    goto  TEXT_OUT_START;


    TEXT_OUT_EXIT:
    lg_start_new_line = 0;


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
int  text_out(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter)
{
    int  ret = 0;

    gui_lock( );
    ret = in_text_out(hdc, x, y, str, code_counter);
    gui_unlock( );

    return  ret;
}
#endif



int  in_text_rotate_special(/* HDC hdc */void *hdc,int x, int y,const TCHAR *str, int code_counter, GUI_ROTATE *rotate)
{
    GUI_SYMMETRY_ROTATE  s_r = {0};
    int  ret = 0;


    if ( hdc == NULL )
        return  -1;
    if ( str == NULL )
	return  -1;
    if ( rotate == NULL )
        return  -1;


    memset(&s_r, 0, sizeof(GUI_SYMMETRY_ROTATE));
    s_r.first = 0;
    memcpy(&(s_r.rotate), rotate, sizeof(GUI_ROTATE));

    ret = in_text_symmetry_rotate(hdc, x, y, str, code_counter, &s_r);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  text_rotate_special(/* HDC hdc */void *hdc,int x, int y,const TCHAR *str, int code_counter, GUI_ROTATE *rotate)
{
    int  ret = 0;

    gui_lock( );
    ret = in_text_rotate_special(hdc, x, y, str, code_counter, rotate);
    gui_unlock( );

    return  ret;
}
#endif


int  in_text_symmetry_special(/* HDC hdc */void *hdc,int x, int y,const TCHAR *str, int code_counter, GUI_SYMMETRY *symmetry)
{
    GUI_SYMMETRY_ROTATE  s_r = {0};
    int  ret = 0;


    if ( hdc == NULL )
        return  -1;
    if ( str == NULL )
	return  -1;
    if ( symmetry == NULL )
        return  -1;


    memset(&s_r, 0, sizeof(GUI_SYMMETRY_ROTATE));
    s_r.first = 0;
    memcpy(&(s_r.symmetry), symmetry, sizeof(GUI_SYMMETRY));

    ret = in_text_symmetry_rotate(hdc, x, y, str, code_counter, &s_r);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  text_symmetry_special(/* HDC hdc */void *hdc,int x, int y,const TCHAR *str, int code_counter, GUI_SYMMETRY *symmetry)
{
    int  ret = 0;

    gui_lock( );
    ret = in_text_symmetry_special(hdc, x, y, str, code_counter, symmetry);
    gui_unlock( );

    return  ret;
}
#endif


int  in_text_symmetry_rotate(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter, GUI_SYMMETRY_ROTATE *s_r)
{
    GUI_FONT  *font        = NULL;
    GUI_FONT  *start_font  = NULL;
    TCHAR     *pstr        = NULL;  
             int  raw_len  = code_counter;
             int  cur_len  = code_counter;
             int  out_len  = 0;

    unsigned int  position = 0;


    
    if ( code_counter == 0 )
        return  0;
    if ( s_r == NULL )
	return  -1;



    (lscrn.output_sequence_start)();


    #ifndef  _LG_WINDOW_ 
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(((HDC)hdc)->rect.left + x, ((HDC)hdc)->rect.top + y, ((HDC)hdc)->rect.right, ((HDC)hdc)->rect.bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_ 
    if (in_init_max_output_rect(hdc) < 1)
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


    if ( code_counter < 0 )
    {
        #ifdef  _LG_UNICODE_VERSION_
            int  temp_len = 0;
            int  i = 0;

            raw_len = 0;
            temp_len = sizeof(*str)/2 + 1;
            for (i = 0; i < temp_len; i++)
            {
               if ( ((*(str+i))=='\0')&& ((*(str+i+1))=='\0'))
               {
                   raw_len = i;
                   break;
               }
            }
        #else
            raw_len = strlen(str);
        #endif

        cur_len = raw_len;
    }

    font       = ((HDC)hdc)->font;
    start_font = font;
    if ( font == 0 )
        goto  TEXT_OUT_EXIT;

    lg_start_x   = x;
    lg_start_y   = y;

    lg_current_x = lg_start_x;
    lg_current_y = lg_start_y;

    pstr = (TCHAR *)str;
    out_len = 0;

    lg_start_new_line = 1;


    TEXT_OUT_START:
    /* ?? */
    /*
    if ( ((font->type) < MIN_FONT_TYPE ) || ((font->type) > MAX_FONT_TYPE) )
        goto  TEXT_OUT_EXIT;
    */
    /* Avoid compilng warning */
    if ( (font->type) > MAX_FONT_TYPE )
        goto  TEXT_OUT_EXIT;


    position = 0;

    #ifdef  _LG_MONO_CHARSET_FONT_
    if ( (font->type) == MONO_CHARSET_FONT_TYPE )
        in_mono_charset_text_out(hdc, font->font, pstr, cur_len, &position, s_r);
    #endif

    #ifdef  _LG_MONO_DISCRETE_FONT_
    if ( (font->type) == MONO_DISCRETE_FONT_TYPE )
        in_mono_discrete_text_out(hdc, font->font, pstr, cur_len, &position, s_r);
    #endif

    #ifdef  _LG_MIXED_CHARSET_FONT_
    if ( (font->type) == MIXED_CHARSET_FONT_TYPE )
        in_mixed_charset_text_out(hdc, font->font, pstr, cur_len, &position, s_r);
    #endif

    #ifdef  _LG_MIXED_DISCRETE_FONT_
    if ( (font->type) == MIXED_DISCRETE_FONT_TYPE )
        in_mixed_discrete_text_out(hdc, font->font, pstr, cur_len, &position, s_r);
    #endif

    pstr    += position;
    out_len += position;
    cur_len  = raw_len - out_len;

    if ( out_len >= raw_len )
        goto  TEXT_OUT_EXIT;

    if ( position > 0 )
        start_font = font;
 
    font = font->next;
    if ( font == start_font )
        goto  TEXT_OUT_BLANK;

    if ( font != 0 )
        goto  TEXT_OUT_START;

    font = ((HDC)hdc)->font;
    if ( font == start_font )
        goto  TEXT_OUT_BLANK;

    goto  TEXT_OUT_START;


    TEXT_OUT_BLANK:
    in_text_out_blank(pdc);
    pstr++;

    font = ((HDC)hdc)->font;
    start_font = font;

    goto  TEXT_OUT_START;


    TEXT_OUT_EXIT:
    lg_start_new_line = 0;


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
int  text_symmetry_rotate(/* HDC hdc */ void *hdc, int x, int y, const TCHAR *str, int code_counter, GUI_SYMMETRY_ROTATE *s_r)
{
    int  ret = 0;

    gui_lock( );
    ret = in_text_symmetry_rotate(hdc, x, y, str, code_counter, s_r);
    gui_unlock( );

    return  ret;
}
#endif




#ifdef  _LG_TEXT_OUT_EXTENSION_
int  in_text_out_rect(/* HDC hdc */ void *hdc, void *rect, const TCHAR *str, int  code_counter, unsigned int  format)
{
    GUI_FONT  *font          = NULL;
    GUI_FONT  *start_font    = NULL;
    TCHAR     *pstr          = NULL;  
    GUI_RECT  *p             = (GUI_RECT *)rect;
 
    unsigned int  raw_len    = code_counter;
    unsigned int  cur_len    = code_counter;
    unsigned int  out_len    = 0;

    unsigned int  position   = 0;
             int  temp1      = 0;
             int  temp2      = 0;
    GUI_SYMMETRY_ROTATE  s_r = {0};



    if ( raw_len == 0 )
        return  0;

    if ( rect == NULL )
        return  0;




    (lscrn.output_sequence_start)();

    #ifndef  _LG_WINDOW_ 
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(((HDC)hdc)->rect.left + p->left, ((HDC)hdc)->rect.top + p->top, ((HDC)hdc)->rect.left + p->right, ((HDC)hdc)->rect.top + p->bottom);
    #endif 
    #endif

    #ifdef  _LG_WINDOW_
    if (in_init_max_output_rect(hdc) < 1)
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


    if ( code_counter < 0 )
    {
        #ifdef  _LG_UNICODE_VERSION_
            int  temp_len = 0;
            int  i = 0;

            raw_len = 0;
            temp_len = sizeof(*str)/2 + 1;
            for (i = 0; i < temp_len; i++)
            {
               if ( ((*(str+i))=='\0')&& ((*(str+i+1))=='\0'))
               {
                   raw_len = i;
                   break;
               }
            }
        #else
            raw_len = strlen(str);
        #endif

        cur_len = raw_len;
    }


    font = ((HDC)hdc)->font;
    start_font = font;
    if ( font == NULL )
        goto  TEXT_OUT_EXIT;


    lg_start_x = p->left;
    lg_start_y = p->top;

    if ( (format&LG_TA_HCENTER) == LG_TA_HCENTER )
    {
        temp1 = in_hdc_get_font_width(hdc);
        if ( temp1 < 0 )
            temp1 = 0;

        temp1 *= raw_len;


        temp2  = GUI_RECTW(p);
        temp2 -= temp1;
        if (temp2 < 0 )
            temp2 = 0;

        /* ?? */
        temp2  = temp2 >> 1;
        if (temp2 < 0)
           temp2 = 0;

        lg_start_x += temp2;
    }

    if ( (format&LG_TA_VCENTER) == LG_TA_VCENTER )
    {
        temp1 = in_hdc_get_font_height(hdc);
        if ( temp1 < 0 )
            temp1 = 0;

        temp2  = GUI_RECTH(p);
        temp2 -= temp1;
        if (temp2 < 0 )
            temp2 = 0;

        /* ?? */
        temp2  = temp2 >> 1;
        if (temp2 < 0)
           temp2 = 0;

        lg_start_y += temp2;
    }


    lg_current_x = lg_start_x;
    lg_current_y = lg_start_y;
    
    pstr = (TCHAR *)str;

    lg_start_new_line = 1;


    TEXT_OUT_START:
    /* ?? */
    /*
    if ( (font->type < MIN_FONT_TYPE ) || (font->type > MAX_FONT_TYPE) )
        goto  TEXT_OUT_EXIT;
    */
    /* Avoid compiling warns */
    if ( (font->type) > MAX_FONT_TYPE )
        goto  TEXT_OUT_EXIT;


    position = 0;

    #ifdef  _LG_MONO_CHARSET_FONT_
    if ( (font->type) == MONO_CHARSET_FONT_TYPE )
        in_mono_charset_text_out(hdc, font->font, pstr, cur_len, &position, &s_r);
    #endif

    #ifdef  _LG_MONO_DISCRETE_FONT_
    if ( (font->type) == MONO_DISCRETE_FONT_TYPE )
        in_mono_discrete_text_out(hdc, font->font, pstr, cur_len, &position, &s_r);
    #endif

    #ifdef  _LG_MIXED_CHARSET_FONT_
    if ( (font->type) == MIXED_CHARSET_FONT_TYPE )
        in_mixed_charset_text_out(hdc, font->font, pstr, cur_len, &position, &s_r);
    #endif

    #ifdef  _LG_MIXED_DISCRETE_FONT_
    if ( (font->type) == MIXED_DISCRETE_FONT_TYPE )
        in_mixed_discrete_text_out(hdc, font->font, pstr, cur_len, &position, &s_r);
    #endif

    pstr    += position;
    out_len += position;
    cur_len = raw_len - out_len;

    if ( out_len >= raw_len )
        goto  TEXT_OUT_EXIT;


    if ( position > 0 )
        start_font = font;
 
    font = font->next;
    if ( font == start_font )
        goto  TEXT_OUT_BLANK;

    if ( font != 0 )
        goto  TEXT_OUT_START;

    font = ((HDC)hdc)->font;
    if ( font == start_font )
        goto  TEXT_OUT_BLANK;

    goto  TEXT_OUT_START;


    TEXT_OUT_BLANK:
    in_text_out_blank(hdc);
    pstr++;

    font = ((HDC)hdc)->font;
    start_font = font;

    goto  TEXT_OUT_START;


    TEXT_OUT_EXIT:
    lg_start_new_line = 0;

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
int  text_out_rect(/* HDC hdc */ void *hdc, void *rect, const TCHAR *str, int  code_counter, unsigned int  format)
{
    int  ret = 0;

    gui_lock( );
    ret = in_text_out_rect(hdc, rect, str, code_counter, format);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_TEXT_OUT_EXTENSION_ */

#endif  /* _LG_FONT_ */
