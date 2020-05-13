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

#include  <stdio.h>

#include  <lgmacro.h>
#include  <text_glyph.h>


#ifdef  _LG_TEXT_GLYPH_

#ifdef  _LG_MONO_CHARSET_FONT_
static  int  in_get_mono_charset_glyph(PDC pdc, const void *font, const TCHAR *str, unsigned int code_counter, unsigned int *position,
                                       unsigned char *buffer, unsigned int  buffer_len, unsigned int  *out_len)
{
    MONO_CHARSET_FONT  *first_mono_charset_font  = (MONO_CHARSET_FONT *)font;
    MONO_CHARSET_FONT  *start_mono_charset_font  = (MONO_CHARSET_FONT *)font;
    MONO_CHARSET_FONT  *mono_charset_font        = (MONO_CHARSET_FONT *)font;

    unsigned int    str_counter = code_counter;
    TCHAR           *pchar = 0;

    unsigned int    glyph_counter = 0;


    #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
    unsigned char   multi_byte_flag = 0;
    #endif

    unsigned int    width        = 0;
    unsigned int    height       = 0;
    unsigned int    index        = 0;
    unsigned int    n            = 0;
    unsigned int    pos          = 0;
    unsigned int    start_index  = 0;

    UCHAR           tchar   = 0;

    int             ret     = 0; 
    unsigned int    counter = 0;



    if ( mono_charset_font == 0 )
    {
        *out_len = 0;
        return  -1;
    }

    width   = mono_charset_font->width;
    height  = mono_charset_font->height;

    counter = (height*width)/8;
    if ( ((height*width)%8) > 0 )
        counter += 1;


    index         = 0;
    pos           = 0;
    *position     = 0;

    glyph_counter = 0;


    for ( n = 0; n < str_counter; n++ )
    {
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        multi_byte_flag = 0;
        #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */

        start_mono_charset_font = mono_charset_font;

        pchar = (TCHAR *)(&str[n]);

        if ( (*pchar == LF) || (*pchar == CR) )
        {   
            pos++;
            continue; 
        }

        if ( *pchar == TAB )
        {
            in_text_glyph_tab(); 
            pos++;
            if ( glyph_counter >= buffer_len )
            {
                 glyph_counter = buffer_len;
                 goto  MONO_CHARSET_FONT_END;
            }
            continue; 
        }

        if ( *pchar == NULL_CHAR )
        {
            pos++;
            continue; 
        }

        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( (unsigned char)(*pchar) >= MULTI_BYTE_START_CODE )
        {
            multi_byte_flag = 1;
            n++;
        } 
        #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */


        MONO_CHARSET_FONT_START:
        ret = mono_charset_font->is_in_this_charset_block(pchar);
        if ( ret < 0 )
            goto  MONO_CHARSET_FONT_NEXT;


        start_index   = mono_charset_font->get_data_start_index(pchar);
        for ( index = 0; index < counter; index++ )
        {    
            tchar = *((UCHAR *)(mono_charset_font->data + start_index + index));
            *(buffer + glyph_counter) = tchar;
            glyph_counter++;
            if ( glyph_counter >= buffer_len )
            {
                 glyph_counter = buffer_len;
                 goto  MONO_CHARSET_FONT_END;
            }
        }

        pos++;
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( multi_byte_flag )
            pos++;
        #endif

        continue;

 
        MONO_CHARSET_FONT_NEXT:
        mono_charset_font = mono_charset_font->next;
        if ( mono_charset_font == start_mono_charset_font )
            goto  MONO_CHARSET_FONT_END;


        if ( mono_charset_font == 0 )
        {
            mono_charset_font = first_mono_charset_font;
            if ( mono_charset_font == start_mono_charset_font )
                goto  MONO_CHARSET_FONT_END;
            
            width   = mono_charset_font->width;
            height  = mono_charset_font->height;
            index  = 0;

            goto  MONO_CHARSET_FONT_START;
        }

        width        = mono_charset_font->width;
        height       = mono_charset_font->height;

        index = 0;

        goto  MONO_CHARSET_FONT_START;
   
    }

    MONO_CHARSET_FONT_END:
    *position = pos; 
    *out_len  = glyph_counter;

    return  1;
}
#endif  /* _LG_MONO_CHARSET_FONT_ */


#ifdef  _LG_MIXED_CHARSET_FONT_
static  int  in_get_mixed_charset_glyph(PDC pdc, const void *font, const TCHAR *str, unsigned int code_counter, unsigned int *position,
                                       unsigned char *buffer, unsigned int  buffer_len, unsigned int  *out_len)
{
    MIXED_CHARSET_FONT  *first_mixed_charset_font  = (MIXED_CHARSET_FONT *)font;
    MIXED_CHARSET_FONT  *start_mixed_charset_font  = (MIXED_CHARSET_FONT *)font;
    MIXED_CHARSET_FONT  *mixed_charset_font        = (MIXED_CHARSET_FONT *)font;

    unsigned int    str_counter = code_counter;

    MIXED_CHARSET_CHAR  *pcharset = 0;
    TCHAR               *pchar    = 0;

    unsigned int  glyph_counter = 0;


    #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
    unsigned char multi_byte_flag = 0;
    #endif /* _LG_MULTI_BYTE_CODE_VERSION_ */

    unsigned int  width             = 0;
    unsigned int  height            = 0;
    unsigned int  index             = 0;
    unsigned int  n                 = 0;
    unsigned int  pos               = 0;
    unsigned int  start_char_index  = 0;

    unsigned int  counter = 0;

    UCHAR         tchar   = 0;
             int  ret     = 0;


    if ( mixed_charset_font == 0 )
    {
        *out_len = 0;
        return  -1;
    }

    index     = 0;
    pos       = 0;
    *position = 0;

    for ( n = 0; n < str_counter; n++ )
    {
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        multi_byte_flag = 0;
        #endif

        start_mixed_charset_font = mixed_charset_font;

        pchar = (TCHAR *)(&str[n]);

        if ( *pchar == LF )
        {   
            pos++;
            continue; 
        }

        if ( *pchar == CR )
        { 
            pos++;
            continue; 
        }

        if ( *pchar == TAB )
        {
            in_text_glyph_tab(); 
            pos++;
            if ( glyph_counter >= buffer_len )
            {
                 glyph_counter = buffer_len;
                 goto  MIXED_CHARSET_FONT_END;
            }
            continue; 
        }

        if ( *pchar == NULL_CHAR )
        {
            pos++;
            continue; 
        }

        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( (unsigned char)(*pchar) >= MULTI_BYTE_START_CODE  )
        {
            multi_byte_flag = 1;
            n++;
        } 
        #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */


        MIXED_CHARSET_FONT_START:
        ret = mixed_charset_font->is_in_this_charset_block(pchar);
        if ( ret < 0 )
            goto  MIXED_CHARSET_FONT_NEXT;

 
        start_char_index = mixed_charset_font->get_code_index(pchar);

        pcharset  = (mixed_charset_font)->mixed_char;
        pcharset  += start_char_index;

        width    = pcharset->width;
        height   = pcharset->height;
    
        counter = (width*height)/8;
        if ( ((width*height)%8) > 0 )
            counter += 1;


        for ( index = 0; index < counter; index++ )
        {   
            tchar =  *((UCHAR *)((pcharset->data) + index));
            *(buffer + glyph_counter) = tchar;
            glyph_counter++;
            if ( glyph_counter >= buffer_len )
            {
                 glyph_counter = buffer_len;
                 goto  MIXED_CHARSET_FONT_END;
            }
        }

        pos++;
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( multi_byte_flag )
            pos++;
        #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */

        continue;

   
        MIXED_CHARSET_FONT_NEXT: 
        mixed_charset_font = mixed_charset_font->next;
        if ( mixed_charset_font == start_mixed_charset_font )
            goto  MIXED_CHARSET_FONT_END;

        if ( mixed_charset_font == 0 )
        {
            mixed_charset_font = first_mixed_charset_font;
            if ( mixed_charset_font == start_mixed_charset_font )
                goto  MIXED_CHARSET_FONT_END;
            
            index  = 0;
            goto  MIXED_CHARSET_FONT_START;
        }

        index = 0;
        goto  MIXED_CHARSET_FONT_START;
   
    }

    MIXED_CHARSET_FONT_END:
    *position = pos; 
    *out_len  = glyph_counter;

    return  1;

}
#endif  /* _LG_MIXED_CHARSET_FONT_ */


#ifdef  _LG_MONO_DISCRETE_FONT_
static  int  in_get_mono_discrete_glyph(PDC pdc, const void *font, const TCHAR *str, unsigned int code_counter, unsigned int *position,
                                        unsigned char *buffer, unsigned int  buffer_len, unsigned int  *out_len)
{
    MONO_DISCRETE_FONT  *first_mono_discrete_font  = (MONO_DISCRETE_FONT *)font;
    MONO_DISCRETE_FONT  *start_mono_discrete_font  = (MONO_DISCRETE_FONT *)font;
    MONO_DISCRETE_FONT  *mono_discrete_font        = (MONO_DISCRETE_FONT *)font;

    unsigned int  str_counter = code_counter;

    UCHAR         *pcharset = 0;
    TCHAR         *pchar = 0;
    unsigned int  char_counter = 0;

    unsigned int  glyph_counter = 0;

    #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
    unsigned char multi_byte_flag = 0;
    #endif

    unsigned int  width        = 0;
    unsigned int  height       = 0;
    unsigned int  index        = 0;
    unsigned int  n            = 0;
    unsigned int  pos          = 0;

    unsigned int  counter = 0;

    unsigned int  i = 0; 

    int    itemp   = 0;

    UCHAR  tchar   = 0;


    if ( mono_discrete_font == 0 )
        return  -1;

    char_counter = mono_discrete_font->char_counter;
    width        = mono_discrete_font->width;
    height       = mono_discrete_font->height;

    counter = (width*height)/8;
    if ( ((width*height)%8) > 0 )
        counter += 1;

    index     = 0;
    pos       = 0;
    *position = 0;


    for ( n = 0; n < str_counter; n++ )
    {
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        multi_byte_flag = 0;
        #endif

        start_mono_discrete_font = mono_discrete_font;

        pchar = (TCHAR *)(&str[n]);
        if ( *pchar == LF )
        {   
            pos++;
            continue; 
        }

        if ( *pchar == CR )
        { 
            pos++;
            continue; 
        }

        if ( *pchar == TAB )
        { 
            in_text_glyph_tab(); 
            pos++;
            if ( glyph_counter >= buffer_len )
            {
                 glyph_counter = buffer_len;
                 goto  MONO_DISCRETE_FONT_END;
            }
            pos++;
            continue; 
        }

        if ( *pchar == NULL_CHAR )
        {
            pos++;
            continue; 
        }

        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( (unsigned char)(*pchar) >= MULTI_BYTE_START_CODE  )
        {
            multi_byte_flag = 1;
            n++;
        } 
        #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */

        MONO_DISCRETE_FONT_START:
        pcharset = mono_discrete_font->mono_char; 
        for (i = 0; i < char_counter; i++)
        {
            #ifdef  _LG_UNICODE_VERSION_
                itemp = *((UINT16 *)pcharset) - *pchar;
            #else
                #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
                if ( multi_byte_flag )
                    itemp = strncmp((const char *)pcharset, pchar, 2*sizeof(UCHAR));
                else
                    itemp = strncmp((const char *)pcharset, pchar, sizeof(UCHAR));
                #else
                itemp = strncmp((const char *)pcharset, pchar, sizeof(UCHAR));
                #endif
            #endif
                
            if ( itemp == 0 )
            {
                for ( index = 0; index < counter; index++ )
                {  
                    tchar = *((UCHAR *)(pcharset+2*sizeof(UCHAR)+index));
                    *(buffer + glyph_counter) = tchar;
                    glyph_counter++;
                    if ( glyph_counter >= buffer_len )
                    {
                        glyph_counter = buffer_len;
                        goto  MONO_DISCRETE_FONT_END;
                    }
                }

                pos++;
                #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
                if ( multi_byte_flag )
                    pos++;
                #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */

                break;

            }
            pcharset += 2*sizeof(UCHAR) + counter;
        }

        if ( i >= char_counter ) 
        {
            mono_discrete_font = mono_discrete_font->next;
            if ( mono_discrete_font == start_mono_discrete_font )
                goto  MONO_DISCRETE_FONT_END;

            if ( mono_discrete_font == 0 )
            {
                mono_discrete_font = first_mono_discrete_font;
                if ( mono_discrete_font == start_mono_discrete_font )
                    goto  MONO_DISCRETE_FONT_END;

                char_counter = mono_discrete_font->char_counter;
                width        = mono_discrete_font->width;
                height       = mono_discrete_font->height;

                counter = (width*height)/8;
                if ( ((width*height)%8) > 0 )
                    counter += 1;

                index = 0;

                goto  MONO_DISCRETE_FONT_START;
            }
 
            char_counter = mono_discrete_font->char_counter;
            width        = mono_discrete_font->width;
            height       = mono_discrete_font->height;

            counter = (width*height)/8;
            if ( ((width*height)%8) > 0 )
                counter += 1;

            index = 0;

            goto  MONO_DISCRETE_FONT_START;
        }
    }

    MONO_DISCRETE_FONT_END:
    *position = pos; 
    *out_len  = glyph_counter;

    return  1;
}
#endif  /* _LG_MONO_DISCRETE_FONT_ */


#ifdef  _LG_MIXED_DISCRETE_FONT_
static  int  in_get_mixed_discrete_glyph(PDC pdc, const void *font, const TCHAR *str, unsigned int code_counter, unsigned int *position,
                                       unsigned char *buffer, unsigned int  buffer_len, unsigned int  *out_len)
{
    MIXED_DISCRETE_FONT  *first_mixed_discrete_font  = (MIXED_DISCRETE_FONT *)font;
    MIXED_DISCRETE_FONT  *start_mixed_discrete_font  = (MIXED_DISCRETE_FONT *)font;
    MIXED_DISCRETE_FONT  *mixed_discrete_font        = (MIXED_DISCRETE_FONT *)font;

    MIXED_DISCRETE_UINT16  *puint16 = 0;
    MIXED_DISCRETE_UCHAR2  *puchar2 = 0;
    TCHAR                  *ptchar  = 0;
    UCHAR                  *pdata   = 0;

    unsigned int   str_counter = code_counter;
    unsigned int   char_counter = 0;

    unsigned int   glyph_counter = 0;


    #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
    unsigned char multi_byte_flag = 0;
    #endif

    unsigned int  width    = 0;
    unsigned int  height   = 0;
    unsigned int  counter  = 0;
    unsigned int  index    = 0;
    unsigned int  n        = 0;
    unsigned int  pos      = 0;

    UCHAR           tchar  = 0;
    int             itemp  = 0;
   
    unsigned int    i     = 0; 


    if ( mixed_discrete_font == 0 )
        return  -1;

    char_counter = mixed_discrete_font->char_counter;

    index     = 0;
    pos       = 0;
    *position = 0;

    for ( n = 0; n < str_counter; n++ )
    {
        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        multi_byte_flag = 0;
        #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */

        start_mixed_discrete_font = mixed_discrete_font;

        ptchar = (TCHAR *)(&str[n]);

        if ( *ptchar == LF )
        {   
            pos++;
            continue; 
        }

        if ( *ptchar == CR )
        { 
            pos++;
            continue; 
        }

        if ( *ptchar == TAB )
        {
            in_text_glyph_tab(); 
            pos++;
            if ( glyph_counter >= buffer_len )
            {
                 glyph_counter = buffer_len;
                 goto  MIXED_DISCRETE_FONT_END;
            }
            pos++;
            continue; 
        }

        if ( *ptchar == NULL_CHAR )
        {
            pos++;
            continue; 
        }

        #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
        if ( (unsigned char)(*ptchar) >= MULTI_BYTE_START_CODE  )
        {
            multi_byte_flag = 1;
            n++;
        } 
        #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */

        MIXED_DISCRETE_FONT_START:
        puint16  = (MIXED_DISCRETE_UINT16 *)(mixed_discrete_font->char_list);
        puchar2  = (MIXED_DISCRETE_UCHAR2 *)(mixed_discrete_font->char_list);
        for (i = 0; i < char_counter; i++)
        {
            if ( (mixed_discrete_font->char_type) == MIXED_DISCRETE_UCHAR2_TYPE )
            {
                #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
                if ( multi_byte_flag )
                    itemp = strncmp((const char *)(puchar2->code), (const char *)ptchar, 2*sizeof(UCHAR));
                else
                    itemp = strncmp((const char *)(puchar2->code), (const char *)ptchar, sizeof(UCHAR));
                #else
                    itemp = strncmp((const char *)(puchar2->code), (const char *)ptchar, sizeof(UCHAR));
                #endif
            } else {
                itemp = puint16->code - *((UINT16 *)ptchar);
            }

            if ( itemp == 0 )
            {
                if ( (mixed_discrete_font->char_type) == MIXED_DISCRETE_UCHAR2_TYPE )
                {
                    width   = puchar2->width;
                    height  = puchar2->height;
                    pdata   = puchar2->data;
                } else {
                    width   = puint16->width;
                    height  = puint16->height;
                    pdata   = puint16->data;
                }

                counter = (width*height)/8;
                if ( ((width*height)%8) > 0 )
                    counter += 1;
  
                for ( index = 0; index < counter; index++ )
                { 
                    tchar  = *(pdata + index);
 
                    *(buffer + glyph_counter) = tchar;
                    glyph_counter++;
                    if ( glyph_counter >= buffer_len )
                    {
                        glyph_counter = buffer_len;
                        goto  MIXED_DISCRETE_FONT_END;
                    }
                }
 
                pos++;

                #ifdef  _LG_MULTI_BYTE_CODE_VERSION_
                if ( multi_byte_flag )
                    pos++;
                #endif  /* _LG_MULTI_BYTE_CODE_VERSION_ */

                break;

            }

            if ( (mixed_discrete_font->char_type) == MIXED_DISCRETE_UCHAR2_TYPE )
                puchar2++;
            else
                puint16++;

        }

        if ( i >= char_counter ) 
        {
            mixed_discrete_font = mixed_discrete_font->next;
            if ( mixed_discrete_font == start_mixed_discrete_font )
                goto  MIXED_DISCRETE_FONT_END;

            if ( mixed_discrete_font == 0 )
            {
                mixed_discrete_font = first_mixed_discrete_font;
                if ( mixed_discrete_font == start_mixed_discrete_font )
                    goto  MIXED_DISCRETE_FONT_END;

                char_counter = mixed_discrete_font->char_counter;

                index = 0;

                goto  MIXED_DISCRETE_FONT_START;
            }
 
            char_counter = mixed_discrete_font->char_counter;

            index = 0;

            goto  MIXED_DISCRETE_FONT_START;
        }
    }

    MIXED_DISCRETE_FONT_END:
    *position = pos; 
    *out_len  = glyph_counter;

    return  1;
}
#endif  /* _LG_MIXED_DISCRETE_FONT_ */


int  in_get_text_glyph(HDC hdc, const TCHAR *str, unsigned int code_counter, unsigned char *buffer, unsigned int buffer_len, unsigned int *out_len)
{
    PDC       pdc;
    GUI_FONT  *font;
    GUI_FONT  *start_font;
    TCHAR     *pstr;  

    unsigned char   *start_buffer;
    unsigned int    start_len = 0;
    unsigned int    glyph_len = 0;
    unsigned int    real_len  = 0;

    unsigned int    raw_len  = code_counter;
    unsigned int    cur_len  = code_counter;
    unsigned int    ret_len  = 0;

    unsigned int    position = 0;


    if ( raw_len < 1 )
    {
        *out_len = 0;
        return  0;
    }

    pdc = in_hdc_to_pdc(hdc);

    font = pdc->font;
    start_font = font;
    if ( font == 0 )
        goto  TEXT_OUT_EXIT;


    pstr         = (TCHAR *)str;
    ret_len      = 0;

    start_buffer = buffer;
    start_len    = buffer_len;
    glyph_len    = 0;
    real_len     = 0;


    TEXT_OUT_START:
    if ( (font->font_type < MIN_FONT_TYPE ) || (font->font_type > MAX_FONT_TYPE) )
        goto  TEXT_OUT_EXIT;

    position = 0;


    #ifdef  _LG_MONO_CHARSET_FONT_
    if ( (font->font_type) == MONO_CHARSET_FONT_TYPE )
        in_get_mono_charset_glyph(pdc, font->font, pstr, cur_len, &position, start_buffer, start_len, &glyph_len);
    #endif  /* _LG_MONO_CHARSET_FONT_ */

    #ifdef  _LG_MIXED_CHARSET_FONT_
    if ( (font->font_type) == MIXED_CHARSET_FONT_TYPE )
        in_get_mixed_charset_glyph(pdc, font->font, pstr, cur_len, &position, start_buffer, start_len, &glyph_len);
    #endif  /* _LG_MIXED_CHARSET_FONT_ */

    #ifdef  _LG_MONO_DISCRETE_FONT_
    if ( (font->font_type) == MONO_DISCRETE_FONT_TYPE )
        in_get_mono_discrete_glyph(pdc, font->font, pstr, cur_len, &position, start_buffer, start_len, &glyph_len);
    #endif  /* _LG_MONO_DISCRETE_FONT_ */

    #ifdef  _LG_MIXED_DISCRETE_FONT_
    if ( (font->font_type) == MIXED_DISCRETE_FONT_TYPE )
        in_get_mixed_discrete_glyph(pdc, font->font, pstr, cur_len, &position, start_buffer, start_len, &glyph_len);
    #endif  /* _LG_MIXED_DISCRETE_FONT_ */


    pstr         += position;

    start_buffer += position;
    start_len    -= position;
    real_len     += glyph_len;

    ret_len      += position;
    cur_len       = raw_len - ret_len;


    if ( ret_len >= raw_len )
        goto  TEXT_OUT_EXIT;


    if ( real_len >= buffer_len )
    {
        real_len =  buffer_len;
        goto  TEXT_OUT_EXIT;
    }

    if ( position > 0 )
        start_font = font;
 
    font = font->next;
    if ( font == start_font )
        goto  TEXT_OUT_BLANK;

    if ( font != 0 )
        goto  TEXT_OUT_START;

    font = pdc->font;
    if ( font == start_font )
        goto  TEXT_OUT_BLANK;

    goto  TEXT_OUT_START;


    TEXT_OUT_BLANK:
    /* fix me */
    /* in_text_out_blank(pdc); */

    pstr++;
    glyph_len     = 0;

    font = pdc->font;
    start_font = font;

    goto  TEXT_OUT_START;


    TEXT_OUT_EXIT:
    *out_len = real_len;

    return  1;
}

#ifndef  _LG_ALONE_VERSION_
int  get_text_glyph(HDC hdc, const TCHAR *str, unsigned int code_counter, unsigned char *buffer, unsigned int buffer_len, unsigned int *out_len) 
{
    int  ret = 0;

    if ( str == 0 )
        return  -1;

    if ( code_counter < 1 )
        return  -1;

    if ( buffer == 0 )
        return  -1;
     
    if ( buffer_len < 1 )
        return  -1;

    if ( out_len == 0 )
        return  -1;


    gui_lock( );
    ret = in_get_text_glyph(hdc, str, code_counter, buffer, buffer_len, out_len); 
    gui_unlock( );

    return  ret;
}
#endif  /* _LG_ALONE_VERSION_ */

#endif  /* _LG_TEXT_GLYPH_ */
