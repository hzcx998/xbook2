#include  <stdio.h>
#include  <string.h>

#include  <learninggui.h>

#include  "message_routine.h"
#include  "counter.h"
#include  "sine.h"

#include  "win_comm.h"
#include  "win_frame4.h"


/* Counter ID */
#define   COUNTER_SINCE_ID              0x01

/* Widget ID */
#define  APP_NAME_EDIT_ID               0x61

#define  APP_FRAME1_BUTTON_ID           0x6A
#define  APP_FRAME2_BUTTON_ID           0x6B
#define  APP_FRAME3_BUTTON_ID           0x6C
#define  APP_FRAME5_BUTTON_ID           0x6D


static  HWND   ghframe4 = NULL;
static  int    goffset  = 0;



/* Frame1 button message routine */
static  int  frame4_frame1_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame4_frame2_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame4_frame3_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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

/* Frame5 button message routine */
static  int  frame4_frame5_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
            create_frame5();
            return  0;

        case  MSG_KEY_DOWN:
            key_value = MESSAGE_GET_KEY_VALUE(msg);
            if ( key_value == GUI_KEY_ENTER )
            {
                close_all_frame();
                create_frame5();
            }
            return  0;

        default:
            break;
    }

    return  1;
}

/* Frame4 main message routine */
static  int  frame4_main_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HDC          hdc;
    int          middle_y;
    int          i;
    int          value;
    GUI_RECT     rect;

    int          last_x = 0;
    int          last_y = 0;
    unsigned int counter_id = 0;
    GUI_COLOR    old_background;


    switch(MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE:
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
            gcomp.app_callback  = frame4_frame1_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame1");
            ginput.push_button.len = sizeof("Frame1");
            push_button_create(ghframe4, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame4_frame2_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame2");
            ginput.push_button.len = sizeof("Frame2");
            push_button_create(ghframe4, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame4_frame3_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame3");
            ginput.push_button.len = sizeof("Frame3");
            push_button_create(ghframe4, &gcomp, &(ginput.push_button));

            /* frame5 */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FRAME5_BUTTON_ID;
            gcomp.left          = 240;
            gcomp.top           = 180;
            gcomp.right         = 299;
            gcomp.bottom        = 209;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame4_frame5_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame5");
            ginput.push_button.len = sizeof("Frame5");
            push_button_create(ghframe4, &gcomp, &(ginput.push_button));
            break;

        case  MSG_CREATE_NEXT:
            gui_counter_create(COUNTER_SINCE_ID, 50, ghframe4);
            break;

        case  MSG_COUNTER:
            counter_id = MESSAGE_GET_COUNTER_ID(msg);
            switch(counter_id)
            {
                case  COUNTER_SINCE_ID:
                    hdc = hdc_get_client(ghframe4);
                    if ( hdc == NULL )
                        return  0;

                    #ifdef  _LG_WINDOW_
                    #ifdef  _LG_CURSOR_
                    cursor_maybe_restore_back_abs(hdc->rect.left, hdc->rect.top, hdc->rect.right, hdc->rect.bottom);
                    #endif 
                    #endif

                    rect = hdc->rect;
                    /* Tip */
                    rect.bottom -= 40;
                    middle_y = GUI_RECTH(&rect)/2;

                    /* ?? */
                    old_background = hdc_get_back_color(hdc);
                    hdc_set_back_color(hdc, GUI_BLACK);
                    rect_fill(hdc, 0, 0, GUI_RECTW(&rect)-1, GUI_RECTH(&rect)-1);
                    /* */

                    last_x = 0;
                    last_y = middle_y - sine(goffset*0x1000000)/0x600;
                    for ( i = 0; i < GUI_RECTW(&rect); i++ )
                    {
                       value = middle_y - sine((i+goffset)*0x1000000)/0x600;

                       hdc_set_fore_color(hdc, GUI_GREEN);
                       move_to(hdc, last_x, last_y); 
                       line_to(hdc, i, value); 

                       last_x = i;
                       last_y = value;
                    }


                    last_x = 0;
                    last_y = middle_y - sine(goffset*0x1500000)/0x900;
                    for ( i = 0; i < GUI_RECTW(&rect); i++ )
                    {
                       value = middle_y - sine((i+goffset)*0x1500000)/0x900;

                       hdc_set_fore_color(hdc, GUI_RED);
                       move_to(hdc, last_x, last_y); 
                       line_to(hdc, i, value); 

                       last_x = i;
                       last_y = value;
                    }

                    hdc_set_back_color(hdc, old_background);

                    hdc_release_win(ghframe4, hdc);
                    goffset++;
                   
                    #ifdef  _LG_WINDOW_
                    #ifdef  _LG_CURSOR_
                    cursor_maybe_refresh( );
                    #endif
                    #endif
                    return  1;

                default:
                    break;
            }
            break;

        case  MSG_CLOSE_NEXT: 
            gui_counter_delete(COUNTER_SINCE_ID);
            ghframe4 = NULL;
            return  0;

        default:
            break;
    }

    return  1;
}

/* Create frame4 */    
HWND   create_frame4(void)
{
    if ( ghframe4 != NULL )
        close_frame4();

    memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
    gcomp.id              = 1;
    gcomp.left            = 0;
    gcomp.top             = 0;
    gcomp.right           = 319;
    gcomp.bottom          = 239;

    gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | WINBAR_STYLE | WINBAR_CLOSE_BTN_STYLE;
    gcomp.ext_style       = 0;

    gcomp.is_app_callback = 1;
    gcomp.app_callback    = frame4_main_routine;

    memset(&ginput, 0, sizeof(WIDGET_UNION));
    strcpy(ginput.frame.text, "Demo sine curve");
    ginput.frame.len = sizeof("Demo sine curve");
 
    ghframe4 = frame_create(NULL, &gcomp, &(ginput.frame));

    return  ghframe4;
}

int   close_frame4(void)
{
    win_close(ghframe4);
    ghframe4 = NULL;

    return  1;
}
