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

#ifndef  __LGUI_WIN_IMAGE_WIDGET_HEADER__
#define  __LGUI_WIN_IMAGE_WIDGET_HEADER__    1

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



/* Image window color */
#ifndef  IMAGE_WIN_DISABLED_BCOLOR
#define  IMAGE_WIN_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  IMAGE_WIN_DISABLED_FCOLOR
#define  IMAGE_WIN_DISABLED_FCOLOR              GUI_DARK
#endif

#ifndef  IMAGE_WIN_INACTIVE_BCOLOR
#define  IMAGE_WIN_INACTIVE_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  IMAGE_WIN_INACTIVE_FCOLOR
#define  IMAGE_WIN_INACTIVE_FCOLOR              GUI_GRAY
#endif

#ifndef  IMAGE_WIN_ACTIVE_BCOLOR
#define  IMAGE_WIN_ACTIVE_BCOLOR                GUI_LIGHT_GRAY
#endif
#ifndef  IMAGE_WIN_ACTIVE_FCOLOR
#define  IMAGE_WIN_ACTIVE_FCOLOR                GUI_BLACK
#endif


/* Image client color */
#ifndef  IMAGE_CLI_DISABLED_BCOLOR
#define  IMAGE_CLI_DISABLED_BCOLOR              GUI_LIGHT_GRAY
#endif
#ifndef  IMAGE_CLI_DISABLED_FCOLOR
#define  IMAGE_CLI_DISABLED_FCOLOR              GUI_GRAY
#endif
#ifndef  IMAGE_CLI_DISABLED_TBCOLOR
#define  IMAGE_CLI_DISABLED_TBCOLOR             GUI_LIGHT_GRAY
#endif
#ifndef  IMAGE_CLI_DISABLED_TFCOLOR
#define  IMAGE_CLI_DISABLED_TFCOLOR             GUI_BLACK
#endif

#ifndef  IMAGE_CLI_INACTIVE_BCOLOR
#define  IMAGE_CLI_INACTIVE_BCOLOR              GUI_LIGHT_WHITE
#endif
#ifndef  IMAGE_CLI_INACTIVE_FCOLOR
#define  IMAGE_CLI_INACTIVE_FCOLOR              GUI_BLACK
#endif
#ifndef  IMAGE_CLI_INACTIVE_TBCOLOR
#define  IMAGE_CLI_INACTIVE_TBCOLOR             GUI_WHITE
#endif
#ifndef  IMAGE_CLI_INACTIVE_TFCOLOR
#define  IMAGE_CLI_INACTIVE_TFCOLOR             GUI_BLACK
#endif


#ifndef  IMAGE_CLI_ACTIVE_BCOLOR
#define  IMAGE_CLI_ACTIVE_BCOLOR                GUI_YELLOW
#endif
#ifndef  IMAGE_CLI_ACTIVE_FCOLOR
#define  IMAGE_CLI_ACTIVE_FCOLOR                GUI_BLACK
#endif
#ifndef  IMAGE_CLI_ACTIVE_TBCOLOR
#define  IMAGE_CLI_ACTIVE_TBCOLOR               GUI_WHITE
#endif
#ifndef  IMAGE_CLI_ACTIVE_TFCOLOR
#define  IMAGE_CLI_ACTIVE_TFCOLOR               GUI_BLACK
#endif


#ifdef  _LG_WINDOW_
#ifdef  _LG_IMAGE_WIDGET_

/* GUI_IMAGE */
struct  _GUI_IMAGE
{
    /* Image type */
    BINT   image_type;

    /* Image align */
    BINT   image_align;

    /* Image frame id */
    unsigned int  frame_id;

    /* Image data pointer */
    const void  *pimage;
};
typedef	struct	_GUI_IMAGE  GUI_IMAGE;

typedef	GUI_IMAGE  IN_GUI_IMAGE;

#define  GET_IN_GUI_IMAGE(hwnd)           ((IN_GUI_IMAGE *)(((HWND)hwnd)->ext))


#ifdef  __cplusplus
extern  "C"
{
#endif

    HWND  in_image_create(HWND parent, void *gui_common_widget, void *gui_image);
    int   in_image_set_image(HWND hwnd, void *gui_image);
    int   in_image_get_image(HWND hwnd, void *gui_image);

    #ifndef  _LG_ALONE_VERSION_
    HWND  image_create(HWND parent, void *gui_common_widget, void *gui_image);
    int   image_set_image(HWND hwnd, void *gui_image);
    int   image_get_image(HWND hwnd, void *gui_image);
    #else
    #define  image_create(parent, gui_common_widget, gui_image)      in_image_create(parent, gui_common_widget, gui_image)
    #define  image_set_image(hwnd, gui_image)                        in_image_set_image(hwnd, gui_image)
    #define  image_get_image(hwnd, gui_image)                        in_image_get_image(hwnd, gui_image)
    #endif

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_IMAGE_WIDGET_ */
#endif  /* _LG_WINDOW_ */

#endif  /* __LGUI_WIN_IMAGE_WIDGET_HEADER__ */
