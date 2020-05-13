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

#include  <lock.h>

#include  <color_match.h>
#include  <dc.h>
#include  <screen.h>

#include  <default.h>


#ifdef  _LG_WINDOW_
#include  <win_type_widget.h>
#include  <win_clip.h>
#include  <win_desktop.h>
#endif

   
int   in_hdc_basic_set_default(HDC hdc)
{
    if ( hdc == NULL )
        return  -1;


    memset(hdc, 0, sizeof(GUI_DC));

    hdc->type = SYSTEM_DC_TYPE;
    hdc->used = GUI_UNUSED;

    hdc->back_mode = MODE_TRANSPARENCY;

    hdc->color[DISABLED_GROUP][BACK_ROLE]      = GUI_DEFAULT_BCOLOR;
    hdc->color[DISABLED_GROUP][FORE_ROLE]      = GUI_DEFAULT_FCOLOR;
    hdc->color[DISABLED_GROUP][TEXT_BACK_ROLE] = GUI_DEFAULT_TEXT_BCOLOR;
    hdc->color[DISABLED_GROUP][TEXT_FORE_ROLE] = GUI_DEFAULT_TEXT_FCOLOR;

    hdc->color[INACTIVE_GROUP][BACK_ROLE]      = GUI_DEFAULT_BCOLOR;
    hdc->color[INACTIVE_GROUP][FORE_ROLE]      = GUI_DEFAULT_FCOLOR;
    hdc->color[INACTIVE_GROUP][TEXT_BACK_ROLE] = GUI_DEFAULT_TEXT_BCOLOR;
    hdc->color[INACTIVE_GROUP][TEXT_FORE_ROLE] = GUI_DEFAULT_TEXT_FCOLOR;

    hdc->color[ACTIVE_GROUP][BACK_ROLE]        = GUI_DEFAULT_BCOLOR;
    hdc->color[ACTIVE_GROUP][FORE_ROLE]        = GUI_DEFAULT_FCOLOR;
    hdc->color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = GUI_DEFAULT_TEXT_BCOLOR;
    hdc->color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = GUI_DEFAULT_TEXT_FCOLOR;

    hdc->cur_group = INACTIVE_GROUP;

    #ifdef  _LG_ALPHA_BLEND_
    hdc->is_alpha_blend      = 0;
    hdc->alpha_blend_op_mode = ALPHA_BLEND_OP_ADD;
    #endif

    #ifdef  _LG_PEN_
    hdc->pen.type  = SOLID_PEN_TYPE;
    hdc->pen.width = 1;
    hdc->pen.cur_pos.x = 0;
    hdc->pen.cur_pos.y = 0;
    #endif

    #ifdef  _LG_BRUSH_
    hdc->brush.brush_type  = SOLID_BRUSH_TYPE;
    hdc->brush.hatch_type  = HORIZONTAL_HATCH_TYPE;

    hdc->brush.cur_pos.x   = 0;
    hdc->brush.cur_pos.y   = 0;
    #endif

    #ifdef  _LG_FONT_
    hdc->font = (GUI_FONT *)lbdcft;
    #ifdef  _LG_TEXT_METRIC_
    hdc->text_metric.space_left        = 0;
    hdc->text_metric.space_top         = 0;
    hdc->text_metric.space_right       = 0;
    hdc->text_metric.space_bottom      = 0;

    hdc->text_metric.is_strike_out     = 0;
    hdc->text_metric.frame_style       = 0;
    hdc->text_metric.offset_italic     = 0;
    hdc->text_metric.offset_escapement = 0;
    #endif
    #endif

    hdc->rect.left   = 0;
    hdc->rect.top    = 0;
    hdc->rect.right  = (lscrn.width) - 1; 
    hdc->rect.bottom = (lscrn.height) - 1;
 
    #ifdef  _LG_WINDOW_
    hdc->hwnd = HWND_DESKTOP;
    #endif
   
    return  1;
}

#ifdef  _LG_WINDOW_
int   in_hdc_window_set_default(HDC hdc)
{
    if ( hdc == NULL )
        return  -1;

    hdc->color[DISABLED_GROUP][BACK_ROLE]      = WIN_DISABLED_BCOLOR;
    hdc->color[DISABLED_GROUP][FORE_ROLE]      = WIN_DISABLED_FCOLOR;
    hdc->color[DISABLED_GROUP][TEXT_BACK_ROLE] = WIN_DISABLED_TBCOLOR;
    hdc->color[DISABLED_GROUP][TEXT_FORE_ROLE] = WIN_DISABLED_TFCOLOR;

    hdc->color[INACTIVE_GROUP][BACK_ROLE]      = WIN_INACTIVE_BCOLOR;
    hdc->color[INACTIVE_GROUP][FORE_ROLE]      = WIN_INACTIVE_FCOLOR;
    hdc->color[INACTIVE_GROUP][TEXT_BACK_ROLE] = WIN_INACTIVE_TBCOLOR;
    hdc->color[INACTIVE_GROUP][TEXT_FORE_ROLE] = WIN_INACTIVE_TFCOLOR;

    hdc->color[ACTIVE_GROUP][BACK_ROLE]        = WIN_ACTIVE_BCOLOR;
    hdc->color[ACTIVE_GROUP][FORE_ROLE]        = WIN_ACTIVE_FCOLOR;
    hdc->color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = WIN_ACTIVE_TBCOLOR;
    hdc->color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = WIN_ACTIVE_TFCOLOR;

    return  1;
}

int   in_hdc_client_set_default(HDC hdc)
{
    if ( hdc == NULL )
        return  -1;

    hdc->color[DISABLED_GROUP][BACK_ROLE]      = CLI_DISABLED_BCOLOR;
    hdc->color[DISABLED_GROUP][FORE_ROLE]      = CLI_DISABLED_FCOLOR;
    hdc->color[DISABLED_GROUP][TEXT_BACK_ROLE] = CLI_DISABLED_TBCOLOR;
    hdc->color[DISABLED_GROUP][TEXT_FORE_ROLE] = CLI_DISABLED_TFCOLOR;

    hdc->color[INACTIVE_GROUP][BACK_ROLE]      = CLI_INACTIVE_BCOLOR;
    hdc->color[INACTIVE_GROUP][FORE_ROLE]      = CLI_INACTIVE_FCOLOR;
    hdc->color[INACTIVE_GROUP][TEXT_BACK_ROLE] = CLI_INACTIVE_TBCOLOR;
    hdc->color[INACTIVE_GROUP][TEXT_FORE_ROLE] = CLI_INACTIVE_TFCOLOR;

    hdc->color[ACTIVE_GROUP][BACK_ROLE]        = CLI_ACTIVE_BCOLOR;
    hdc->color[ACTIVE_GROUP][FORE_ROLE]        = CLI_ACTIVE_FCOLOR;
    hdc->color[ACTIVE_GROUP][TEXT_BACK_ROLE]   = CLI_ACTIVE_TBCOLOR;
    hdc->color[ACTIVE_GROUP][TEXT_FORE_ROLE]   = CLI_ACTIVE_TFCOLOR;

    return  1;
}
#endif
 
int   in_hdc_basic_init(void)
{
    int  i = 0;

    memset((void *)lbasdc, 0, sizeof(GUI_DC));
    for ( i = 0; i < MAX_DC_NUM; i++ )
        in_hdc_basic_set_default((HDC)(&lbasdc[i]));

    /* adjust for DESKTOP_DC */
    lbasdc[0].type = DESKTOP_DC_TYPE;

    return  1;
}

int   in_basic_set_default_font(const void *font)
{
    int  i = 0;

    if (font == NULL )
        return  -1;
    
    lbdcft = (GUI_FONT *)font;

    for ( i = 0; i < MAX_DC_NUM; i++ )
    {
        if (lbasdc[i].used == GUI_UNUSED )
            lbasdc[i].font = (GUI_FONT *)lbdcft;
    }

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int   basic_set_default_font(const void *font)
{
    int  ret = 0;

    gui_lock( );
    ret = in_basic_set_default_font(font);
    gui_unlock( );

    return  ret;
}
#endif
