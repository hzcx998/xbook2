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

#ifndef  __LGUI_INPUT_WIDGET_HEADER__
#define  __LGUI_INPUT_WIDGET_HEADER__

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



#ifdef  _LG_WINDOW_
#include  <win_type_widget.h>
#endif

#ifdef  _LG_FRAME_WIDGET_
#include  <win_frame.h>
#endif

#ifdef  _LG_GROUP_BOX_WIDGET_
#include  <win_group_box.h>
#endif

#ifdef  _LG_CELL_WIDGET_
#include  <win_cell.h>
#endif

#ifdef  _LG_LABEL_WIDGET_
#include  <win_label.h>
#endif

#ifdef  _LG_PUSH_BUTTON_WIDGET_
#include  <win_push_button.h>
#endif

#ifdef  _LG_RADIO_BUTTON_WIDGET_
#include  <win_radio_button.h>
#endif

#ifdef  _LG_CHECK_BOX_WIDGET_
#include  <win_check_box.h>
#endif

#ifdef  _LG_LINE_EDIT_WIDGET_
#include  <win_line_edit.h>
#endif

#ifdef  _LG_LIST_BOX_WIDGET_
#include  <win_list_box.h>
#endif

#ifdef  _LG_COM_BOX_WIDGET_
#include  <win_com_box.h>
#endif

#ifdef  _LG_PROGRESS_BAR_WIDGET_
#include  <win_progress_bar.h>
#endif

#ifdef  _LG_SLIDER_BAR_WIDGET_
#include  <win_slider_bar.h>
#endif

#ifdef  _LG_IMAGE_WIDGET_
#include  <win_image.h>
#endif



#ifdef  _LG_WINDOW_

/* WIDGET_UNION */
union  _WIDGET_UNION
{
    #ifdef  _LG_FRAME_WIDGET_
    GUI_FRAME         frame;
    #endif

    #ifdef  _LG_GROUP_BOX_WIDGET_
    GUI_GROUP_BOX     group_box;
    #endif

    #ifdef  _LG_CELL_WIDGET_
    GUI_CELL          cell;
    #endif

    #ifdef  _LG_LABEL_WIDGET_
    GUI_LABEL         label;
    #endif

    #ifdef  _LG_PUSH_BUTTON_WIDGET_
    GUI_PUSH_BUTTON   push_button;
    #endif

    #ifdef  _LG_RADIO_BUTTON_WIDGET_
    GUI_RADIO_BUTTON  radio_button;
    #endif

    #ifdef  _LG_CHECK_BOX_WIDGET_
    GUI_CHECK_BOX     check_box;
    #endif

    #ifdef  _LG_LINE_EDIT_WIDGET_
    GUI_LINE_EDIT     line_edit;
    #endif

    #ifdef  _LG_LIST_BOX_WIDGET_
    GUI_LIST_BOX      list_box;
    #endif

    #ifdef  _LG_COM_BOX_WIDGET_
    GUI_COM_BOX      com_box;
    #endif

    #ifdef  _LG_PROGRESS_BAR_WIDGET_
    GUI_PROGRESS_BAR progress_bar;
    #endif

    #ifdef  _LG_SLIDER_BAR_WIDGET_
    GUI_SLIDER_BAR   slider_bar;
    #endif

    #ifdef  _LG_IMAGE_WIDGET_
    GUI_IMAGE        image;
    #endif
};
typedef	union	_WIDGET_UNION  WIDGET_UNION;

#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_INPUT_WIDGET_HEADER__ */
