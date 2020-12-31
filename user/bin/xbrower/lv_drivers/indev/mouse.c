/**
 * @file mouse.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "mouse.h"
#if USE_MOUSE != 0

#include "xgui_view.h"
#include "xgui_screen.h"
#include "xgui_vrender.h"
#include "xgui_image.h"
#include <xgui_bitmap.h>

/*********************
 *      DEFINES
 *********************/
#ifndef MONITOR_ZOOM
#define MONITOR_ZOOM    1
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static bool left_button_down = false;
static int16_t last_x = 0;
static int16_t last_y = 0;

xgui_view_t *lv_mouse_view = NULL;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the mouse
 */
void lv_mouse_init(void)
{
    int iw, ih;
    unsigned char *ibuf = xgui_load_image("/res/cursor.png", &iw, &ih, NULL);
    lv_mouse_view = xgui_view_create(xgui_screen.width / 2, xgui_screen.height / 2, iw, ih);
    xgui_bitmap_t ibmp;
    xgui_bitmap_init(&ibmp, iw, ih, (xgui_color_t *) ibuf);
    xgui_vrender_bitblt(lv_mouse_view, 0, 0, &ibmp, 0,0,iw, ih);
    free(ibuf);
    xgui_view_move_upper_top(lv_mouse_view);
}

/**
 * Get the current position and state of the mouse
 * @param indev_drv pointer to the related input device driver
 * @param data store the mouse data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool lv_mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/

    /*Store the collected data*/
    data->point.x = last_x;
    data->point.y = last_y;
    data->state = left_button_down ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

    return false;
}

/**
 * It will be called from the main SDL thread
 */
void lv_mouse_handler(xgui_msg_t *msg)
{
    switch(xgui_msg_get_type(msg)) {
    case XGUI_MSG_MOUSE_LBTN_DOWN:
        left_button_down = true;
        last_x = xgui_msg_get_mouse_x(msg) / MONITOR_ZOOM;
        last_y = xgui_msg_get_mouse_y(msg) / MONITOR_ZOOM;
        break;
    case XGUI_MSG_MOUSE_LBTN_UP:
        left_button_down = false;
        break;
    case XGUI_MSG_MOUSE_MOTION:
        last_x = xgui_msg_get_mouse_x(msg) / MONITOR_ZOOM;
        last_y = xgui_msg_get_mouse_y(msg) / MONITOR_ZOOM;
        xgui_view_set_xy(lv_mouse_view, last_x, last_y);
        break;
    default:
        break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
