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

#include  <d2_pixel.h>

#include  <screen.h>
#include  <image_gif.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif


#define   GLOBAL_PALETTE                            0x00
#define   LOCAL_PALETTE                             0x01


#ifdef  _LG_GIF_ 
static  int  in_gif_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int frame_id, unsigned int flag)
{
    static  unsigned char last_need_back = 0;
    static  int           last_x         = 0;
    static  int           last_y         = 0;
    static  unsigned int  last_width     = 0;
    static  unsigned int  last_height    = 0;


    const GUI_GIF                   *pgif = (GUI_GIF *)gui_gif;

    const GIF_LOGICAL_SCREEN        *pscreen   = NULL;
    const GUI_PALETTE               *ppalette  = NULL;
    const GIF_BLOCK                 *pblock    = NULL;

    const GIF_PLAIN_TEXT_EXTENSION  *ptext     = NULL;
    const GIF_IMAGE_DESCRIPTOR      *pimage    = NULL;
    const GIF_CONTROL_EXTENSION     *pcontrol  = NULL;

    int                   real_x        = 0;
    int                   real_y        = 0;

    unsigned int          raw_width     = 0;
    unsigned int          raw_height    = 0;
    unsigned int          offset        = 0;
    unsigned char         bit           = 0;
    unsigned char         data          = 0;
    unsigned char         index         = 0;
    unsigned char         color_bits    = 0;

    unsigned char         trans_flag    = 0;
    unsigned char         trans_index   = 0;

    unsigned char         chtemp        = 0;

    unsigned char         block_flag    = GIF_IMAGE_TYPE; 
    unsigned char         palette_flag  = GLOBAL_PALETTE;

    unsigned char         back_flag     = 0;
    unsigned char         render_flag   = 0;

    SCREEN_COLOR          screen_color  = 0;

    GUI_COLOR             color         = 0;

    unsigned int          i       = 0;
    unsigned int          j       = 0;



    if ( pgif == NULL )
        return  -1;

    if ( hdc == NULL )
        return  -1;
        


    (lscrn.output_sequence_start)();
 
    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_restore_back_abs(hdc->rect.left + x, hdc->rect.top + y, hdc->rect.right, hdc->rect.bottom);
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
    #endif  /* _LG_WINDOW_ */


    pscreen = pgif->screen;
    if ( pscreen == NULL )
    {
        (lscrn.output_sequence_end)();
        return  -1;
    }

    ppalette = pgif->global_palette;
    if ( ppalette == NULL )
    {
        (lscrn.output_sequence_end)();
        return  -1;
    }

    if ( frame_id > (pgif->block_num - 1) )
    {
        (lscrn.output_sequence_end)();
        return  -1;
    }

    pblock        = (pgif->block_list)+frame_id;

    if ( (pblock->type) == GIF_TEXT_TYPE )
    {
        ptext       = (GIF_PLAIN_TEXT_EXTENSION *)(pblock->block);

        real_x      = x + ptext->text_left;
        real_y      = y + ptext->text_top;
        raw_width   = ptext->text_width;
        raw_height  = ptext->text_height;
        block_flag  = GIF_TEXT_TYPE;

    } else {
        pimage      = (GIF_IMAGE_DESCRIPTOR *)(pblock->block);

        real_x      = x + pimage->left;
        real_y      = y + pimage->top;
        raw_width   = pimage->width;
        raw_height  = pimage->height;
        block_flag  = GIF_IMAGE_TYPE;
        if ( ((pimage->packed_fields)&0x80) == 0x80 )
        {
            palette_flag = LOCAL_PALETTE;
        } else {
            palette_flag = GLOBAL_PALETTE;
        }
    }

    /* ?? */
    pcontrol = pblock->control;

    color_bits = (((pscreen->packed_fields)&0x70) >> 4) + 1;

    if ( block_flag == GIF_TEXT_TYPE )
    {
        (lscrn.output_sequence_end)();
        return  -1;
    }

    if ( pcontrol != 0 )
    {
        if ( (pcontrol->packed_fields)&0x01 )
        {
            trans_flag = 1;
        }

        if ( trans_flag == 1 )
        {
            trans_index = pcontrol->trans_color_index;
        }

        chtemp = (pcontrol->packed_fields)&0x1C;
        chtemp = (chtemp>>2)&0x07;

    }

    if ( frame_id == 0 )
        flag = 1;

    if ( flag )
    {
        last_x         = real_x;
        last_y         = real_y;
        last_width     = raw_width;
        last_height    = raw_height;
        last_need_back = 1;
    }

    for ( j = 0; j < (pscreen->height); j++ )
    {
        for ( i = 0; i < (pscreen->width); i++ )
        {
            back_flag   = 0;
            render_flag = 0;

            if ( last_need_back )
            {
                if ( (i >= last_x) && (i < (last_x+last_width)) && (j >= last_y) && (j < (last_y+last_height)) )
                    back_flag = 1;
            }

            if ( (i >= (pimage->left)) && (i < ((pimage->left)+raw_width)) && (j >= (pimage->top)) && (j < ((pimage->top)+raw_height)) )
                render_flag = 1;

            if ( flag )
                back_flag = 1;

            if ( (chtemp == 2) || (chtemp == 3) )
               back_flag = 1;

            if ( render_flag  )
            {
                switch ( color_bits )
                {
                   #ifdef  _LG_1_BIT_GIF_
                    case  1:
                        offset = (j*raw_width+i)/8;
                        bit    = (i%8);

                        data   = *((pimage->data)+offset);
                        data   = (data<<bit)&0x80;
                        if ( data == 0x80 )
                            index = 1;
                        else
                            index = 0;

                        break;
                   #endif  /* _LG_1_BIT_GIF_ */

                   #ifdef  _LG_2_BIT_GIF_
                    case  2:
                        break;
                   #endif  /* _LG_2_BIT_GIF_ */

                   #ifdef  _LG_4_BIT_GIF_
                    case  4:
                        offset = (j*raw_width+i)/2;
                        bit    = (i%2);

                        index  = *((pimage->data)+offset);
                        if ( bit > 0 )
                            index &= 0x0F;
                        else
                            index = (index>>4)&0x0F;

                        break;
                   #endif  /* _LG_4_BIT_GIF_ */

                   #ifdef  _LG_8_BIT_GIF_
                    case  7:
                    case  8:
                        offset = (j-(pimage->top))*raw_width + (i-(pimage->left));
                        index  = *((pimage->data)+offset);
                        break;
                   #endif  /* _LG_8_BIT_GIF_ */

                    default:
                        (lscrn.output_sequence_end)();
                        return  1;

                }

                if ( (trans_flag == 1) && (index == trans_index) )
                { 
                    if ( back_flag )
                    {
                        index = pscreen->back_color_index;
                        color = ppalette->entries[index];
                    } else {
                        continue;
                    }
                }

                if ( palette_flag == LOCAL_PALETTE )
                {
                    if ( (pimage->local_palette) == 0 )
                    {
                        (lscrn.output_sequence_end)();
                        return  -1;
                    }

                    color  = pimage->local_palette->entries[index];
                } else
                    color  = ppalette->entries[index];
                        
           
                /* ?? 20170223 */ 
                screen_color = (lscrn.gui_to_screen_color)(color);
                in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);

           } else {
                if ( back_flag )
                {
                    index = pscreen->back_color_index;
                    color = ppalette->entries[index];

                    /* ?? 20170223 */ 
                    screen_color = (lscrn.gui_to_screen_color)(color);
                    in_output_screen_pixel_abs(hdc, x+i+hdc->rect.left, y+j+hdc->rect.top, screen_color);
                }
            }
        }
    }


    if ( (chtemp == 2) || ( chtemp == 3) ) 
        last_need_back = 1;
    else
        last_need_back = 0;


    last_x      = real_x;
    last_y      = real_y;
    last_width  = raw_width;
    last_height = raw_height;
  
    #ifdef  _LG_WINDOW_
    }
    #endif  /* _LG_WINDOW_ */

    #ifndef  _LG_WINDOW_
    #ifdef  _LG_CURSOR_
    in_cursor_maybe_refresh( );
    #endif
    #endif

    (lscrn.output_sequence_end)();
 
    return  1;
}

int  in_gif_auto_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int *frame_id)
{
    const GUI_GIF         *pgif = (GUI_GIF *)gui_gif;

    unsigned int   *id = 0;
             int   ret = 0;


    if ( gui_gif == 0 )
        return  -1;

    id = frame_id;
    if ( *id > (pgif->block_num - 1) )
        *id = 0;

    ret = in_gif_play(hdc, x, y, gui_gif, *id, 0);
    (*id)++;

    return  ret;
}
    
#ifndef  _LG_ALONE_VERSION_
int  gif_auto_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int *frame_id)
{         
    int   ret = 0;

    gui_lock( );
    ret = in_gif_auto_play(hdc, x, y, gui_gif, frame_id);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */


int  in_gif_man_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int frame_id)
{
    int   ret = 0;


    if ( gui_gif == 0 )
        return  -1;

    ret = in_gif_play(hdc, x, y, gui_gif, frame_id, 1);

    return  ret;
}

#ifndef  _LG_ALONE_VERSION_
int  gif_man_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int frame_id)
{
    int   ret = 0;

    if ( gui_gif == 0 )
        return  -1;

    gui_lock( );
    ret = in_gif_man_play(hdc, x, y, gui_gif, frame_id);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_GIF_ */
