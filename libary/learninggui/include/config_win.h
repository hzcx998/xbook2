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

/*
 * Created by Rao Youkun (raoyoukun@aliyun.com) at 2013-05-20
 */
#ifndef  __LGUI_CONFIG_WIN_HEADER__
#define  __LGUI_CONFIG_WIN_HEADER__

/* 
 * Window property default value 
 */

/* Window border default width */
#define  WIN_BORDER_WIDTH                         3

/* Default window back color */
#define  DEFAULT_WINDOW_BCOLOR                    GUI_GREEN

/* Window background image */
#define  _LG_WINDOW_BACKGROUND_IMAGE_

/* Caret support macro */
#define  _LG_CARET_



/* 
 * Widgets
 */

/* Widget user data interface */
#define  _LG_WIDGET_USER_DATA_
    #define  MAX_USER_DATA_LEN                    32


/* Window bar */
#define  _LG_WINDOW_BAR_
    /* Window bar default height */
    #define  WIN_BAR_HEIGHT                       20

   /* Window text max byte length */
    #define  MAX_WIN_TEXT_LEN                     32

    /* Window bar default color */
    #define  WINBAR_WIN_DISABLE_BCOLOR            0x00606060
    #define  WINBAR_WIN_DISABLE_FCOLOR            GUI_GRAY
    #define  WINBAR_WIN_DISABLE_TBCOLOR           0x00606060
    #define  WINBAR_WIN_DISABLE_TFCOLOR           GUI_BLACK

    #define  WINBAR_WIN_INACTIVE_BCOLOR           0x007A96DF
    #define  WINBAR_WIN_INACTIVE_FCOLOR           GUI_BLACK
    #define  WINBAR_WIN_INACTIVE_TBCOLOR          0x007A96DF
    #define  WINBAR_WIN_INACTIVE_TFCOLOR          GUI_BLACK

    #define  WINBAR_WIN_ACTIVE_BCOLOR             0x000055E5
    #define  WINBAR_WIN_ACTIVE_FCOLOR             GUI_BLACK
    #define  WINBAR_WIN_ACTIVE_TBCOLOR            0x000055E5
    #define  WINBAR_WIN_ACTIVE_TFCOLOR            GUI_BLACK

    #define  WINBAR_CLI_DISABLE_BCOLOR            0x00606060
    #define  WINBAR_CLI_DISABLE_FCOLOR            GUI_WHITE
    #define  WINBAR_CLI_DISABLE_TBCOLOR           0x00606060
    #define  WINBAR_CLI_DISABLE_TFCOLOR           GUI_BLACK

    #define  WINBAR_CLI_INACTIVE_BCOLOR           0x007A96DF
    #define  WINBAR_CLI_INACTIVE_FCOLOR           GUI_WHITE
    #define  WINBAR_CLI_INACTIVE_TBCOLOR          0x007A96DF
    #define  WINBAR_CLI_INACTIVE_TFCOLOR          GUI_BLACK

    #define  WINBAR_CLI_ACTIVE_BCOLOR             0x000055E5
    #define  WINBAR_CLI_ACTIVE_FCOLOR             GUI_WHITE
    #define  WINBAR_CLI_ACTIVE_TBCOLOR            0x000055E5
    #define  WINBAR_CLI_ACTIVE_TFCOLOR            GUI_BLACK

    #define  WINBAR_CLOSE_DISABLED_BCOLOR         0x00D0C0C0
    #define  WINBAR_CLOSE_INACTIVE_BCOLOR         0x00D0A0A0
    #define  WINBAR_CLOSE_ACTIVE_BCOLOR           0x00E15025
 
    /* Window system button default width */
    #define  SYSTEM_BTN_WIDTH                     16


/* Scroll bar */
#define  _LG_SCROLL_BAR_
    /* Scroll bar default height or width */
    #define  SCBAR_HEIGHT_WIDTH                   20

    /* Scroll bar x multi step */
    #define  SCBAR_X_MULTI_STEP                   10

    /* Scroll bar y multi step */
    #define  SCBAR_Y_MULTI_STEP                   10


    /* Scroll bar default color */
    #define  SCBAR_WIN_DISABLE_BCOLOR             0x00606060
    #define  SCBAR_WIN_DISABLE_FCOLOR             GUI_BLACK
    #define  SCBAR_WIN_DISABLE_TBCOLOR            0x00606060
    #define  SCBAR_WIN_DISABLE_TFCOLOR            GUI_BLACK

    #define  SCBAR_WIN_INACTIVE_BCOLOR            0x007A96DF
    #define  SCBAR_WIN_INACTIVE_FCOLOR            GUI_BLACK
    #define  SCBAR_WIN_INACTIVE_TBCOLOR           0x007A96DF
    #define  SCBAR_WIN_INACTIVE_TFCOLOR           GUI_BLACK

    #define  SCBAR_WIN_ACTIVE_BCOLOR              0x000055E5
    #define  SCBAR_WIN_ACTIVE_FCOLOR              GUI_BLACK
    #define  SCBAR_WIN_ACTIVE_TBCOLOR             0x000055E5
    #define  SCBAR_WIN_ACTIVE_TFCOLOR             GUI_BLACK

    #define  SCBAR_CLI_DISABLE_BCOLOR             0x00606060
    #define  SCBAR_CLI_DISABLE_FCOLOR             GUI_WHITE
    #define  SCBAR_CLI_DISABLE_TBCOLOR            0x00606060
    #define  SCBAR_CLI_DISABLE_TFCOLOR            GUI_BLACK

    #define  SCBAR_CLI_INACTIVE_BCOLOR            0x007A96DF
    #define  SCBAR_CLI_INACTIVE_FCOLOR            GUI_WHITE
    #define  SCBAR_CLI_INACTIVE_TBCOLOR           0x007A96DF
    #define  SCBAR_CLI_INACTIVE_TFCOLOR           GUI_BLACK

    #define  SCBAR_CLI_ACTIVE_BCOLOR              0x000055E5
    #define  SCBAR_CLI_ACTIVE_FCOLOR              GUI_WHITE
    #define  SCBAR_CLI_ACTIVE_TBCOLOR             0x000055E5
    #define  SCBAR_CLI_ACTIVE_TFCOLOR             GUI_BLACK

    #define  SCBAR_CLOSE_DISABLED_BCOLOR          0x00D0C0C0
    #define  SCBAR_CLOSE_INACTIVE_BCOLOR          0x00D0A0A0
    #define  SCBAR_CLOSE_ACTIVE_BCOLOR            0x00E15025
 

/* Frame */
#define  _LG_FRAME_WIDGET_



/* GroupBox */
#define  _LG_GROUP_BOX_WIDGET_
    /* Max GroupBox text length */
    #define  MAX_GROUP_BOX_TEXT_LEN               63

    /* GroupBox window color */
    #define  GBOX_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  GBOX_WIN_DISABLED_FCOLOR             GUI_DARK

    #define  GBOX_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  GBOX_WIN_INACTIVE_FCOLOR             GUI_GRAY

    #define  GBOX_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  GBOX_WIN_ACTIVE_FCOLOR               GUI_BLACK

    /* GroupBox client color */
    #define  GBOX_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  GBOX_CLI_DISABLED_FCOLOR             GUI_GRAY
    #define  GBOX_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
    #define  GBOX_CLI_DISABLED_TFCOLOR            GUI_BLACK

    #define  GBOX_CLI_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  GBOX_CLI_INACTIVE_FCOLOR             GUI_BLACK
    #define  GBOX_CLI_INACTIVE_TBCOLOR            GUI_LIGHT_GRAY
    #define  GBOX_CLI_INACTIVE_TFCOLOR            GUI_BLACK

    #define  GBOX_CLI_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  GBOX_CLI_ACTIVE_FCOLOR               GUI_BLACK
    #define  GBOX_CLI_ACTIVE_TBCOLOR              GUI_LIGHT_GRAY
    #define  GBOX_CLI_ACTIVE_TFCOLOR              GUI_BLACK


/* Cell widget */
#define  _LG_CELL_WIDGET_
    /* Cell window color */
    #define  CELL_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  CELL_WIN_DISABLED_FCOLOR             GUI_DARK

    #define  CELL_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  CELL_WIN_INACTIVE_FCOLOR             GUI_GRAY

    #define  CELL_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  CELL_WIN_ACTIVE_FCOLOR               GUI_BLACK

    /* Cell client color */
    #define  CELL_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  CELL_CLI_DISABLED_FCOLOR             GUI_GRAY
    #define  CELL_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
    #define  CELL_CLI_DISABLED_TFCOLOR            GUI_BLACK

    #define  CELL_CLI_INACTIVE_BCOLOR             GUI_BLACK
    #define  CELL_CLI_INACTIVE_FCOLOR             GUI_WHITE
    #define  CELL_CLI_INACTIVE_TBCOLOR            GUI_BLACK
    #define  CELL_CLI_INACTIVE_TFCOLOR            GUI_WHITE

    #define  CELL_CLI_ACTIVE_BCOLOR               GUI_BLACK
    #define  CELL_CLI_ACTIVE_FCOLOR               GUI_WHITE
    #define  CELL_CLI_ACTIVE_TBCOLOR              GUI_BLACK
    #define  CELL_CLI_ACTIVE_TFCOLOR              GUI_WHITE


/* Label */
#define  _LG_LABEL_WIDGET_
    /* Label max length */
    #define  MAX_LABEL_TEXT_LEN                   255
    /* Label align type */
    #define  LABEL_ALIGN_TYPE                     (TEXT_HCENTER_ALIGN | TEXT_VCENTER_ALIGN)

    /* Label window color */
    #define  LBL_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  LBL_WIN_DISABLED_FCOLOR              GUI_DARK

    #define  LBL_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  LBL_WIN_INACTIVE_FCOLOR              GUI_GRAY

    #define  LBL_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
    #define  LBL_WIN_ACTIVE_FCOLOR                GUI_BLACK

    /* Label client color */
    #define  LBL_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  LBL_CLI_DISABLED_FCOLOR              GUI_GRAY
    #define  LBL_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
    #define  LBL_CLI_DISABLED_TFCOLOR             GUI_BLACK

    #define  LBL_CLI_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  LBL_CLI_INACTIVE_FCOLOR              GUI_BLACK
    #define  LBL_CLI_INACTIVE_TBCOLOR             GUI_LIGHT_GRAY
    #define  LBL_CLI_INACTIVE_TFCOLOR             GUI_BLACK

    #define  LBL_CLI_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
    #define  LBL_CLI_ACTIVE_FCOLOR                GUI_BLACK
    #define  LBL_CLI_ACTIVE_TBCOLOR               GUI_LIGHT_GRAY
    #define  LBL_CLI_ACTIVE_TFCOLOR               GUI_BLACK


/* PushButton widget */
#define  _LG_PUSH_BUTTON_WIDGET_
    /* PushButton window color */
    #define  PBTN_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  PBTN_WIN_DISABLED_FCOLOR              GUI_DARK

    #define  PBTN_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  PBTN_WIN_INACTIVE_FCOLOR              GUI_GRAY

    #define  PBTN_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
    #define  PBTN_WIN_ACTIVE_FCOLOR                GUI_BLACK

    /* PushButton client color */
    #define  PBTN_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  PBTN_CLI_DISABLED_FCOLOR              GUI_GRAY
    #define  PBTN_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
    #define  PBTN_CLI_DISABLED_TFCOLOR             GUI_BLACK

    #define  PBTN_CLI_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  PBTN_CLI_INACTIVE_FCOLOR              GUI_BLACK
    #define  PBTN_CLI_INACTIVE_TBCOLOR             GUI_LIGHT_GRAY
    #define  PBTN_CLI_INACTIVE_TFCOLOR             GUI_BLACK

    #define  PBTN_CLI_ACTIVE_BCOLOR                0x00E0E0E0
    #define  PBTN_CLI_ACTIVE_FCOLOR                GUI_BLACK
    #define  PBTN_CLI_ACTIVE_TBCOLOR               GUI_LIGHT_GRAY
    #define  PBTN_CLI_ACTIVE_TFCOLOR               GUI_BLACK


/* WidgetGroup widget */
#define  _LG_WIDGET_GROUP_


/* RadioButton widget */
#define  _LG_RADIO_BUTTON_WIDGET_

    #define  RBTN_RADIUS_OFFSET                    4

    /* RadioButton window color */
    #define  RBTN_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  RBTN_WIN_DISABLED_FCOLOR              GUI_DARK

    #define  RBTN_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  RBTN_WIN_INACTIVE_FCOLOR              GUI_GRAY

    #define  RBTN_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
    #define  RBTN_WIN_ACTIVE_FCOLOR                GUI_BLACK

    /* RadioButton client color */
    #define  RBTN_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  RBTN_CLI_DISABLED_FCOLOR              GUI_GRAY
    #define  RBTN_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
    #define  RBTN_CLI_DISABLED_TFCOLOR             GUI_BLACK

    #define  RBTN_CLI_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  RBTN_CLI_INACTIVE_FCOLOR              GUI_WHITE
    #define  RBTN_CLI_INACTIVE_TBCOLOR             GUI_LIGHT_GRAY
    #define  RBTN_CLI_INACTIVE_TFCOLOR             GUI_BLACK

    #define  RBTN_CLI_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
    #define  RBTN_CLI_ACTIVE_FCOLOR                GUI_WHITE
    #define  RBTN_CLI_ACTIVE_TBCOLOR               GUI_LIGHT_GRAY
    #define  RBTN_CLI_ACTIVE_TFCOLOR               GUI_BLACK



/* CheckBox widget */
#define  _LG_CHECK_BOX_WIDGET_
    /* CheckBox window color */
    #define  CKBOX_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  CKBOX_WIN_DISABLED_FCOLOR             GUI_DARK

    #define  CKBOX_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  CKBOX_WIN_INACTIVE_FCOLOR             GUI_GRAY

    #define  CKBOX_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  CKBOX_WIN_ACTIVE_FCOLOR               GUI_BLACK

    /* CheckBox client color */
    #define  CKBOX_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  CKBOX_CLI_DISABLED_FCOLOR             GUI_GRAY
    #define  CKBOX_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
    #define  CKBOX_CLI_DISABLED_TFCOLOR            GUI_BLACK

    #define  CKBOX_CLI_INACTIVE_BCOLOR             GUI_LIGHT_WHITE
    #define  CKBOX_CLI_INACTIVE_FCOLOR             GUI_BLACK
    #define  CKBOX_CLI_INACTIVE_TBCOLOR            GUI_WHITE
    #define  CKBOX_CLI_INACTIVE_TFCOLOR            GUI_BLACK

    #define  CKBOX_CLI_ACTIVE_BCOLOR               GUI_YELLOW
    #define  CKBOX_CLI_ACTIVE_FCOLOR               GUI_BLACK
    #define  CKBOX_CLI_ACTIVE_TBCOLOR              GUI_WHITE
    #define  CKBOX_CLI_ACTIVE_TFCOLOR              GUI_BLACK


/* LineEdit */
#define  _LG_LINE_EDIT_WIDGET_
    /* Text max length */
    #define  MAX_LINE_EDIT_TEXT_LEN                255

    /* LineEdit window color */
    #define  LEDIT_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  LEDIT_WIN_DISABLED_FCOLOR             GUI_DARK

    #define  LEDIT_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  LEDIT_WIN_INACTIVE_FCOLOR             GUI_GRAY

    #define  LEDIT_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  LEDIT_WIN_ACTIVE_FCOLOR               GUI_BLACK

    /* LineEdit client color */
    #define  LEDIT_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  LEDIT_CLI_DISABLED_FCOLOR             GUI_GRAY
    #define  LEDIT_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
    #define  LEDIT_CLI_DISABLED_TFCOLOR            GUI_BLACK

    #define  LEDIT_CLI_INACTIVE_BCOLOR             GUI_LIGHT_WHITE
    #define  LEDIT_CLI_INACTIVE_FCOLOR             GUI_BLACK
    #define  LEDIT_CLI_INACTIVE_TBCOLOR            GUI_WHITE
    #define  LEDIT_CLI_INACTIVE_TFCOLOR            GUI_BLACK

    #define  LEDIT_CLI_ACTIVE_BCOLOR               GUI_YELLOW
    #define  LEDIT_CLI_ACTIVE_FCOLOR               GUI_BLACK
    #define  LEDIT_CLI_ACTIVE_TBCOLOR              GUI_WHITE
    #define  LEDIT_CLI_ACTIVE_TFCOLOR              GUI_BLACK


/* ListBox */
#define  _LG_LIST_BOX_WIDGET_
    /* ListBox min row height */
    #define  MIN_LIST_BOX_ROW_HEIGHT               16
 
    /* ListBox min char width */
    #define  MIN_LIST_BOX_CHAR_WIDTH               8
 
    /* ListBox window color */
    #define  LBOX_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  LBOX_WIN_DISABLED_FCOLOR              GUI_DARK

    #define  LBOX_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  LBOX_WIN_INACTIVE_FCOLOR              GUI_GRAY

    #define  LBOX_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
    #define  LBOX_WIN_ACTIVE_FCOLOR                GUI_BLACK

    /* ListBox client color */
    #define  LBOX_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  LBOX_CLI_DISABLED_FCOLOR              GUI_GRAY
    #define  LBOX_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
    #define  LBOX_CLI_DISABLED_TFCOLOR             GUI_BLACK

    #define  LBOX_CLI_INACTIVE_BCOLOR              GUI_LIGHT_WHITE
    #define  LBOX_CLI_INACTIVE_FCOLOR              GUI_BLACK
    #define  LBOX_CLI_INACTIVE_TBCOLOR             GUI_WHITE
    #define  LBOX_CLI_INACTIVE_TFCOLOR             GUI_BLACK

    #define  LBOX_CLI_ACTIVE_BCOLOR                GUI_WHITE
    #define  LBOX_CLI_ACTIVE_FCOLOR                GUI_BLACK
    #define  LBOX_CLI_ACTIVE_TBCOLOR               GUI_WHITE
    #define  LBOX_CLI_ACTIVE_TFCOLOR               GUI_BLACK


/* ComBox */
#define  _LG_COM_BOX_WIDGET_

    #define   _LG_COM_BOX_EXTENSION_

    /* ComBox window color */
    #define  CBBOX_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  CBBOX_WIN_DISABLED_FCOLOR             GUI_DARK

    #define  CBBOX_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  CBBOX_WIN_INACTIVE_FCOLOR             GUI_GRAY
    #define  CBBOX_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  CBBOX_WIN_ACTIVE_FCOLOR               GUI_BLACK

    /* ComBox client color */
    #define  CBBOX_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  CBBOX_CLI_DISABLED_FCOLOR             GUI_GRAY
    #define  CBBOX_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
    #define  CBBOX_CLI_DISABLED_TFCOLOR            GUI_BLACK

    #define  CBBOX_CLI_INACTIVE_BCOLOR             GUI_LIGHT_WHITE
    #define  CBBOX_CLI_INACTIVE_FCOLOR             GUI_BLACK
    #define  CBBOX_CLI_INACTIVE_TBCOLOR            GUI_WHITE
    #define  CBBOX_CLI_INACTIVE_TFCOLOR            GUI_BLACK

    #define  CBBOX_CLI_ACTIVE_BCOLOR               GUI_YELLOW
    #define  CBBOX_CLI_ACTIVE_FCOLOR               GUI_BLACK
    #define  CBBOX_CLI_ACTIVE_TBCOLOR              GUI_WHITE
    #define  CBBOX_CLI_ACTIVE_TFCOLOR              GUI_BLACK


/* ProgressBar widget */
#define  _LG_PROGRESS_BAR_WIDGET_
    /* ProgressBar window color */
    #define  PGBAR_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  PGBAR_WIN_DISABLED_FCOLOR             GUI_DARK

    #define  PGBAR_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  PGBAR_WIN_INACTIVE_FCOLOR             GUI_GRAY

    #define  PGBAR_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  PGBAR_WIN_ACTIVE_FCOLOR               GUI_BLACK

    /* ProgressBar client color */
    #define  PGBAR_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  PGBAR_CLI_DISABLED_FCOLOR             GUI_GRAY
    #define  PGBAR_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
    #define  PGBAR_CLI_DISABLED_TFCOLOR            GUI_BLACK

    #define  PGBAR_CLI_INACTIVE_BCOLOR             GUI_GRAY
    #define  PGBAR_CLI_INACTIVE_FCOLOR             GUI_BLACK
    #define  PGBAR_CLI_INACTIVE_TBCOLOR            GUI_GRAY
    #define  PGBAR_CLI_INACTIVE_TFCOLOR            GUI_BLACK

    #define  PGBAR_CLI_ACTIVE_BCOLOR               GUI_GRAY
    #define  PGBAR_CLI_ACTIVE_FCOLOR               GUI_BLACK
    #define  PGBAR_CLI_ACTIVE_TBCOLOR              GUI_GRAY
    #define  PGBAR_CLI_ACTIVE_TFCOLOR              GUI_BLACK


/* SliderBar widget */
#define  _LG_SLIDER_BAR_WIDGET_
    /* SliderBar window color */
    #define  SLBAR_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  SLBAR_WIN_DISABLED_FCOLOR             GUI_DARK

    #define  SLBAR_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  SLBAR_WIN_INACTIVE_FCOLOR             GUI_GRAY

    #define  SLBAR_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  SLBAR_WIN_ACTIVE_FCOLOR               GUI_BLACK

    /* SliderBar client color */
    #define  SLBAR_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  SLBAR_CLI_DISABLED_FCOLOR             GUI_GRAY
    #define  SLBAR_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
    #define  SLBAR_CLI_DISABLED_TFCOLOR            GUI_BLACK

    #define  SLBAR_CLI_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  SLBAR_CLI_INACTIVE_FCOLOR             GUI_BLACK
    #define  SLBAR_CLI_INACTIVE_TBCOLOR            GUI_LIGHT_GRAY
    #define  SLBAR_CLI_INACTIVE_TFCOLOR            GUI_BLACK

    #define  SLBAR_CLI_ACTIVE_BCOLOR               0x00E0E0E0
    #define  SLBAR_CLI_ACTIVE_FCOLOR               GUI_BLACK
    #define  SLBAR_CLI_ACTIVE_TBCOLOR              GUI_LIGHT_GRAY
    #define  SLBAR_CLI_ACTIVE_TFCOLOR              GUI_BLACK


/* Image widget */
#define  _LG_IMAGE_WIDGET_

    /* Image window color */
    #define  IMAGE_WIN_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  IMAGE_WIN_DISABLED_FCOLOR             GUI_DARK

    #define  IMAGE_WIN_INACTIVE_BCOLOR             GUI_LIGHT_GRAY
    #define  IMAGE_WIN_INACTIVE_FCOLOR             GUI_GRAY
    #define  IMAGE_WIN_ACTIVE_BCOLOR               GUI_LIGHT_GRAY
    #define  IMAGE_WIN_ACTIVE_FCOLOR               GUI_BLACK

    /* ComBox client color */
    #define  IMAGE_CLI_DISABLED_BCOLOR             GUI_LIGHT_GRAY
    #define  IMAGE_CLI_DISABLED_FCOLOR             GUI_GRAY
    #define  IMAGE_CLI_DISABLED_TBCOLOR            GUI_LIGHT_GRAY
    #define  IMAGE_CLI_DISABLED_TFCOLOR            GUI_BLACK

    #define  IMAGE_CLI_INACTIVE_BCOLOR             GUI_LIGHT_WHITE
    #define  IMAGE_CLI_INACTIVE_FCOLOR             GUI_BLACK
    #define  IMAGE_CLI_INACTIVE_TBCOLOR            GUI_WHITE
    #define  IMAGE_CLI_INACTIVE_TFCOLOR            GUI_BLACK

    #define  IMAGE_CLI_ACTIVE_BCOLOR               GUI_YELLOW
    #define  IMAGE_CLI_ACTIVE_FCOLOR               GUI_BLACK
    #define  IMAGE_CLI_ACTIVE_TBCOLOR              GUI_WHITE
    #define  IMAGE_CLI_ACTIVE_TFCOLOR              GUI_BLACK


/* Table */
#define  _LG_TABLE_WIDGET_
    /* Tabel max length */
    /* #define  MAX_LABEL_TEXT_LEN                   255 */

    /* Table align type */
    /* #define  LABEL_ALIGN_TYPE                     (TEXT_HCENTER_ALIGN | TEXT_VCENTER_ALIGN) */

    /* Table window color */
    #define  TAB_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  TAB_WIN_DISABLED_FCOLOR              GUI_DARK

    #define  TAB_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  TAB_WIN_INACTIVE_FCOLOR              GUI_GRAY

    #define  TAB_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
    #define  TAB_WIN_ACTIVE_FCOLOR                GUI_BLACK

    /* Tabel client color */
    #define  TAB_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
    #define  TAB_CLI_DISABLED_FCOLOR              GUI_GRAY
    #define  TAB_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
    #define  TAB_CLI_DISABLED_TFCOLOR             GUI_BLACK

    #define  TAB_CLI_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
    #define  TAB_CLI_INACTIVE_FCOLOR              GUI_BLACK
    #define  TAB_CLI_INACTIVE_TBCOLOR             GUI_LIGHT_GRAY
    #define  TAB_CLI_INACTIVE_TFCOLOR             GUI_BLACK

    #define  TAB_CLI_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
    #define  TAB_CLI_ACTIVE_FCOLOR                GUI_BLACK
    #define  TAB_CLI_ACTIVE_TBCOLOR               GUI_LIGHT_GRAY
    #define  TAB_CLI_ACTIVE_TFCOLOR               GUI_BLACK


#endif	/* __LGUI_CONFIGI_WIN_HEADER__ */
