#include  <stdio.h>
#include  <string.h>

#include  <learninggui.h>

#include  "message_routine.h"
#include  "counter.h"

#include  "win_comm.h"
#include  "win_frame2.h"


#define  APP_GROUP_BOX1_ID              0x22

#define  APP_FROM_LABEL_ID              0x23
#define  APP_FROM_COM_BOX_ID            0x24

#define  APP_FAVORITE_LABEL_ID          0x25
#define  APP_FAVORITE_LIST_BOX_ID       0x26

#define  APP_MAJOR_LABEL_ID             0x27
#define  APP_MAJOR_LIST_BOX_ID          0x28

#define  APP_FRAME1_BUTTON_ID           0x29
#define  APP_FRAME3_BUTTON_ID           0x2A
#define  APP_FRAME4_BUTTON_ID           0x2B
#define  APP_FRAME5_BUTTON_ID           0x2C


#define  X_LEFT                       2
#define  Y_TOP                        2

#define  X_DELTA                      6
#define  Y_DELTA                      30

#define  LABEL_WIDTH                  80

#define  WIDGET_WIDTH                 24
#define  WIDGET_DELTA                 2

#define  ROW_HEIGHT                   24
#define  ROW_SPACE                    4


static  HWND   ghframe2 = NULL;

static  HWND   ghfavo   = NULL;
static  HWND   ghmajo   = NULL;


/* From message routine */
static  int  frame2_com_box_from_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   com_box_p  = NULL;
    HWND   list_box_p = NULL;
    int    ret        = 0;


    com_box_p = GET_TO_HWND_FROM_MSG(msg);
    if ( com_box_p == NULL )
        return  -1;

    ret = com_box_get_list_box_hwnd(com_box_p, &list_box_p);
    if ( ret < 1 )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE_NEXT:
            /* Notice: list_box p */
            list_box_add_item(list_box_p, "Beijing", 7);

            list_box_add_item(list_box_p, "Hongkong", 8);
            list_box_add_item(list_box_p, "Macau", 5);

            list_box_add_item(list_box_p, "Chongqing", 9);
            list_box_add_item(list_box_p, "Shanghai", 8);
            list_box_add_item(list_box_p, "Tianjin", 7);

            list_box_add_item(list_box_p, "Anhui", 5);

            list_box_add_item(list_box_p, "Fujian", 6);

            list_box_add_item(list_box_p, "Guizhou", 7);

            list_box_add_item(list_box_p, "Gansu", 5);
            list_box_add_item(list_box_p, "Guangdong", 9);
            list_box_add_item(list_box_p, "Guangxi", 7);

            list_box_add_item(list_box_p, "Jiangsu", 7);
            list_box_add_item(list_box_p, "Jiangxi", 7);
            list_box_add_item(list_box_p, "Jilin", 5);

            /* Notice: com_box p */
            com_box_list_box_add_item(com_box_p, "Hainan", 6);
            com_box_list_box_add_item(com_box_p, "Hebei", 5);
            com_box_list_box_add_item(com_box_p, "Heilongjiang", 12);
            com_box_list_box_add_item(com_box_p, "Henan", 5);
            com_box_list_box_add_item(com_box_p, "Hubei", 5);
            com_box_list_box_add_item(com_box_p, "Hunan", 5);

            /* Notice: list_box p */
            list_box_add_item(list_box_p, "Liaoning", 8);

            list_box_add_item(list_box_p, "Neimenggu", 9);
            list_box_add_item(list_box_p, "Ningxia", 7);

            list_box_add_item(list_box_p, "Qinghai", 7);

            list_box_add_item(list_box_p, "Sichuan", 7);

            list_box_add_item(list_box_p, "Shandong", 8);
            list_box_add_item(list_box_p, "Shanxi", 6);
            list_box_add_item(list_box_p, "Shanxi", 6);

            list_box_add_item(list_box_p, "Taiwan", 6);

            list_box_add_item(list_box_p, "Xinjiang", 8);
            list_box_add_item(list_box_p, "Xizang", 6);

            list_box_add_item(list_box_p, "Yunnan", 6);

            list_box_add_item(list_box_p, "Zhejiang", 8);

            /* Notic: must be com_box p */
            com_box_set_index_item(com_box_p, 0);

            break;

        default:
            break;
    }

    return  1;
}

/* Favorite message routine */
static  int  frame2_list_box_favorite_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE_NEXT:
            /* Add item */
            list_box_add_item(p, "Travel", 6);
            list_box_add_item(p, "Photo", 5);
            list_box_add_item(p, "Personal computer software, hardware and Internet chat", 54);
            list_box_add_item(p, "Reading and writing", 19);
            list_box_add_item(p, "Football", 8);
            list_box_add_item(p, "Bastball", 8);
            list_box_add_item(p, "Swimming", 8);
            list_box_add_item(p, "Table tennis ball", 17);
            list_box_add_item(p, "Chest", 5);
            list_box_add_item(p, "Majiang", 7);
            list_box_add_item(p, "Taiji", 5);
            list_box_add_item(p, "Cars", 4);
            list_box_add_item(p, "Stock", 5);
            list_box_add_item(p, "Films", 5);
            list_box_add_item(p, "Games", 5);
            list_box_add_item(p, "Foods", 5);

            /* Set selected index */
            list_box_set_selected_index(p, 2);
            break;

        default:
            break;
    }

    return  1;
}

/* Major message routine */
static  int  frame2_list_box_major_routine(/* GUI_MESSAGE *msg */ void *msg)
{
    HWND   p = NULL;


    p = GET_TO_HWND_FROM_MSG(msg);
    if ( p == NULL )
        return  -1;

    switch (MESSAGE_GET_ID(msg))
    {
        case  MSG_CREATE_NEXT:
            list_box_add_item(p, "Marketing", 9);
            list_box_add_item(p, "Optics", 6);
            list_box_add_item(p, "Computer", 9);
            list_box_add_item(p, "Electronics", 11);
            list_box_add_item(p, "Chemistry", 9);
            break;

        default:
            break;
    }

    return  1;
}

/* Frame1 button message routine */
static  int  frame2_frame1_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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

/* Frame3 button message routine */
static  int  frame2_frame3_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame2_frame4_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame2_frame5_button_routine(/* GUI_MESSAGE *msg */ void *msg)
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
static  int  frame2_main_routine(/* GUI_MESSAGE *msg */ void *msg)
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
            group_box_create(ghframe2, &gcomp, &(ginput.group_box));

            /* From */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FROM_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA - 6;
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top  + ROW_HEIGHT - 1;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "From:");
            ginput.label.len = sizeof("From:");
            label_create(ghframe2, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FROM_COM_BOX_ID;
            gcomp.left          = X_LEFT + X_DELTA + LABEL_WIDTH - 1 + WIDGET_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA - 6;
            gcomp.right         = gcomp.right + 307;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1 + 6;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame2_com_box_from_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.com_box.line_edit_style     = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            ginput.com_box.line_edit_ext_style = 0;
            ginput.com_box.list_box_style      = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE | SCBAR_VBAR_STYLE;
            ginput.com_box.list_box_ext_style  = 0;
            ginput.com_box.open_dir            = CBBOX_OPEN_DOWN;
            ginput.com_box.open_len            = 100;
         
            com_box_create(ghframe2, &gcomp, &(ginput.com_box));

            /* Favorite */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FAVORITE_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA;
            gcomp.top           = Y_TOP + Y_DELTA + (ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1 ;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;
            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Favorite:");
            ginput.label.len = sizeof("Favorite:");
            label_create(ghframe2, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_FAVORITE_LIST_BOX_ID;
            gcomp.left          = X_LEFT + X_DELTA  + WIDGET_DELTA - 1;
            gcomp.top           = Y_TOP + Y_DELTA + 2*ROW_HEIGHT;
            gcomp.right         = gcomp.left + 179;
            gcomp.bottom        = gcomp.top + 89;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE | SCBAR_HBAR_STYLE | SCBAR_VBAR_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback  = frame2_list_box_favorite_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.list_box.multi_flag = 1;

            ghfavo = list_box_create(ghframe2, &gcomp, &(ginput.list_box));

            list_box_set_read_only(ghfavo, 1);

            /* Major */
            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MAJOR_LABEL_ID;
            gcomp.left          = X_LEFT + X_DELTA + 189;
            gcomp.top           = Y_TOP + Y_DELTA + (ROW_HEIGHT+ROW_SPACE);
            gcomp.right         = gcomp.left + LABEL_WIDTH - 1;
            gcomp.bottom        = gcomp.top + ROW_HEIGHT - 1 ;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE;
            gcomp.ext_style     = 0;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.label.text, "Major:");
            ginput.label.len = sizeof("Major:");
            label_create(ghframe2, &gcomp, &(ginput.label));

            memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
            gcomp.id            = APP_MAJOR_LIST_BOX_ID;
            gcomp.left          = X_LEFT + X_DELTA  + WIDGET_DELTA - 1 + 189;
            gcomp.top           = Y_TOP + Y_DELTA + 2*ROW_HEIGHT;
            gcomp.right         = gcomp.left + 105;
            gcomp.bottom        = gcomp.top + 89;
            gcomp.style         = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | BORDER_3D_STYLE;
            gcomp.ext_style     = 0;
            gcomp.is_app_callback = 1;
            gcomp.app_callback    = frame2_list_box_major_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            ginput.list_box.multi_flag = 0;

            ghmajo = list_box_create(ghframe2, &gcomp, &(ginput.list_box));

            list_box_set_read_only(ghmajo, 1);


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
            gcomp.app_callback  = frame2_frame1_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame1");
            ginput.push_button.len = sizeof("Frame1");
            push_button_create(ghframe2, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame2_frame3_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame3");
            ginput.push_button.len = sizeof("Frame3");
            push_button_create(ghframe2, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame2_frame4_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame4");
            ginput.push_button.len = sizeof("Frame4");
            push_button_create(ghframe2, &gcomp, &(ginput.push_button));

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
            gcomp.app_callback  = frame2_frame5_button_routine;

            memset(&ginput, 0, sizeof(WIDGET_UNION));
            strcpy(ginput.push_button.text, "Frame5");
            ginput.push_button.len = sizeof("Frame5");
            push_button_create(ghframe2, &gcomp, &(ginput.push_button));
            break;

        case  MSG_CLOSE_NEXT:           
            ghframe2 = NULL;
            return  0;

        default:
            break;
    }

    return  1;
}

/* Create frame2 */    
HWND   create_frame2(void)
{
    if ( ghframe2 != NULL )
        close_frame2();

    memset(&gcomp, 0, sizeof(GUI_COMMON_WIDGET));
    gcomp.id              = 1;
    gcomp.left            = 0;
    gcomp.top             = 0;
    gcomp.right           = 319;
    gcomp.bottom          = 239;

    gcomp.style           = VISUAL_STYLE | ENABLE_STYLE | BORDER_STYLE | WINBAR_STYLE;
    gcomp.ext_style       = 0;

    gcomp.is_app_callback = 1;
    gcomp.app_callback    = frame2_main_routine;

    memset(&ginput, 0, sizeof(WIDGET_UNION));
    memcpy(ginput.frame.text, "Demo widget(2)", sizeof("Demo widget(2)"));
    ginput.frame.len = sizeof("Demo widget(2)");

    ghframe2 = frame_create(NULL, &gcomp, &(ginput.frame));

    return  ghframe2;
}

int   close_frame2(void)
{
    win_close(ghframe2);
    ghframe2 = NULL;

    return  1;
}
