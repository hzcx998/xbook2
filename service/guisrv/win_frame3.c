#include  <stdio.h>
#include  <string.h>

#include  <learninggui.h>

#include  "message_routine.h"
#include  "counter.h"

#include  "win_comm.h"
#include  "win_frame3.h"


#define  APP_HPRESSURE_LABEL_ID         0x42
#define  APP_HPRESSURE_ID               0x43

#define  APP_HSLIDER_LABEL_ID           0x44
#define  APP_HSLIDER_ID                 0x45

#define  APP_VVOLUME_LABEL_ID           0x46
#define  APP_VVOLUME_ID                 0x47

#define  APP_VSLIDER_LABEL_ID           0x48
#define  APP_VSLIDER_ID                 0x49

#define  APP_FRAME1_BUTTON_ID           0x4A
#define  APP_FRAME2_BUTTON_ID           0x4B
#define  APP_FRAME4_BUTTON_ID           0x4C
#define  APP_FRAME5_BUTTON_ID           0x4D


#define  X_LEFT                         2
#define  Y_TOP                          2

#define  X_DELTA                        0
#define  Y_DELTA                        4

#define  LABEL_WIDTH                    110

#define  WIDGET_WIDTH                   24
#define  WIDGET_DELTA                   2

#define  ROW_HEIGHT                     24
#define  ROW_SPACE                      5


        
static  HWND   ghframe3 = NULL;

static  HWND   ghhpre   = NULL;
static  HWND   ghvvol   = NULL;



/* Oil hpressure message routine */
static  int  frame3_oil_hpressure_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE_NEXT:
            break;

        default:
            break;
    }

    return  1;
}

/* Hslider message routine */
static  int  frame3_hslider_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND          p       = NULL;
    unsigned int  percent = 0;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE_NEXT:
            progress_bar_get_percent(ghhpre, &percent);
            slider_bar_set_percent(p, percent);
            break;

        case  MSG_NOTIFY_VALUE_CHANGED:
            slider_bar_get_percent(p, &percent);
            progress_bar_set_percent(ghhpre, percent);
            break;

        default:
            break;
    }

    return  1;
}

/* Vslider message routine */
static  int  frame3_vslider_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND          p       = NULL;
    unsigned int  percent = 0;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE_NEXT:
            progress_bar_get_percent(ghvvol, &percent);
            slider_bar_set_percent(p, percent);
            break;

        case  MSG_NOTIFY_VALUE_CHANGED:
            slider_bar_get_percent(p, &percent);
            progress_bar_set_percent(ghvvol, percent);
            break;

        default:
            break;
    }

    return  1;
}

/* Vvolume message routine */
static  int  frame3_vvolume_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE_NEXT:
            break;

        default:
            break;
    }

    return  1;
}

/* Frame1 button message routine */
static  int  frame3_frame1_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame3_frame2_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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

/* Frame4 button message routine */
static  int  frame3_frame4_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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

/* Frame5 button message routine */
static  int  frame3_frame5_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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

/* Frame3 main message routine */
static  int  frame3_main_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p  = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE:
            /* Oil pressure */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_HPRESSURE_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA;
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Oil Pressure:");
            ginput.label.len = sizeof("Oil Pressure:");

            label_create(ghframe3, &gcomp, &(ginput.label));


            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id              = APP_HPRESSURE_ID;
            gcomp.left            = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA;
            gcomp.top             = Y_TOP + Y_DELTA;
            gcomp.right           = gcomp.left + 192;
            gcomp.bottom          = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style       = PGBAR_HBAR_STYLE;
            gcomp.is_app_callback = 1;
            gcomp.app_callback    = frame3_oil_hpressure_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.progress_bar.min_value       = 0;
            ginput.progress_bar.max_value       = 100000;
            ginput.progress_bar.current_value   =  69528;
            ginput.progress_bar.is_display_text = 1;
            ginput.progress_bar.display_style   = PGBAR_DISPLAY_PERCENT_STYLE;
            ginput.progress_bar.decimal_digits  = 2;

            ghhpre = progress_bar_create(ghframe3, &gcomp, &(ginput.progress_bar));

            /* HSlider */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_HSLIDER_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + (ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "HSlider:"); 
            ginput.label.len = sizeof("HSlider:"); 
            label_create(ghframe3, &gcomp, &(ginput.label));


            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id              = APP_HSLIDER_ID;
            gcomp.left            = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA;
            gcomp.top             = Y_TOP + Y_DELTA + (ROW_HEIGHT+ROW_SPACE);
            gcomp.right           = gcomp.left + 192;
            gcomp.bottom          = gcomp.top + ROW_HEIGHT - 1 + 6;
            gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style       = SLBAR_HBAR_STYLE;
            gcomp.is_app_callback = 1;
            gcomp.app_callback    = frame3_hslider_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.slider_bar.min_value       = 0;
            ginput.slider_bar.max_value       = 100000;
            ginput.slider_bar.current_value   = 30000;
            ginput.slider_bar.step_value      = 5000;
            ginput.slider_bar.decimal_digits  = 2;
            ginput.slider_bar.ruler_height    = SLBAR_RULER_HEIGHT;
            ginput.slider_bar.slot_height     = SLBAR_SLOT_HEIGHT;
            ginput.slider_bar.tick_width      = SLBAR_TICK_WIDTH;

            slider_bar_create(ghframe3, &gcomp, &(ginput.slider_bar));


            /* Volume */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_VVOLUME_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 2*(ROW_HEIGHT+2*ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Volume:");
            ginput.label.len = sizeof("Volume:");

            label_create(ghframe3, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id              = APP_HPRESSURE_ID;
            gcomp.left            = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA - 45;
            gcomp.top             = Y_TOP + Y_DELTA + 2*(ROW_HEIGHT+2*ROW_SPACE);
            gcomp.right           = gcomp.left + 42;
            gcomp.bottom          = gcomp.top + ROW_HEIGHT - 1 + 75;
            gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style       = PGBAR_VBAR_STYLE;
            gcomp.is_app_callback = 1;
            gcomp.app_callback    = frame3_vvolume_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.progress_bar.min_value       = 0;
            ginput.progress_bar.max_value       = 200;
            ginput.progress_bar.current_value   = 120;
            ginput.progress_bar.is_display_text = 1;
            ginput.progress_bar.display_style   = PGBAR_DISPLAY_PERCENT_STYLE;
            ginput.progress_bar.decimal_digits  = 0;

            ghvvol = progress_bar_create(ghframe3, &gcomp, &(ginput.progress_bar));


            /* VSlider */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_VSLIDER_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA + 150;
            gcomp.top           = Y_TOP + Y_DELTA + 2*(ROW_HEIGHT+2*ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "VSlider:"); 
            ginput.label.len = sizeof("VSlider:"); 
            label_create(ghframe3, &gcomp, &(ginput.label));


            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id              = APP_VSLIDER_ID;
            gcomp.left            = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA - 45 + 160;
            gcomp.top             = Y_TOP + Y_DELTA + 2*(ROW_HEIGHT+2*ROW_SPACE);
            gcomp.right           = gcomp.left + 42;
            gcomp.bottom          = gcomp.top + ROW_HEIGHT - 1 + 75;
            gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style       = SLBAR_VBAR_STYLE;
            gcomp.is_app_callback = 1;
            gcomp.app_callback    = frame3_vslider_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.slider_bar.min_value       = 0;
            ginput.slider_bar.max_value       = 1000;
            ginput.slider_bar.current_value   = 300;
            ginput.slider_bar.step_value      = 100;
            ginput.slider_bar.decimal_digits  = 0;

            slider_bar_create(ghframe3, &gcomp, &(ginput.slider_bar));


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
            gcomp.app_callback  = frame3_frame1_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame1");
            ginput.push_button.len = sizeof("Frame1");
            push_button_create(ghframe3, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame3_frame2_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame2");
            ginput.push_button.len = sizeof("Frame2");
            push_button_create(ghframe3, &gcomp, &(ginput.push_button));

            /* frame4 */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FRAME4_BUTTON_ID;
            gcomp.left          = 165;
            gcomp.top           = 180;
            gcomp.right         = 224;
            gcomp.bottom        = 209;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame3_frame4_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame4");
            ginput.push_button.len = sizeof("Frame4");
            push_button_create(ghframe3, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame3_frame5_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame5");
            ginput.push_button.len = sizeof("Frame5");
            push_button_create(ghframe3, &gcomp, &(ginput.push_button));
            break;

        case  MSG_CLOSE_NEXT:           
            ghframe3 = NULL;
            return  0;

        default:
            break;
    }

    return  1;
}

/* Create frame3 */    
HWND   create_frame3(void)
{
    if ( ghframe3 != NULL )
        close_frame3();


    memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
    gcomp.id              = 1;
    gcomp.left            = 0;
    gcomp.top             = 0;
    gcomp.right           = 319;
    gcomp.bottom          = 239;

    gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE | WINBAR_STYLE;
    gcomp.ext_style       = 0;

    gcomp.is_app_callback = 1;
    gcomp.app_callback    = frame3_main_routine;

    memset(&ginput, 0, sizeof(WIDGET_UNION));
    memcpy(ginput.frame.text, "Demo widget(3)", sizeof("Demo widget(3)"));
    ginput.frame.len = sizeof("Demo widget(3)");

    ghframe3 = frame_create(NULL, &gcomp, &(ginput.frame));

    return  ghframe3;
}

int   close_frame3(void)
{
    win_close(ghframe3);
    ghframe3 = NULL;

    return  1;
}
