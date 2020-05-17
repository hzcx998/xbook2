#include  <string.h>

#include  <learninggui.h>

#include  "gui_main.h"

#include  "lake_alpha.h"

#include  "guisrv.h"

static  GUI_VAR_CONST  TCHAR  ascii_latin_str1[] = "abcdefghijklmnopqrstuvwxyz";
static  GUI_VAR_CONST  TCHAR  ascii_latin_str2[] = "ABCDEFGHIJKLMNOPQRSTUVQXYZ";
static  GUI_VAR_CONST  TCHAR  ascii_latin_str3[] = "0123456789 ,.?=+-*/%!@#%&$"; 


GUI_VAR_CONST  GUI_FONT  ascltn = 
{
    /* .type = */ MONO_CHARSET_FONT_TYPE,
    /* .font = */ (void *)(&lasc08),
    /* .next = */ 0
};


int  paint_gui_main(void) 
{
    HDC   hdc = NULL;

    hdc = hdc_get_basic( );
    if ( hdc == NULL )
        return  -1;

    hdc_clear(hdc);

    hdc_set_font(hdc, &ascltn);
    text_out( hdc, 0, 0,  ascii_latin_str1, -1 ); 
    text_out( hdc, 0, 20, ascii_latin_str2, -1 ); 
    text_out( hdc, 0, 40, ascii_latin_str3, -1 ); 


    

    /* 1: draw red line */
    hdc_set_fore_color(hdc, GUI_RED);
    line(hdc, 0, 0, 99, 99);

    /* 2: draw green line */
    hdc_set_fore_color(hdc, GUI_GREEN);
    line(hdc, 5, 100, 69, 29);

    /* 3: draw blue line */
    hdc_set_fore_color(hdc, GUI_BLUE);
    line(hdc, 20, 60, 129, 15);


    /* 4: draw red rectranle */
    hdc_set_fore_color(hdc, GUI_RED);
    rect_frame(hdc, 20, 10, 69, 39);

    /* 5: draw green rectranle */
    hdc_set_fore_color(hdc, GUI_GREEN);
    rect_frame(hdc, 30, 20, 79, 49);

    /* 6: draw blue rectranle */
    hdc_set_fore_color(hdc, GUI_BLUE);
    rect_frame(hdc, 40, 30, 89, 69);


    /* 7: draw red filled rectranle */
    hdc_set_back_color(hdc, GUI_RED);
    rect_fill(hdc, 100, 10, 159, 39);

    /* 8: draw green filled rectranle */
    hdc_set_back_color(hdc, GUI_GREEN);
    rect_fill(hdc, 120, 20, 179, 49);

    /* 9: draw blue filled rectranle */
    hdc_set_back_color(hdc, GUI_BLUE);
    rect_fill(hdc, 140, 30, 199, 59);

    
    hdc_clear(hdc);

    bitmap_fill(hdc, 400,   0,   &lake_alpha);

    hdc_set_alpha_value(hdc, 128);
    /*
    hdc_set_alpha_op_mode(hdc, ALPHA_BLEND_OP_ADD);
    hdc_enable_alpha(hdc);
    */
    hdc_set_back_color(hdc, GUI_LIGHT_WHITE);
    rect_fill(hdc, 30, 50, 299, 199);



    hdc_release_basic( hdc );

    return  1;
}
 
int  message_user_main_routine(/* GUI_MESSAGE */ void *msg)
{
    int   key       = 0;
    int x, y;
    if ( msg == 0 )
        return  -1;

    switch ( MESSAGE_GET_ID(msg) )
    {
        case  MSG_KEY_DOWN:
            key = MESSAGE_GET_KEY_VALUE(msg);
            printf("%s: key down %d %c.\n", SRV_NAME, key, key);
            break;
        case  MSG_KEY_UP:
            key = MESSAGE_GET_KEY_VALUE(msg);
            printf("%s: key up %d %c.\n", SRV_NAME, key, key);
            break;
        case MSG_MTJT_MOVE:
            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            break;
            //printf("%s: mouse (%d, %d).\n", SRV_NAME, x, y);
        case MSG_MTJT_LBUTTON_DOWN:
            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            printf("%s: mouse left button down (%d, %d).\n", SRV_NAME, x, y);
            break;
        case MSG_MTJT_LBUTTON_UP:
            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            printf("%s: mouse left button up (%d, %d).\n", SRV_NAME, x, y);
            break;
        case MSG_MTJT_RBUTTON_DOWN:
            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            printf("%s: mouse right button down (%d, %d).\n", SRV_NAME, x, y);
            break;
        case MSG_MTJT_RBUTTON_UP:
            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            printf("%s: mouse right button up (%d, %d).\n", SRV_NAME, x, y);
            break;
        case MSG_MTJT_MBUTTON_DOWN:
            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            printf("%s: mouse middle button down (%d, %d).\n", SRV_NAME, x, y);
            break;
        case MSG_MTJT_MBUTTON_UP:
            x = MESSAGE_GET_MTJT_X(msg);
            y = MESSAGE_GET_MTJT_Y(msg);
            printf("%s: mouse middle button up (%d, %d).\n", SRV_NAME, x, y);
            break;
        
        default:
            break;
    }

    return  1;
}
