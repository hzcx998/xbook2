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


#ifndef  __LGUI_ERROR_CODE_HEADER__
#define  __LGUI_ERROR_CODE_HEADER__

/*
 * gui_open error code
 */
/* Screen relatived error code */
#define  ESCREEN_OPEN                           1
#define  ESCREEN_GUI_TO_SCREEN_COLOR            2
#define  ESCREEN_SCREEN_TO_GUI_COLOR            3
#define  ESCREEN_OUTPUT_PIXEL                   4
#define  ESCREEN_CLEAR                          5
#define  ESCREEN_CLOSE                          6
/* Keyboard relatived error code */
#define  EKEYBOARD_OPEN                         10
#define  EKEYBOARD_READ                         11
#define  EKEYBOARD_WRITE                        12
#define  EKEYBOARD_CONTROL                      13
#define  EKEYBOARD_CLOSE                        14
/* Mtjt relatived error code */
#define  EMTJT_OPEN                             20
#define  EMTJT_READ                             21
#define  EMTJT_WRITE                            22
#define  EMTJT_CONTROL                          23
#define  EMTJT_CLOSE                            24
/* GUI locker relatived error code */
#define  EGUI_LOCKER_INIT                       30
#define  EGUI_LOCKER_LOCK                       31
#define  EGUI_LOCKER_UNLOCK                     32
#define  EGUI_LOCKER_DESTROY                    33
/* Callback locker relatived error code */
#define  ECALLBACK_LOCKER_INIT                  40
#define  ECALLBACK_LOCKER_LOCK                  41
#define  ECALLBACK_LOCKER_UNLOCK                42
#define  ECALLBACK_LOCKER_DESTROY               43
/* Init relatived error code */
#define  ECOUNTER_INIT                          50
#define  ETIMER_INIT                            51
#define  ECURSOR_INIT                           52
#define  ESCREEN_OPEN_RETURN                    53
#define  EDC_INIT                               54
#define  EWINDOW_INIT                           55
#define  EKEYBOARD_OPEN_RETURN                  56
#define  EMTJT_OPEN_RETURN                      57
/* Thread relatived error code */
#define  ETHREAD_KEYBOARD_MTTJ                  60
#define  ETHREAD_KEYBOARD                       61
#define  ETHREAD_MTTJ                           62

#endif  /* __LGUI_ERROR_CODE_HEADER__ */
