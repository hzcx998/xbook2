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

#include  <lock.h>

#include  <brush.h>


#ifdef  _LG_BRUSH_

GUI_BRUSH  *in_get_brush(HDC hdc)
{
    if ( hdc == NULL )
        return  NULL;

    return  (&(hdc->brush));
}

#ifndef  _LG_ALONE_VERSION_
GUI_BRUSH  *get_brush(HDC hdc)
{
    GUI_BRUSH  *brush = NULL;

    gui_lock( );
    brush = in_get_brush(hdc);
    gui_unlock( );

    return  brush;
}
#endif

int  in_set_brush(HDC hdc, void *brush)
{
    if ( hdc == NULL )
        return  -1;
    if ( brush == NULL )
        return  -1;

    hdc->brush = *((GUI_BRUSH *)brush);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  set_brush(HDC hdc, void *brush)
{
    int  ret = 0;

    gui_lock( );
    ret = in_set_brush(hdc, brush);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_BRUSH_ */
