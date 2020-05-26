#include  <stdio.h>
#include  <string.h>

#include  <learninggui.h>

#include  "message_routine.h"
#include  "counter.h"

#include  "win_comm.h"
#include  "win_frame5.h"


#define  APP_DIG_CELL_ID                0x82

#define  APP_FRAME1_BUTTON_ID           0x89
#define  APP_FRAME2_BUTTON_ID           0x8A
#define  APP_FRAME3_BUTTON_ID           0x8B
#define  APP_FRAME4_BUTTON_ID           0x8C


static  HWND    ghframe5 = NULL;


/* Frame1 button message routine */
static  int  frame5_frame1_button_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;
    int    key_value = 0;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_MTJT_LBUTTON_UP:
            close_all_frame();
            create_frame1();
            return  0;

        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);
            if ( key_value == GUI_KEY_ENTER )
            {
                close_all_frame();
                create_frame1();
            }
            return  0;

        default:
            break;
    }

    return  1;
}

/* Frame2 button message routine */
static  int  frame5_frame2_button_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;
    int    key_value = 0;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_MTJT_LBUTTON_UP:
            close_all_frame();
            create_frame2();
            return  0;

        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);
            if ( key_value == GUI_KEY_ENTER )
            {
                close_all_frame();
                create_frame2();
            }
            return  0;

        default:
            break;
    }

    return  1;
}

/* Frame3 button message routine */
static  int  frame5_frame3_button_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;
    int    key_value = 0;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_MTJT_LBUTTON_UP:
            close_all_frame();
            create_frame3();
            return  0;

        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);
            if ( key_value == GUI_KEY_ENTER )
            {
                close_all_frame();
                create_frame3();
            }
            return  0;

        default:
            break;
    }

    return  1;
}

/* Frame4 button message routine */
static  int  frame5_frame4_button_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;
    int    key_value = 0;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_MTJT_LBUTTON_UP:
            close_all_frame();
            create_frame4();
            return  0;

        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);
            if ( key_value == GUI_KEY_ENTER )
            {
                close_all_frame();
                create_frame4();
            }
            return  0;

        default:
            break;
    }

    return  1;
}

/* Cell message routine */
static  int  main_frame_cell_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND       p = NULL;
    HDC        hdc;
    GUI_RECT   rect;
    int        i;
    int        delta;
    char       str[16];


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_PAINT_NEXT:

            hdc = hdc_get_client(p);
            if ( hdc == NULL )
                return  0;


            #ifdef  _LG_WINDOW_
            #ifdef  _LG_CURSOR_
            cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
            #endif 
            #endif

            /* Draw background */
            hdc_set_back_color(hdc, GUI_BLACK);
            hdc_get_rect(hdc, &rect);
            rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);

            /* Draw grid */
            hdc_set_fore_color(hdc, GUI_GRAY);
            delta = (GUI_RECTH(&rect)-1)/10;
            for ( i = 1; i < 10; i++ )
                line(hdc, 0, GUI_RECTH(&rect)-1-delta*i, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1-delta*i );

            delta = (GUI_RECTW(&rect)-1)/15;
            for ( i = 1; i < 15; i++ )
                line(hdc, delta*i, 0, delta*i, GUI_RECTH(&rect)-1 );

            /* Draw percent */
            hdc_set_fore_color(hdc, GUI_WHITE);
            hdc_set_back_mode(hdc, MODE_TRANSPARENCY);
            delta = (GUI_RECTH(&rect)-1)/5;
            for ( i = 1; i < 5; i++ )
            {
                sprintf(str, " %d", 20*i);
                strcat(str, "%");
                text_out(hdc, 0, GUI_RECTH(&rect)-1-delta*i-6, str, -1);
            }

            /* Draw Sales Block */
            hdc_set_back_color(hdc, GUI_GREEN);
            delta = (GUI_RECTW(&rect)-1)/10;
            rect_fill(hdc, (GUI_RECTW(&rect)-1)/5, (GUI_RECTH(&rect)-1)/100*120, (GUI_RECTW(&rect)-1)/5 + delta, GUI_RECTH(&rect)-1);
            text_out(hdc, (GUI_RECTW(&rect)-1)/5, 10 , "Sales", -1);

            /* Draw RD Block */
            hdc_set_back_color(hdc, GUI_YELLOW);
            delta = (GUI_RECTW(&rect)-1)/10;
            rect_fill(hdc, (GUI_RECTW(&rect)-1)/5*2, (GUI_RECTH(&rect)-1)/100*150, (GUI_RECTW(&rect)-1)/5*2 + delta, GUI_RECTH(&rect)-1);
            text_out(hdc, (GUI_RECTW(&rect)-1)/5*2 + 5, 10 , "R&D", -1);

            /* Draw Workshop Block */
            hdc_set_back_color(hdc, GUI_CYAN);
            delta = (GUI_RECTW(&rect)-1)/10;
            rect_fill(hdc, (GUI_RECTW(&rect)-1)/5*3, (GUI_RECTH(&rect)-1)/100*80, (GUI_RECTW(&rect)-1)/5*3 + delta, GUI_RECTH(&rect)-1);
            text_out(hdc, (GUI_RECTW(&rect)-1)/5*3 - 10, 10 , "Workshop", -1);

            /* Draw HR Block */
            hdc_set_back_color(hdc, GUI_RED);
            delta = (GUI_RECTW(&rect)-1)/10;
            rect_fill(hdc, (GUI_RECTW(&rect)-1)/5*4, (GUI_RECTH(&rect)-1)/100*135, (GUI_RECTW(&rect)-1)/5*4 + delta, GUI_RECTH(&rect)-1);
            text_out(hdc, (GUI_RECTW(&rect)-1)/5*4 + 5, 10 , "HR", -1);

            hdc_release_client(p, hdc);

            #ifdef  _LG_WINDOW_
            #ifdef  _LG_CURSOR_
            cursor_maybe_refresh( );
            #endif
            #endif

            return  0;

        default:
            break;
    }

    return  1;
}

/* Frame5 main message routine */
static  int  frame5_main_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND  p = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE:
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id              = APP_DIG_CELL_ID;
            gcomp.left            = 3;
            gcomp.top             = 3;
            gcomp.right           = 309;
            gcomp.bottom          = 169;
            gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style       = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback    = main_frame_cell_routine;
            cell_create(ghframe5, &gcomp, NULL);

            /* frame1 */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FRAME1_BUTTON_ID;
            gcomp.left          = 15;
            gcomp.top           = 180;
            gcomp.right         = 74;
            gcomp.bottom        = 209;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame5_frame1_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame1");
            ginput.push_button.len = sizeof("Frame1");
            push_button_create(ghframe5, &gcomp, &(ginput.push_button));

            /* frame2 */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FRAME2_BUTTON_ID;
            gcomp.left          = 90;
            gcomp.top           = 180;
            gcomp.right         = 149;
            gcomp.bottom        = 209;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame5_frame2_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame2");
            ginput.push_button.len = sizeof("Frame2");
            push_button_create(ghframe5, &gcomp, &(ginput.push_button));

            /* frame3 */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FRAME3_BUTTON_ID;
            gcomp.left          = 165;
            gcomp.top           = 180;
            gcomp.right         = 224;
            gcomp.bottom        = 209;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame5_frame3_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame3");
            ginput.push_button.len = sizeof("Frame3");
            push_button_create(ghframe5, &gcomp, &(ginput.push_button));

            /* frame4 */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FRAME4_BUTTON_ID;
            gcomp.left          = 240;
            gcomp.top           = 180;
            gcomp.right         = 299;
            gcomp.bottom        = 209;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame5_frame4_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame4");
            ginput.push_button.len = sizeof("Frame4");
            push_button_create(ghframe5, &gcomp, &(ginput.push_button));
            break;

       case  MSG_CLOSE_NEXT:           
           ghframe5 = NULL;
           return  0;

        default:
            break;
    }

    return  1;
}

/* Create frame5 */    
HWND   create_frame5(void)
{
    if ( ghframe5 != NULL )
        close_frame5();

    memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
    gcomp.id              = 1;
    gcomp.left            = 0;
    gcomp.top             = 0;
    gcomp.right           = 319;
    gcomp.bottom          = 239;

    gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | WINBAR_STYLE;
    gcomp.ext_style       = 0;

    gcomp.is_app_callback = 1;
    gcomp.app_callback    = frame5_main_routine;

    memset(&ginput, 0, sizeof(WIDGET_UNION));
    strcpy(ginput.frame.text, "Demo cell");
    ginput.frame.len = sizeof("Demo cell");

    ghframe5 = frame_create(NULL, &gcomp, &(ginput.frame));

    return  ghframe5;
}

int   close_frame5(void)
{
    win_close(ghframe5);
    ghframe5 = NULL;

    return  1;
}
