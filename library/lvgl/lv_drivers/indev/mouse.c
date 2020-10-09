/**
 * @file mouse.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "mouse.h"
#if USE_MOUSE != 0

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
void lv_mouse_handler(g_msg_t *msg)
{
    switch(g_msg_get_type(msg)) {
    case GM_MOUSE_LBTN_DOWN:
        left_button_down = true;
        last_x = g_msg_get_mouse_x(msg) / MONITOR_ZOOM;
        last_y = g_msg_get_mouse_y(msg) / MONITOR_ZOOM;
        break;
    case GM_MOUSE_LBTN_UP:
        left_button_down = false;
        break;
    case GM_MOUSE_MOTION:
        last_x = g_msg_get_mouse_x(msg) / MONITOR_ZOOM;
        last_y = g_msg_get_mouse_y(msg) / MONITOR_ZOOM;
        break;
    default:
        break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
