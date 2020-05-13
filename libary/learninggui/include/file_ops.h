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

#ifndef  __LGUI_FILE_OPS_HEADER__
#define  __LGUI_FILE_OPS_HEADER__

#include  <stdio.h>

#include  <config_type.h>

#include  <config_basic.h>
#include  <dep_cnf_basic.h>
#include  <dep_cnf_type.h>

#ifdef  _LG_WINDOW_
#include  <config_win.h>
#include  <dep_cnf_win.h>
#endif

#include  <type_gui.h>


#ifdef  _LG_FILE_SYSTEM_

#ifdef  __cplusplus
extern  "C"
{
#endif

    /* int  stream_create_path(const char *path); */

    FILE  *stream_open_file(const char *path, const char *mode);
    int    stream_read_file(FILE *stream, void *buf, size_t len);
    int    stream_write_file(FILE *stream, const void *buf, size_t len);
    int    stream_close_file(FILE *stream);
    int    stream_remove_file(const char *path);

#ifdef  __cplusplus
}
#endif

#endif  /* _LG_FILE_SYSTEM_ */

#endif  /* __LGUI_FILE_OPS_HEADER__ */
