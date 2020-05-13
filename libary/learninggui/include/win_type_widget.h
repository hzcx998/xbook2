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

#ifndef  __LGUI_WIN_TYPE_WIDGET_HEADER__
#define  __LGUI_WIN_TYPE_WIDGET_HEADER__

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
#include  <win_bar.h>
#include  <win_scbar.h>
#endif



/* GUI_COMMON_WIDGET */
struct  _GUI_COMMON_WIDGET
{
    /* Rao ?? */
    /* Widget ID */
             int  id;

    /* Widget logical coordinates */
    int           left;
    int           top;
    int           right;
    int           bottom;

    /* Widget type */
    BUINT         type;

    /* Widget style */
    UINT         style;

    /* Widget ext style */
    UINT         ext_style;

    /* Widget border width */
    BUINT         border_width;

    /* Window dc */
    GUI_DC        win_dc;

    /* Client dc */
    GUI_DC        client_dc;

    /* Absolute coordinate */
    GUI_RECT      cur_clip_rect;

    /* Invalidate rectangle */
    GUI_RECT      invalidate_rect;

    /* Invalidate rectangle flag */
    BINT          invalidate_flag;

    /* Widget internal callback flag */
    BINT          is_in_callback;

    /* Widget internal callback */
    int           (*in_callback)(void *msg);

    /* Widget app callback flag */
    BINT          is_app_callback;

    /* Widget app callback */
    int           (*app_callback)(void *msg);

    /* Widget status */
    BINT          status;

    /* Accessory widget flag */
    BINT          acc_hwnd_flag;

    /* No focus flag */
    BINT          no_focus_flag;

    /* No erase back flag */
    BINT          no_erase_back_flag;

    /* Widget delete callback flag */
    BINT          is_delete_callback;

    /* Widget delete callback */
    int           (*delete_callback)(void *p);

    #ifdef  _LG_WINDOW_BAR_
    /* Window bar */
    GUI_WINBAR   *winbar;
    #endif

    #ifdef  _LG_SCROLL_BAR_
    /* Horizonal scroll bar */
    GUI_SCBAR    *schbar;

    /* Vertical scroll bar */
    GUI_SCBAR    *scvbar;
    #endif

    #ifdef  _LG_WINDOW_BACKGROUND_IMAGE_
    /* Background image flag */
    BINT          bimage_flag;

    /* Background image type */
    BINT          bimage_type;

    /* Background image align */
    BINT          bimage_align;

    /* Background image frame id */
    unsigned int  frame_id;

    /* Background image data */
    const  void  *pimage;
    #endif

    #ifdef  _LG_WIDGET_USER_DATA_
    /* Widget  user data max bytes */
    unsigned int  max_data_bytes;

    /* Widget  user data bytes */
    unsigned int  data_bytes;

    /* Widget user */
    void         *pdata;
    #endif
};
typedef	struct	_GUI_COMMON_WIDGET  GUI_COMMON_WIDGET;


/* GUI_WND */
struct  _GUI_WND;


/* GUI_WND_HEAD */
struct  _GUI_WND_HEAD
{
    /* Memory prev hwnd pointer */
    struct  _GUI_WND  *prev;

    /* Memory next hwnd pointer */
    struct  _GUI_WND  *next;

    /* Parent hwnd pointer */
    struct  _GUI_WND  *parent;

    /* The first child hwnd pointer */
    struct  _GUI_WND  *fc;

    /* The last child hwnd pointer */
    struct  _GUI_WND  *lc;
};
typedef	struct	_GUI_WND_HEAD   GUI_WND_HEAD;


/* GUI_WND */
struct  _GUI_WND
{
    /* Common hwnd head */
    GUI_WND_HEAD       head;

    /* Common Widget */
    GUI_COMMON_WIDGET  common;

    /* Extended Widget */
    char               ext[4];
};
typedef	struct	_GUI_WND   GUI_WND;
typedef	struct	_GUI_WND  *HWND;


/* Widget type */
#define  FRAME_WIDGET_TYPE               0x01
#define  GROUP_BOX_WIDGET_TYPE           0x02
#define  CELL_WIDGET_TYPE                0x03
#define  LABEL_WIDGET_TYPE               0x04
#define  PUSH_BUTTON_WIDGET_TYPE         0x05
#define  WIDGET_GROUP_TYPE               0x06
#define  RADIO_BUTTON_WIDGET_TYPE        0x07
#define  CHECK_BOX_WIDGET_TYPE           0x08
#define  LINE_EDIT_WIDGET_TYPE           0x09
#define  LIST_BOX_WIDGET_TYPE            0x0A
#define  COM_BOX_WIDGET_TYPE             0x0B
#define  PROGRESS_BAR_WIDGET_TYPE        0x0C
#define  SLIDER_BAR_WIDGET_TYPE          0x0D
#define  IMAGE_WIDGET_TYPE               0x0E
#define  TABLE_WIDGET_TYPE               0x0F



/* Widget type macros */
#define  IS_FRAME_WIDGET(w)              ((((GUI_COMMON_WIDGET *)w)->type)&FRAME_WIDGET_TYPE)
#define  IS_GROUP_BOX_WIDGET(w)          ((((GUI_COMMON_WIDGET *)w)->type)&GROUP_BOX_WIDGET_TYPE)
#define  IS_CELL_WIDGET(w)               ((((GUI_COMMON_WIDGET *)w)->type)&CELL_WIDGET_TYPE)
#define  IS_LABEL_WIDGET(w)              ((((GUI_COMMON_WIDGET *)w)->type)&LABEL_WIDGET_TYPE)
#define  IS_PUSH_BUTTON_WIDGET(w)        ((((GUI_COMMON_WIDGET *)w)->type)&PUSH_BUTTON_WIDGET_TYPE)
#define  IS_WIDGET_GROUP(w)              ((((GUI_COMMON_WIDGET *)w)->type)&WIDGET_GROUP_TYPE)
#define  IS_RADIO_BUTTON_WIDGET(w)       ((((GUI_COMMON_WIDGET *)w)->type)&RADIO_BUTTON_WIDGET_TYPE)
#define  IS_CHECK_BOX_WIDGET(w)          ((((GUI_COMMON_WIDGET *)w)->type)&CHECK_BOX_WIDGET_TYPE)
#define  IS_LINE_EDIT_WIDGET(w)          ((((GUI_COMMON_WIDGET *)w)->type)&LINE_EDIT_WIDGET_TYPE)
#define  IS_LIST_BOX_WIDGET(w)           ((((GUI_COMMON_WIDGET *)w)->type)&LIST_BOX_WIDGET_TYPE)
#define  IS_COM_BOX_WIDGET(w)            ((((GUI_COMMON_WIDGET *)w)->type)&COM_BOX_WIDGET_TYPE)
#define  IS_PROGRESS_BAR_WIDGET(w)       ((((GUI_COMMON_WIDGET *)w)->type)&PROGRESS_BAR_WIDGET_TYPE)
#define  IS_SLIDER_BAR_WIDGET(w)         ((((GUI_COMMON_WIDGET *)w)->type)&SLIDER_BAR_WIDGET_TYPE)
#define  IS_IMAGE_WIDGET(w)              ((((GUI_COMMON_WIDGET *)w)->type)&IMAGE_WIDGET_TYPE)
#define  IS_TABLE_WIDGET(w)              ((((GUI_COMMON_WIDGET *)w)->type)&TABLE_WIDGET_TYPE)


/* THIS macros */
#define  THIS_HWND(msg)                  ((HWND)(((GUI_MESSAGE *)(msg))->to_hwnd))


/* Widget style */
#define  VISUAL_STYLE                    (1<<0)
#define  ENABLE_STYLE                    (1<<1)
#define  BORDER_STYLE                    (1<<2)
#define  WINBAR_STYLE                    (1<<3)
#define  SCBAR_STYLE                     (1<<4)

#define  BORDER_3D_STYLE                 (1<<8)

#define  WINBAR_CLOSE_BTN_STYLE          (1<<9)
#define  WINBAR_MAX_BTN_STYLE            (1<<10)
#define  WINBAR_MIN_BTN_STYLE            (1<<11)

#define  SCBAR_HBAR_STYLE                (1<<12)
#define  SCBAR_VBAR_STYLE                (1<<13)



/* Widget style macros */
#define  IS_BORDER_WIDGET(w)             ((((GUI_COMMON_WIDGET *)w)->style)&BORDER_STYLE)
#define  IS_BORDER_3D_WIDGET(w)          ((((GUI_COMMON_WIDGET *)w)->style)&BORDER_3D_STYLE)

#define  IS_WINBAR_WIDGET(w)             ((((GUI_COMMON_WIDGET *)w)->style)&WINBAR_STYLE)



/* Rect macros */
#define  GET_WINDOW_RECT(hwnd)           (((HWND)hwnd)->common.win_dc.rect)
#define  GET_CLIENT_RECT(hwnd)           (((HWND)hwnd)->common.client_dc.rect)


/* Window bar or scrool bar subwidget ext style */
#define  IS_CLOSE_BTN_WINBAR(w)          ((((GUI_COMMON_WIDGET *)w)->style)&WINBAR_CLOSE_BTN_STYLE)
#define  IS_MAX_BTN_WINBAR(w)            ((((GUI_COMMON_WIDGET *)w)->style)&WINBAR_MAX_BTN_STYLE)
#define  IS_MIN_BTN_WINBAR(w)            ((((GUI_COMMON_WIDGET *)w)->style)&WINBAR_MIN_BTN_STYLE)

#define  IS_HBAR_SCBAR(w)                ((((GUI_COMMON_WIDGET *)w)->style)&SCBAR_HBAR_STYLE)
#define  IS_VBAR_SCBAR(w)                ((((GUI_COMMON_WIDGET *)w)->style)&SCBAR_HBAR_STYLE)


/* Window status */
#define  WINDOW_NORMAL_STATUS            (1<<0x00)
#define  WINDOW_MAX_STATUS               (1<<0x01)
#define  WINDOW_MIN_STATUS               (1<<0x02)


/* Window border default width */
#ifndef  WINDOW_BORDER_WIDTH
#define  WINDOW_BORDER_WIDTH             3
#endif

#endif  /* __LGUI_WIN_TYPE_WIDGET_HEADER__ */
