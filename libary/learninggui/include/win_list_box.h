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

#ifndef  __LGUI_WIN_LIST_BOX_WIDGET_HEADER__
#define  __LGUI_WIN_LIST_BOX_WIDGET_HEADER__    1

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
#include  <win_item_data.h>
#endif


/* ListBox min row height */
#ifndef  MIN_LIST_BOX_ROW_HEIGHT
#define  MIN_LIST_BOX_ROW_HEIGHT               16
#endif
#if  (MIN_LIST_BOX_ROW_HEIGHT < 8)
#undef   MIN_LIST_BOX_ROW_HEIGHT
#define  MIN_LIST_BOX_ROW_HEIGHT               8
#endif

/* ListBox min char width */
#ifndef  MIN_LIST_BOX_CHAR_WIDTH
#define  MIN_LIST_BOX_CHAR_WIDTH               8
#endif
#if  (MIN_LIST_BOX_CHAR_WIDTH < 6)
#undef   MIN_LIST_BOX_CHAR_WIDTH
#define  MIN_LIST_BOX_CHAR_WIDTH               6
#endif

 
/* ListBox window color */
#ifndef  LBOX_WIN_DISABLED_BCOLOR
#define  LBOX_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LBOX_WIN_DISABLED_FCOLOR
#define  LBOX_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  LBOX_WIN_INACTIVE_BCOLOR
#define  LBOX_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LBOX_WIN_INACTIVE_FCOLOR
#define  LBOX_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  LBOX_WIN_ACTIVE_BCOLOR
#define  LBOX_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  LBOX_WIN_ACTIVE_FCOLOR
#define  LBOX_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif

/* ListBox client color */
#ifndef  LBOX_CLI_DISABLED_BCOLOR
#define  LBOX_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  LBOX_CLI_DISABLED_FCOLOR
#define  LBOX_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  LBOX_CLI_DISABLED_TBCOLOR
#define  LBOX_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  LBOX_CLI_DISABLED_TFCOLOR
#define  LBOX_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  LBOX_CLI_INACTIVE_BCOLOR
#define  LBOX_CLI_INACTIVE_BCOLOR              GUI_LIGHT_WHITE
#endif
#ifndef  LBOX_CLI_INACTIVE_FCOLOR
#define  LBOX_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  LBOX_CLI_INACTIVE_TBCOLOR
#define  LBOX_CLI_INACTIVE_TBCOLOR             GUI_WHITE
#endif
#ifndef  LBOX_CLI_INACTIVE_TFCOLOR
#define  LBOX_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif

#ifndef  LBOX_CLI_ACTIVE_BCOLOR
#define  LBOX_CLI_ACTIVE_BCOLOR                GUI_WHITE
#endif
#ifndef  LBOX_CLI_ACTIVE_FCOLOR
#define  LBOX_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  LBOX_CLI_ACTIVE_TBCOLOR
#define  LBOX_CLI_ACTIVE_TBCOLOR               GUI_WHITE
#endif
#ifndef  LBOX_CLI_ACTIVE_TFCOLOR
#define  LBOX_CLI_ACTIVE_TFCOLOR               GUI_BLACK
#endif


#ifndef  MAX_LIST_BOX_TEXT_LEN
#define  MAX_LIST_BOX_TEXT_LEN                 255
#endif
#if  (MAX_LIST_BOX_TEXT_LEN < 1)
#undef   MAX_LIST_BOX_TEXT_LEN
#define  MAX_LIST_BOX_TEXT_LEN                 255
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_LIST_BOX_WIDGET_


/* GUI_LIST_BOX */
struct  _GUI_LIST_BOX
{
    /* ListBox multi-select flag */  
    unsigned int     multi_flag;
};
typedef	struct	_GUI_LIST_BOX  GUI_LIST_BOX;


/* IN_GUI_LIST_BOX */
struct  _IN_GUI_LIST_BOX
{
    /* ListBox list rect */
    GUI_RECT         list_rect;

    /* ListBox item */	
    ITEM_DATA_LIST  *pitem;

    /* ListBox counter */
    unsigned int     counter;  

    /* ListBox max_text_len */
    unsigned int     max_text_len;  

    /* ListBox multi-select flag */  
    unsigned int     multi_flag;

    /* ListBox single-select id */  
             int     selected_index;

    /* ListBox high-lighted id */  
             int     lighted_index;

    /* ListBox x step */	
             int     x_step;

    /* ListBox y step */	
             int     y_step;

    /* ListBox x multi-step */	
    unsigned int     x_multi_step;

    /* ListBox y multi-step */	
    unsigned int     y_multi_step;

    /* ListBox read-only */	
    unsigned int     read_only;
};
typedef	struct	_IN_GUI_LIST_BOX  IN_GUI_LIST_BOX;

#define  GET_IN_GUI_LIST_BOX(hwnd)           ((IN_GUI_LIST_BOX *)(((HWND)hwnd)->ext))





#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_list_box_create(HWND parent, void *gui_common_widget, void *gui_list_box);

    int   in_list_box_insert_item(HWND hwnd, int index, TCHAR *text, unsigned int text_len);
    int   in_list_box_get_item(HWND hwnd, unsigned int index, TCHAR *text, unsigned int *text_len);
    int   in_list_box_delete_item(HWND hwnd, unsigned int index);

    int   in_list_box_set_select_mode(HWND hwnd, unsigned int  mode);
    int   in_list_box_get_select_mode(HWND hwnd, unsigned int *mode);

    int   in_list_box_set_selected_index(HWND hwnd, unsigned int index);
    int   in_list_box_is_selected_index(HWND hwnd, unsigned int index);
    int   in_list_box_get_selected_index(HWND hwnd, unsigned int start_index, int *index);

    int   in_list_box_set_lighted_index(HWND hwnd, unsigned int index);
    int   in_list_box_is_lighted_index(HWND hwnd, unsigned int index);
    int   in_list_box_get_lighted_index(HWND hwnd, int *index);

    int   in_list_box_get_item_counter(HWND hwnd, unsigned int *counter);

    int   in_list_box_is_read_only(HWND hwnd);
    int   in_list_box_set_read_only(HWND hwnd, unsigned int read_only);


    int  in_list_box_update_lighted_index_rect(/* HWND hwnd */ void *hwnd);

    #ifdef  _LG_SCROLL_BAR_
    int  in_list_box_update_schbar_mid_rect(/* HWND hwnd */ void *hwnd);
    int  in_list_box_update_scvbar_mid_rect(/* HWND hwnd */ void *hwnd);
    #endif

    #ifndef  _LG_ALONE_VERSION_
    HWND  list_box_create(HWND parent, void *gui_common_widget, void *gui_list_box);

    int   list_box_insert_item(HWND hwnd, int index, TCHAR *text, unsigned int text_len);
    int   list_box_get_item(HWND hwnd, unsigned int index, TCHAR *text, unsigned int *text_len);
    int   list_box_delete_item(HWND hwnd, unsigned int index);

    int   list_box_set_select_mode(HWND hwnd, unsigned int  mode);
    int   list_box_get_select_mode(HWND hwnd, unsigned int *mode);

    int   list_box_set_selected_index(HWND hwnd, unsigned int index);
    int   list_box_is_selected_index(HWND hwnd, unsigned int index);
    int   list_box_get_selected_index(HWND hwnd, unsigned int start_index, int *index);

    int   list_box_set_lighted_index(HWND hwnd, unsigned int index);
    int   list_box_is_lighted_index(HWND hwnd, unsigned int index);
    int   list_box_get_lighted_index(HWND hwnd, int *index);

    int   list_box_get_item_counter(HWND hwnd, unsigned int *counter);

    int   list_box_is_read_only(HWND hwnd);
    int   list_box_set_read_only(HWND hwnd, unsigned int read_only);

    #else  /* _LG_ALONE_VERSION_ */
    #define  list_box_create(parent, gui_common_widget, gui_list_box)   in_list_box_create(parent, gui_common_widget, gui_list_box)

    #define  list_box_insert_item(hwnd, index, text, text_len)       in_list_box_insert_item(hwnd,index, text, text_len)
    #define  list_box_get_item(hwnd,index, text, text_len)           in_list_box_get_item(hwnd,index, text, text_len)
    #define  list_box_delete_item(hwnd, index)                       in_list_box_delete_item(hwnd, index)

    #define  list_box_set_select_mode(hwnd, mode)                    in_list_box_set_select_mode(hwnd, mode)
    #define  list_box_get_select_mode(hwnd, mode)                    in_list_box_get_select_mode(hwnd, mode)

    #define  list_box_set_selected_index(hwnd,index)                 in_list_box_set_selected_index(hwnd,index)
    #define  list_box_is_selected_index(hwnd, index)                 in_list_box_is_selected_index(hwnd, index)
    #define  list_box_get_selected_index(hwnd, start_index, index)   in_list_box_get_selected_index(hwnd, start_index, index)

    #define  list_box_set_lighted_index(hwnd,index)                  in_list_box_set_lighted_index(hwnd,index)
    #define  list_box_is_lighted_index(hwnd, index)                  in_list_box_is_lighted_index(hwnd, index)
    #define  list_box_get_lighted_index(hwnd, index)                 in_list_box_get_lighted_index(hwnd, index)

    #define  list_box_get_item_counter(hwnd, counter)                in_list_box_get_item_counter(hwnd, counter)

    #define  list_box_is_read_only(hwnd)                             in_list_box_is_read_only(hwnd)
    #define  list_box_set_read_only(hwnd, read_only)                 in_list_box_set_read_only(hwnd, read_only)
    #endif  /* _LG_ALONE_VERSION_ */

    #define  list_box_add_item(hwnd, text, text_len)                 list_box_insert_item(hwnd, -1, text, text_len)

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_LIST_BOX_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_LIST_BOX_WIDGET_HEADER__ */
