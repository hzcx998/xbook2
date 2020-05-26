#include  <stdio.h>
#include  <string.h>

#include  <learninggui.h>

#include  "message_routine.h"
#include  "counter.h"

#include  "win_comm.h"
#include  "win_frame1.h"


#define  APP_GROUP_BOX1_ID              0x02

#define  APP_NAME_LABEL_ID              0x03
#define  APP_NAME_LINE_EDIT_ID          0x04

#define  APP_DEPART_LABEL_ID            0x05
#define  APP_DEPART_LINE_EDIT_ID        0x06

#define  APP_MANGER_LABEL_ID            0x07
#define  APP_MANGER_CHECK_BOX_ID        0x08

#define  APP_GRADUATE_LABEL_ID          0x09
#define  APP_GRADUATE_CHECK_BOX_ID      0x0A

#define  APP_MARRIED_LABEL_ID           0x0B
#define  APP_MARRIED_RADIO_YES_ID       0x0C
#define  APP_MARRIED_LABEL_YES_ID       0x0D
#define  APP_MARRIED_RADIO_NO_ID        0x0E
#define  APP_MARRIED_LABEL_NO_ID        0x0F

#define  APP_LEVEL_LABEL_ID             0x10
#define  APP_LEVEL_RADIO_1_ID           0x11
#define  APP_LEVEL_LABEL_1_ID           0x12
#define  APP_LEVEL_RADIO_2_ID           0x13
#define  APP_LEVEL_LABEL_2_ID           0x14
#define  APP_LEVEL_RADIO_3_ID           0x15
#define  APP_LEVEL_LABEL_3_ID           0x16
#define  APP_LEVEL_RADIO_4_ID           0x17
#define  APP_LEVEL_LABEL_4_ID           0x18

#define  APP_FRAME2_BUTTON_ID           0x19
#define  APP_FRAME3_BUTTON_ID           0x1A
#define  APP_FRAME4_BUTTON_ID           0x1B
#define  APP_FRAME5_BUTTON_ID           0x1C


#define  X_LEFT                         2
#define  Y_TOP                          2

#define  X_DELTA                        6
#define  Y_DELTA                        30

#define  LABEL_WIDTH                    70
#define  LABEL_MINI_WIDTH               40

#define  WIDGET_WIDTH                   24
#define  WIDGET_DELTA                   2

#define  ROW_HEIGHT                     24
#define  ROW_SPACE                      4


static  HWND  ghframe1 = NULL;


static  HWND  ghymar   = NULL;
static  HWND  ghnmar   = NULL;
static  HWND  ghgmar   = NULL;

static  HWND  gh1lev   = NULL;
static  HWND  gh2lev   = NULL;
static  HWND  gh3lev   = NULL;
static  HWND  gh4lev   = NULL;
static  HWND  ghglev   = NULL;



/* Frame2 button message routine */
static  int  frame1_frame2_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame1_frame3_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame1_frame4_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame1_frame5_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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


/* Main frame message routine */
static  int  frame1_main_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE:
            /* GroupBox */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_GROUP_BOX1_ID;
            gcomp.left          = X_LEFT;
            gcomp.top           = Y_TOP;
            gcomp.right         = 310;
            gcomp.bottom        = 177;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.group_box.text, " Employee Information ");
            ginput.group_box.len = sizeof(" Employee Information ");
            ginput.group_box.left_offset = 20;

            group_box_create(ghframe1, &gcomp, &(ginput.group_box));


            /* Name */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_NAME_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA;
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Name:");
            ginput.label.len = sizeof("Name:");
            ginput.label.no_back_flag  = 1;

            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_NAME_LINE_EDIT_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA;
            gcomp.right         = gcomp.right + 299;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.line_edit.text, "Zhang San");
            ginput.line_edit.len = sizeof("Zhang San");

            line_edit_create(ghframe1, &gcomp, &(ginput.line_edit));


            /* Depart */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_DEPART_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + (ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1 ;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Depart:");
            ginput.label.len = sizeof("Depart:");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_DEPART_LINE_EDIT_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + (ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.right + 299;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.line_edit.text, "Software Department");
            ginput.line_edit.len = sizeof("Software Department");

            /* ghdept = */ line_edit_create(ghframe1, &gcomp, &(ginput.line_edit));


            /* Manager */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MANGER_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 2*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Manager:");
            ginput.label.len = sizeof("Manager:");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MANGER_CHECK_BOX_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 2*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + WIDGET_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.check_box.state = 1;
            check_box_create(ghframe1, &gcomp, &(ginput.check_box));

            /* College graduate */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_GRADUATE_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_WIDTH + 5*WIDGET_DELTA + 20 ;
            gcomp.top           = Y_TOP + Y_DELTA + 2*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1 + 70;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "College Graduate:");
            ginput.label.len = sizeof("College Graduate:");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_GRADUATE_CHECK_BOX_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 8*WIDGET_WIDTH + 3*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 2*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + WIDGET_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.check_box.state         = 0;

            /* ghgrad =*/ check_box_create(ghframe1, &gcomp, &(ginput.check_box));

            /* Married widget group */
            ghgmar = widget_group_create(ghframe1);

            /* Married */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MARRIED_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 3*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left  + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top   + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Married:");
            ginput.label.len = sizeof("Married:");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MARRIED_RADIO_YES_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 3*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + WIDGET_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.radio_button.radius_offset = RBTN_RADIUS_OFFSET;
            ghymar = radio_button_create(ghframe1, &gcomp, &(ginput.radio_button));

            attach_widget(ghymar, ghgmar);

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MARRIED_LABEL_YES_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 1*WIDGET_WIDTH + 2*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 3*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left  + LABEL_MINI_WIDTH - 1 - 10;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Yes");
            ginput.label.len = sizeof("Yes");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MARRIED_RADIO_NO_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 2*WIDGET_WIDTH + 3*WIDGET_DELTA + 10;
            gcomp.top           = Y_TOP + Y_DELTA + 3*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + WIDGET_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.radio_button.radius_offset = RBTN_RADIUS_OFFSET;

            ghnmar = radio_button_create(ghframe1, &gcomp, &(ginput.radio_button));

            attach_widget(ghnmar, ghgmar);
            radio_button_set_state(ghnmar, 1);

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MARRIED_LABEL_NO_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 3*WIDGET_WIDTH + 4*WIDGET_DELTA + 10;
            gcomp.top           = Y_TOP + Y_DELTA + 3*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left  + LABEL_MINI_WIDTH - 1 - 10;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "No");
            ginput.label.len = sizeof("No");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));


            /* Level widget group */
            ghglev = widget_group_create(ghframe1);

            /* Level */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Level:");
            ginput.label.len = sizeof("Level:");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_RADIO_1_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + WIDGET_WIDTH - 1;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.radio_button.radius_offset = RBTN_RADIUS_OFFSET;

            gh1lev = radio_button_create(ghframe1, &gcomp, &(ginput.radio_button));

            attach_widget(gh1lev, ghglev);

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_LABEL_1_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_WIDTH + 2*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left  + LABEL_MINI_WIDTH - 1 - 20;
            gcomp.bottom        = gcomp.top   + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "1");
            ginput.label.len = sizeof("1");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_RADIO_2_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 2*WIDGET_WIDTH + 3*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + WIDGET_WIDTH - 1;
            gcomp.bottom        = gcomp.top   + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.radio_button.radius_offset = RBTN_RADIUS_OFFSET;

            gh2lev = radio_button_create(ghframe1, &gcomp, &(ginput.radio_button));

            attach_widget(gh2lev, ghglev);

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_LABEL_2_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 3*WIDGET_WIDTH + 4*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left  + LABEL_MINI_WIDTH - 1 - 20;
            gcomp.bottom        = gcomp.top   + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "2");
            ginput.label.len = sizeof("2");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_RADIO_3_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 4*WIDGET_WIDTH + 5*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + WIDGET_WIDTH - 1;
            gcomp.bottom        = gcomp.top   + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.radio_button.radius_offset = RBTN_RADIUS_OFFSET;

            gh3lev = radio_button_create(ghframe1, &gcomp, &(ginput.radio_button));

            attach_widget(gh3lev, ghglev);
            radio_button_set_state(gh3lev, 1);

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_LABEL_3_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 5*WIDGET_WIDTH + 6*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left  + LABEL_MINI_WIDTH - 1 - 20;
            gcomp.bottom        = gcomp.top   + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "3");
            ginput.label.len = sizeof("3");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_RADIO_4_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 6*WIDGET_WIDTH + 7*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + WIDGET_WIDTH - 1;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.radio_button.radius_offset = RBTN_RADIUS_OFFSET;

            gh4lev = radio_button_create(ghframe1, &gcomp, &(ginput.radio_button));

            attach_widget(gh4lev, ghglev);

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_LEVEL_LABEL_4_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + 7*WIDGET_WIDTH + 8*WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + 4*(ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left  + LABEL_MINI_WIDTH - 1 - 20;
            gcomp.bottom        = gcomp.top   + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "4");
            ginput.label.len = sizeof("4");
            ginput.label.no_back_flag = 1;
            label_create(ghframe1, &gcomp, &(ginput.label));

            /* frame2 */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FRAME2_BUTTON_ID;
            gcomp.left          = 15;
            gcomp.top           = 180;
            gcomp.right         = 74;
            gcomp.bottom        = 209;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame1_frame2_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame2");
            ginput.push_button.len = sizeof("Frame2");
            push_button_create(ghframe1, &gcomp, &(ginput.push_button));

            /* frame3 */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FRAME3_BUTTON_ID;
            gcomp.left          = 90;
            gcomp.top           = 180;
            gcomp.right         = 149;
            gcomp.bottom        = 209;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame1_frame3_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame3");
            ginput.push_button.len = sizeof("Frame3");
            push_button_create(ghframe1, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame1_frame4_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame4");
            ginput.push_button.len = sizeof("Frame4");
            push_button_create(ghframe1, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame1_frame5_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame5");
            ginput.push_button.len = sizeof("Frame5");
            push_button_create(ghframe1, &gcomp, &(ginput.push_button));
            break;

        case MSG_CLOSE_NEXT:           
           ghframe1 = NULL;
           return  0;

        default:
            break;
    }

    return  1;
}

/* Create frame1 */    
HWND   create_frame1(void)
{
    if ( ghframe1 != NULL )
        close_frame1();

    memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
    gcomp.id             = 1;
    gcomp.left           = 0;
    gcomp.top            = 0;
    gcomp.right          = 319;
    gcomp.bottom         = 239;

    gcomp.style          = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | WINBAR_STYLE;
    gcomp.ext_style      = 0;

    gcomp.is_app_callback = 1;
    gcomp.app_callback    = frame1_main_routine;

    memset(&ginput, 0, sizeof(WIDGET_UNION));
    memcpy(ginput.frame.text, "Demo frame1", sizeof("Demo frame1"));
    ginput.frame.len = sizeof("Demo frame1");

    ghframe1 = frame_create(NULL, &gcomp, &(ginput.frame));

    return  ghframe1;
}

int   close_frame1(void)
{
    win_close(ghframe1);
    ghframe1 = NULL;

    ghymar  = NULL;
    ghnmar  = NULL;
    ghgmar  = NULL;

    gh1lev  = NULL;
    gh2lev  = NULL;
    gh3lev  = NULL;
    gh4lev  = NULL;
    ghglev  = NULL;

    return  1;
}
