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

#include  <pen.h>


#ifdef  _LG_PEN_
GUI_PEN  *in_get_pen(HDC hdc)
{
    if ( hdc == NULL )
        return  NULL;

    return  (&(hdc->pen));
}

#ifndef  _LG_ALONE_VERSION_
GUI_PEN  *get_pen(HDC hdc)
{
    GUI_PEN  *pen = NULL;

    gui_lock( );
    pen = in_get_pen(hdc);
    gui_unlock( );

    return  pen;
}
#endif

int  in_set_pen(HDC hdc, void *pen)
{
    if ( hdc == NULL )
        return  -1;
    if ( pen == NULL )
        return  -1;

    hdc->pen = *((GUI_PEN *)pen);

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  set_pen(HDC hdc, void *pen)
{
    int  ret = 0;

    gui_lock( );
    ret = in_set_pen(hdc, pen);
    gui_unlock( );

    return  ret;
}
#endif

#endif  /* _LG_PEN_ */
