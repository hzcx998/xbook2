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

#ifndef  __LGUI_IMAGE_GIF_HEADER__
#define  __LGUI_IMAGE_GIF_HEADER__

#include  <type_color.h>

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

#include  <type_gui.h>


#include  <palette.h>


#define  GIF_IMAGE_TYPE                0x00
#define  GIF_TEXT_TYPE                 0x01

#define  GIF_UNCOMPRESS_DATA           0x00
#define  GIF_COMPRESS_DATA             0x01


struct  _GIF_LOGICAL_SCREEN 
{
    UINT16   width;
    UINT16   height;
    UINT8    packed_fields;
    UINT8    back_color_index;
    UINT8    pixel_aspect_ration;
};
typedef  struct  _GIF_LOGICAL_SCREEN   GIF_LOGICAL_SCREEN;


struct  _GIF_CONTROL_EXTENSION 
{
    UINT8    packed_fields;
    UINT16   delay_time;
    UINT8    trans_color_index;
};
typedef  struct  _GIF_CONTROL_EXTENSION   GIF_CONTROL_EXTENSION;


struct  _GIF_IMAGE_DESCRIPTOR 
{
    UINT16         left;
    UINT16         top;
    UINT16         width;
    UINT16         height;
    UINT8          packed_fields;
    GUI_PALETTE    *local_palette;
    UINT8          data_type;
    UINT8          lzw_code_size;
    UINT           len;   
    unsigned char  *data;
};
typedef  struct  _GIF_IMAGE_DESCRIPTOR   GIF_IMAGE_DESCRIPTOR;


struct  _GIF_PLAIN_TEXT_EXTENSION 
{
    UINT16         text_left;
    UINT16         text_top;
    UINT16         text_width;
    UINT16         text_height;
    UINT8          cell_width;
    UINT8          cell_height;
    UINT8          fore_color_index;
    UINT8          back_color_index;
    UINT           len;
    unsigned char  *text;
};
typedef  struct  _GIF_PLAIN_TEXT_EXTENSION   GIF_PLAIN_TEXT_EXTENSION;


struct  _GIF_BLOCK 
{
          unsigned char           type;
    const GIF_CONTROL_EXTENSION  *control;
    const void                   *block;
};
typedef  struct  _GIF_BLOCK   GIF_BLOCK;


struct  _GUI_GIF 
{
    const GIF_LOGICAL_SCREEN  *screen;	
    const GUI_PALETTE         *global_palette;
          UINT                 block_num;
    const GIF_BLOCK           *block_list;
};
typedef  struct  _GUI_GIF   GUI_GIF;


#ifdef  _LG_GIF_

#ifdef  __cplusplus
extern  "C"
{
#endif

    int  in_gif_auto_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int *frame_id);
    int  in_gif_man_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int frame_id);

    #ifndef  _LG_ALONE_VERSION_
    int  gif_auto_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int *frame_id);
    int  gif_man_play(HDC hdc, int x, int y, const void *gui_gif, unsigned int frame_id);
    #else
    #define  gif_auto_play(hdc, x, y, gui_gif, frame_id)    in_gif_auto_play(hdc, x, y, gui_gif, frame_id)
    #define  gif_man_play(hdc, x, y, gui_gif, frame_id)     in_gif_man_paly(hdc, x, y, gui_gif, frame_id)
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_GIF_ */

#endif  /* __LGUI_IMAGE_GIF_HEADER__ */
