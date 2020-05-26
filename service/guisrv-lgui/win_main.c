#include  <stdio.h>
#include  <string.h>

#include  <learninggui.h>

#include  "message_routine.h"
#include  "counter.h"
#include  "zhuhai.h"


#define  APP_IMAGE_ID                0x02
#define  APP_QUIT_BUTTON_ID              0x03


HWND   ghmain  = NULL;

static  GUI_COMMON_WIDGET   gcomp;
static  WIDGET_UNION        ginput;


/* Image message routine */
static  int  main_frame_image_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        default:
            break;
    }

    return  1;
}

/* Quit message routine */
static  int  main_frame_quit_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;
    int    key_value = 0;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
       case  MSG_MTJT_LBUTTON_UP:
           message_post_quit();
           return  0;

       case  MSG_KEY_DOWN:
           key_value = MESSAGE_GET_KEY_VALUE(msg);
           if ( key_value == GUI_KEY_ENTER )
               message_post_quit();

           return  0;

        default:
            break;
    }

    return  1;
}

/* Main frame message routine */
static  int  main_frame_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE:
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id              = APP_IMAGE_ID;
            gcomp.left            = 3;
            gcomp.top             = 3;
            gcomp.right           = 309;
            gcomp.bottom          = 169;
            gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style       = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback    = main_frame_image_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.image.image_type  = IMAGE_BITMAP;
            ginput.image.image_align = IMAGE_ALIGN_FILL;
            ginput.image.pimage      = (const void *)(&zhuhai);
            image_create(ghmain, &gcomp, &(ginput.image));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_QUIT_BUTTON_ID;
            gcomp.left          = 180;
            gcomp.top           = 178;
            gcomp.right         = 259;
            gcomp.bottom        = 207;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = main_frame_quit_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Quit");
            ginput.push_button.len = sizeof("Quit");
            push_button_create(ghmain, &gcomp, &(ginput.push_button));
            break;

       case  MSG_CLOSE_NEXT:           
           ghmain = NULL;
           return  0;

        default:
            break;
    }

    return  1;
}

/* Create user main window */    
HWND   create_user_main_window(void)
{
    memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
    gcomp.id              = 1;
    gcomp.left            = 0;
    gcomp.top             = 0;
    gcomp.right           = 319;
    gcomp.bottom          = 239;

    gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | WINBAR_STYLE;
    gcomp.ext_style       = 0;

    gcomp.is_app_callback = 1;
    gcomp.app_callback    = main_frame_routine;

    memset(&ginput, 0, sizeof(WIDGET_UNION));
    memcpy(ginput.frame.text, "Demo bitmap", sizeof("Demo bitmap"));
    ginput.frame.len = sizeof("Demo bitmap");

    ghmain = frame_create(NULL, &gcomp, &(ginput.frame));

    return  ghmain;
}
