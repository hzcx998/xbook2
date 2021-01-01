/**
 * @file mouse.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "mouse.h"
#if USE_MOUSE != 0

#include "xbrower_view.h"
#include "xbrower_screen.h"
#include "xbrower_render.h"
#include "xbrower_image.h"
#include <xbrower_bitmap.h>
#include <stdlib.h>

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

xbrower_view_t *lv_mouse_view = NULL;
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
    unsigned char *ibuf = xbrower_load_image("/res/cursor.png", &iw, &ih, NULL);
    lv_mouse_view = xbrower_view_create(xbrower_screen.width / 2, xbrower_screen.height / 2, iw, ih);
    xbrower_bitmap_t ibmp;
    xbrower_bitmap_init(&ibmp, iw, ih, (xbrower_color_t *) ibuf);
    xbrower_render_bitblt(lv_mouse_view, 0, 0, &ibmp, 0,0,iw, ih);
    free(ibuf);
    xbrower_view_move_upper_top(lv_mouse_view);
}

void lv_mouse_exit(void)
{
    xbrower_view_set_z(lv_mouse_view, -1);
    xbrower_view_destroy(lv_mouse_view);
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
void lv_mouse_handler(xbrower_msg_t *msg)
{
    switch(xbrower_msg_get_type(msg)) {
    case XGUI_MSG_MOUSE_LBTN_DOWN:
        left_button_down = true;
        last_x = xbrower_msg_get_mouse_x(msg) / MONITOR_ZOOM;
        last_y = xbrower_msg_get_mouse_y(msg) / MONITOR_ZOOM;
        break;
    case XGUI_MSG_MOUSE_LBTN_UP:
        left_button_down = false;
        break;
    case XGUI_MSG_MOUSE_MOTION:
        last_x = xbrower_msg_get_mouse_x(msg) / MONITOR_ZOOM;
        last_y = xbrower_msg_get_mouse_y(msg) / MONITOR_ZOOM;
        xbrower_view_set_xy(lv_mouse_view, last_x, last_y);
        break;
    default:
        break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
