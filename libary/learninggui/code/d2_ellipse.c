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
#include  <cursor.h>

#include  <dc.h>

#include  <d2_pixel.h>
#include  <d2_line.h>
#include  <screen.h>

#include  <d2_ellipse.h>

#ifdef  _LG_WINDOW_
#include  <win_clip.h>
#endif



#ifdef	_LG_ELLIPSE_
int  in_ellipse(HDC hdc, int x0, int y0, int x1, int y1)
{

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  ellipse(HDC hdc, int x0, int y0, int x1, int y1)
{
    int  ret = 0;

    gui_lock( );
    ret = in_ellipse(hdc, x0, y0, x1, y1);
    gui_unlock( );

    return  ret;
}
#endif

#endif	/* _LG_ELLIPSE_ */
