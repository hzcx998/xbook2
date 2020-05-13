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

#include  <font_ops.h>


#ifdef  _LG_FONT_
	
#ifdef  _LG_FONT_ID_
int  in_get_font_id_by_font_name(const void *font, const UCHAR *font_name, UCHAR *font_id)
{
    GUI_FONT  *p = (GUI_FONT *)font;

    if ( p == 0 )
        return  0;

    /* If MONO_CHARSET_FONT_TYPE == 0, should avoid to comping warning */
    /*    
    if ( (p->type < MONO_CHARSET_FONT_TYPE) || (p->type > MIXED_DISCRETE_FONT_TYPE) )
        goto  GET_FONT_END;
    */    
    if (p->type > MIXED_DISCRETE_FONT_TYPE )
        goto  GET_FONT_END;


    if ( (p->type) == MONO_CHARSET_FONT_TYPE )
        ;

    if ( (p->type) == MIXED_CHARSET_FONT_TYPE )
        ;

    if ( (p->type) == MONO_DISCRETE_FONT_TYPE )
        ;

    if ( (p->type) == MIXED_DISCRETE_FONT_TYPE )
        ;


    GET_FONT_END:

    return  0;
}

#ifndef  _LG_ALONE_VERSION_
int  get_font_id_by_font_name(const void *font, const UCHAR *font_name, UCHAR *font_id)
{
    int  ret = 0;

    gui_lock( );
    ret = in_get_font_id_by_font_name(font, font_name, font_id);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

int  in_set_font_id_by_font_name(const void *font, const UCHAR *font_name, UCHAR font_id)
{
    GUI_FONT  *p = (GUI_FONT *)font;

    if ( p == 0 )
        return  0;


    return  0;
}

#ifndef  _LG_ALONE_VERSION_
int  set_font_id_by_font_name(const void *font, const UCHAR *font_name, UCHAR font_id)
{
    int  ret = 0;

    gui_lock( );
    ret =  in_set_font_id_by_font_name(font, font_name, font_id);
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */
#endif  /* _LG_FONT_ID_ */

#endif	/* _LG_FONT_ */
