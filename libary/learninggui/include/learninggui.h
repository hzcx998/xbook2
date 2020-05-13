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

#ifndef  __LGUI_ALL_APP_INCLUDED_HEADER__
#define  __LGUI_ALL_APP_INCLUDED_HEADER__

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


#include  <version.h>
#include  <error_code.h>

#include  <lgconst.h>


#include  <color_match.h>
#include  <palette.h>  
#ifdef  _LG_PALETTE_CONVERSION_
#include  <palette_conversion.h>
#endif        


#include  <lock.h>

#include  <color_match.h>


#ifdef  _LG_SCREEN_
#include  <screen.h>
#ifdef  _LG_SNAPSHOT_
#include  <snapshot.h>
#endif
#endif

#ifdef  _LG_PEN_
#include  <pen.h>
#endif

#ifdef  _LG_BRUSH_
#include  <brush.h>
#endif

#ifdef  _LG_FONT_
#include  <font_ops.h>
#include  <charsets.h>
#endif

#include  <text_ops.h>
#ifdef  _LG_TEXT_GLYPH_
#include  <text_glyph.h>
#endif

#include  <default.h>

#ifdef  _LG_DC_
#include  <dc.h>
#endif

#ifdef  _LG_2D_
#include  <d2_pixel.h>
#include  <d2_line.h>
#include  <d2_rect.h>
#include  <d2_triangle.h>
#include  <d2_circle.h>
#include  <d2_ellipse.h>
#endif

#include  <image_comm.h>
#ifdef  _LG_BITMAP_
#include  <image_bitmap.h>
#endif
#ifdef  _LG_ICON_
#include  <image_icon.h>
#endif
#ifdef  _LG_GIF_
#include  <image_gif.h>
#endif

#ifdef  _LG_MESSAGE_
#include  <message.h>
#endif

#ifdef  _LG_COUNTER_
#include  <counter.h>
#endif

#ifdef  _LG_TIMER_
#include  <timer.h>
#endif

#include  <keyboard.h>

#include  <mtjt.h>

#ifdef  _LG_CURSOR_
#include  <cursor.h>
#endif


/* Notice: Maybe declare window data type in basic version */
#ifdef  _LG_WINDOW_
#include  <win_type_widget.h>
#include  <win_desktop.h>
#include  <win_default.h>
#include  <win_dc.h>
#include  <win_invalidate.h>
#include  <win_interface.h>
#include  <win_item_data.h>
#include  <win_input_widget.h>
#include  <win_tools.h>
#endif

#ifdef _LG_FRAME_WIDGET_
#include  <win_frame.h>
#endif

#ifdef _LG_GROUP_BOX_WIDGET_
#include  <win_group_box.h>
#endif

#ifdef _LG_CELL_WIDGET_
#include  <win_cell.h>
#endif

#ifdef _LG_LABEL_WIDGET_
#include  <win_label.h>
#endif

#ifdef _LG_PUSH_BUTTON_WIDGET_
#include  <win_push_button.h>
#endif

#ifdef _LG_WIDGET_GROUP_
#include  <win_widget_group.h>
#endif

#ifdef _LG_RADIO_BUTTON_WIDGET_
#include  <win_radio_button.h>
#endif

#ifdef _LG_CHECK_BOX_WIDGET_
#include  <win_check_box.h>
#endif

#ifdef _LG_LINE_EDIT_WIDGET_
#include  <win_line_edit.h>
#endif

#ifdef _LG_LIST_BOX_WIDGET_
#include  <win_list_box.h>
#endif

#ifdef _LG_COM_BOX_WIDGET_
#include  <win_com_box.h>
#endif

#ifdef _LG_PROGRESS_BAR_WIDGET_
#include  <win_progress_bar.h>
#endif

#ifdef _LG_SLIDER_BAR_WIDGET_
#include  <win_slider_bar.h>
#endif

#ifdef _LG_IMAGE_WIDGET_
#include  <win_image.h>
#endif

#include  <driver.h>

#include  <code_tools.h>

#include  <file_ops.h>


#ifdef  __cplusplus
extern  "C"
{
#endif

    int  gui_open(void); 
    int  gui_close(void); 

#ifdef  __cplusplus
}
#endif

#endif  /* __LGUI_ALL_APP_INCLUDED_HEADER__ */
