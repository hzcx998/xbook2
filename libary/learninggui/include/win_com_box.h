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

#ifndef  __LGUI_COM_BOX_WIDGET_HEADER__
#define  __LGUI_COM_BOX_WIDGET_HEADER__    1

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




/* ComBox window color */
#ifndef  CBBOX_WIN_DISABLED_BCOLOR
#define  CBBOX_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CBBOX_WIN_DISABLED_FCOLOR
#define  CBBOX_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  CBBOX_WIN_INACTIVE_BCOLOR
#define  CBBOX_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CBBOX_WIN_INACTIVE_FCOLOR
#define  CBBOX_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  CBBOX_WIN_ACTIVE_BCOLOR
#define  CBBOX_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  CBBOX_WIN_ACTIVE_FCOLOR
#define  CBBOX_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif

/* ComBox client color */
#ifndef  CBBOX_CLI_DISABLED_BCOLOR
#define  CBBOX_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  CBBOX_CLI_DISABLED_FCOLOR
#define  CBBOX_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  CBBOX_CLI_DISABLED_TBCOLOR
#define  CBBOX_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  CBBOX_CLI_DISABLED_TFCOLOR
#define  CBBOX_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  CBBOX_CLI_INACTIVE_BCOLOR
#define  CBBOX_CLI_INACTIVE_BCOLOR              GUI_LIGHT_WHITE
#endif
#ifndef  CBBOX_CLI_INACTIVE_FCOLOR
#define  CBBOX_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  CBBOX_CLI_INACTIVE_TBCOLOR
#define  CBBOX_CLI_INACTIVE_TBCOLOR             GUI_WHITE
#endif
#ifndef  CBBOX_CLI_INACTIVE_TFCOLOR
#define  CBBOX_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif

#ifndef  CBBOX_CLI_ACTIVE_BCOLOR
#define  CBBOX_CLI_ACTIVE_BCOLOR                GUI_YELLOW
#endif
#ifndef  CBBOX_CLI_ACTIVE_FCOLOR
#define  CBBOX_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  CBBOX_CLI_ACTIVE_TBCOLOR
#define  CBBOX_CLI_ACTIVE_TBCOLOR               GUI_WHITE
#endif
#ifndef  CBBOX_CLI_ACTIVE_TFCOLOR
#define  CBBOX_CLI_ACTIVE_TFCOLOR               GUI_BLACK
#endif


/* ComBox opening directiopn */
#ifndef  CBBOX_OPEN_DOWN
#define  CBBOX_OPEN_DOWN                        0x00
#endif
#ifndef  CBBOX_OPEN_UP
#define  CBBOX_OPEN_UP                          0x01
#endif

/* ComBox height */
#ifndef  CBBOX_LIST_BOX_MINI_HEIGHT
#define  CBBOX_LIST_BOX_MINI_HEIGHT             20
#endif

#ifndef  CBBOX_LIST_BOX_HEIGHT
#define  CBBOX_LIST_BOX_HEIGHT                  100
#endif

#if      (CBBOX_LIST_BOX_HEIGHT < CBBOX_LIST_BOX_MINI_HEIGHT)
#undef   CBBOX_LIST_BOX_HEIGHT
#define  CBBOX_LIST_BOX_HEIGHT                  CBBOX_LIST_BOX_MINI_HEIGHT
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_COM_BOX_WIDGET_

/* GUI_COM_BOX */
struct  _GUI_COM_BOX
{
    /* Line edit style */
    BUINT        line_edit_style;

    /* Line edit ext style */
    BUINT        line_edit_ext_style;

    /* List box style */
    BUINT        list_box_style;

    /* List box ext style */
    BUINT        list_box_ext_style;

    /* Com box opening direction */
    BUINT        open_dir;

    /* Com box opening length */
    unsigned int open_len;
};
typedef	struct	_GUI_COM_BOX  GUI_COM_BOX;



/* IN_GUI_COM_BOX */
struct  _IN_GUI_COM_BOX
{
    /* LineEdit pointer */	
    HWND         pline_edit;

    /* ListBox pointer */	
    HWND         plist_box;

    /* ComBox arrow rect */
    GUI_RECT     btn_rect;

    /* Line edit style */
    BUINT        line_edit_style;

    /* Line edit ext style */
    BUINT        line_edit_ext_style;

    /* List box style */
    BUINT        list_box_style;

    /* List box ext style */
    BUINT        list_box_ext_style;

    /* Com box opening direction */
    BUINT        open_dir;

    /* Com box opening length */
    unsigned int open_len;

    /* ComBox state */
    BINT  state;

    /* saved rect */
    GUI_RECT     closed_win_rect;
    GUI_RECT     closed_cli_rect;

    GUI_RECT     opened_win_rect;
    GUI_RECT     opened_cli_rect;
};
typedef	struct	_IN_GUI_COM_BOX  IN_GUI_COM_BOX;

#define  GET_IN_GUI_COM_BOX(hwnd)           ((IN_GUI_COM_BOX *)(((HWND)hwnd)->ext))



#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND in_com_box_create(HWND parent, void *gui_common_widget, void *gui_com_box);

    int  in_com_box_get_line_edit_hwnd(HWND hwnd, HWND *hline_edit);
    int  in_com_box_get_list_box_hwnd(HWND hwnd, HWND *hlist_box);

    int  in_com_box_get_state(HWND hwnd, unsigned int *opened);
    int  in_com_box_set_state(HWND hwnd, unsigned int  opened);

    int  in_com_box_set_index_item(HWND hwnd, unsigned int index);

    int  in_com_box_line_edit_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len);
    int  in_com_box_line_edit_set_text(HWND hwnd, TCHAR *text, unsigned int text_len);

    #ifdef   _LG_COM_BOX_EXTENSION_
    int  in_com_box_list_box_insert_item(HWND hwnd, int index, TCHAR *text, unsigned int text_len);
    int  in_com_box_list_box_get_item(HWND hwnd, unsigned int index, TCHAR *text, unsigned int *text_len);
    int  in_com_box_list_box_delete_item(HWND hwnd, unsigned int index);

    int  in_com_box_list_box_set_select_mode(HWND hwnd, unsigned int  mode);
    int  in_com_box_list_box_get_select_mode(HWND hwnd, unsigned int *mode);

    int  in_com_box_list_box_set_selected_index(HWND hwnd, unsigned int index);
    int  in_com_box_list_box_is_selected_index(HWND hwnd, unsigned int index);
    int  in_com_box_list_box_get_selected_index(HWND hwnd, unsigned int start_index, int *index);

    int  in_com_box_list_box_set_lighted_index(HWND hwnd, unsigned int index);
    int  in_com_box_list_box_is_lighted_index(HWND hwnd, unsigned int index);
    int  in_com_box_list_box_get_lighted_index(HWND hwnd, int *index);

    int  in_com_box_list_box_get_item_counter(HWND hwnd, unsigned int *counter);

    int  in_com_box_list_box_is_read_only(HWND hwnd);
    int  in_com_box_list_box_set_read_only(HWND hwnd, unsigned int read_only);
    #endif  /* _LG_COM_BOX_EXTENSION_ */


    #ifndef  _LG_ALONE_VERSION_
    HWND com_box_create(HWND parent, void *gui_common_widget, void *gui_com_box);

    int  com_box_get_line_edit_hwnd(HWND hwnd, HWND *hline_edit);
    int  com_box_get_list_box_hwnd(HWND hwnd, HWND *hlist_box);

    int  com_box_get_state(HWND hwnd, unsigned int *opened);
    int  com_box_set_state(HWND hwnd, unsigned int  opened);

    int  com_box_set_index_item(HWND hwnd, unsigned int index);

    int  com_box_line_edit_get_text(HWND hwnd, TCHAR *text, unsigned int *text_len);
    int  com_box_line_edit_set_text(HWND hwnd, TCHAR *text, unsigned int text_len);

    #ifdef   _LG_COM_BOX_EXTENSION_
    int  com_box_list_box_insert_item(HWND hwnd, int index, TCHAR *text, unsigned int text_len);
    int  com_box_list_box_get_item(HWND hwnd, unsigned int index, TCHAR *text, unsigned int *text_len);
    int  com_box_list_box_delete_item(HWND hwnd, unsigned int index);

    int  com_box_list_box_set_select_mode(HWND hwnd, unsigned int  mode);
    int  com_box_list_box_get_select_mode(HWND hwnd, unsigned int *mode);

    int  com_box_list_box_set_selected_index(HWND hwnd, unsigned int index);
    int  com_box_list_box_is_selected_index(HWND hwnd, unsigned int index);
    int  com_box_list_box_get_selected_index(HWND hwnd, unsigned int start_index, int *index);

    int  com_box_list_box_set_lighted_index(HWND hwnd, unsigned int index);
    int  com_box_list_box_is_lighted_index(HWND hwnd, unsigned int index);
    int  com_box_list_box_get_lighted_index(HWND hwnd, int *index);

    int  com_box_list_box_get_item_counter(HWND hwnd, unsigned int *counter);

    int  com_box_list_box_is_read_only(HWND hwnd);
    int  com_box_list_box_set_read_only(HWND hwnd, unsigned int read_only);
    #endif  /* _LG_COM_BOX_EXTENSION_ */

    #else  /* _LG_ALONE_VERSION_ */

    #define  com_box_create(parent, gui_common_widget, gui_com_box)          in_com_box_create(parent, gui_common_widget, gui_com_box)

    #define  com_box_get_line_edit_hwnd(hwnd, hline_edit)                    in_com_box_get_line_edit_hwnd(hwnd, hline_edit)
    #define  com_box_get_list_box_hwnd(hwnd, hlist_box)                      in_com_box_get_list_box_hwnd(hwnd, hlist_box)

    #define  com_box_get_state(hwnd, opened)                                 in_com_box_get_state(hwnd, opened)
    #define  com_box_set_state(hwnd, opened)                                 in_com_box_set_state(hwnd, opened)

    #define  com_box_set_index_item(hwnd, index)                             in_com_box_set_index_item(hwnd, index)

    #define  com_box_line_edit_get_text(hwnd, text, max_len)                 in_com_box_line_edit_get_text(hwnd, text, max_len)
    #define  com_box_line_edit_set_text(hwnd, text, text_len)                in_com_box_line_edit_set_text(hwnd, text, text_len)

    #ifdef   _LG_COM_BOX_EXTENSION_
    #define  com_box_list_box_insert_item(hwnd, index, text, text_len)       in_com_box_list_box_insert_item(hwnd,index, text, text_len)
    #define  com_box_list_box_get_item(hwnd,index, text, text_len)           in_com_box_list_box_get_item(hwnd,index, text, text_len)
    #define  com_box_list_box_delete_item(hwnd, index)                       in_com_box_list_box_delete_item(hwnd, index)

    #define  com_box_list_box_set_select_mode(hwnd, mode)                    in_com_box_list_box_set_select_mode(hwnd, mode)
    #define  com_box_list_box_get_select_mode(hwnd, mode)                    in_com_box_list_box_get_select_mode(hwnd, mode)

    #define  com_box_list_box_set_selected_index(hwnd,index)                 in_com_box_list_box_set_selected_index(hwnd,index)
    #define  com_box_list_box_is_selected_index(hwnd, index)                 in_com_box_list_box_is_selected_index(hwnd, index)
    #define  com_box_list_box_get_selected_index(hwnd, start_index, index)   in_com_box_list_box_get_selected_index(hwnd, start_index, index)

    #define  com_box_list_box_set_lighted_index(hwnd,index)                  in_com_box_list_box_set_lighted_index(hwnd,index)
    #define  com_box_list_box_is_lighted_index(hwnd, index)                  in_com_box_list_box_is_lighted_index(hwnd, index)
    #define  com_box_list_box_get_lighted_index(hwnd, index)                 in_com_box_list_box_get_lighted_index(hwnd, index)

    #define  com_box_list_box_get_item_counter(hwnd, counter)                in_com_box_list_box_get_item_counter(hwnd, counter)

    #define  com_box_list_box_is_read_only(hwnd)                             in_com_box_list_box_is_read_only(hwnd)
    #define  com_box_list_box_set_read_only(hwnd, read_only)                 in_com_box_list_box_set_read_only(hwnd, read_only)
    #endif  /* _LG_COM_BOX_EXTENSION_ */

    #endif  /* _LG_ALONE_VERSION_ */

    #ifdef   _LG_COM_BOX_EXTENSION_
    #define  com_box_list_box_add_item(hwnd, text, text_len)                 com_box_list_box_insert_item(hwnd, -1, text, text_len)
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_COM_BOX_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_COM_BOX_WIDGET_HEADER__ */
