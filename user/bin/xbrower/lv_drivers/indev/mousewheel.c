/**
 * @file mousewheel.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "mousewheel.h"
#if USE_MOUSEWHEEL

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static int16_t enc_diff = 0;
static lv_indev_state_t state = LV_INDEV_STATE_REL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the mousewheel
 */
void lv_mousewheel_init(void)
{
    /*Nothing to init*/
}

/**
 * Get encoder (i.e. mouse wheel) ticks difference and pressed state
 * @param indev_drv pointer to the related input device driver
 * @param data store the read data here
 * @return false: all ticks and button state are handled
 */
bool lv_mousewheel_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/

    data->state = state;
    data->enc_diff = enc_diff;
    enc_diff = 0;

    return false;       /*No more data to read so return false*/
}

/**
 * It is called periodically from the SDL thread to check mouse wheel state
 * @param event describes the event
 */
void lv_mousewheel_handler(xgui_msg_t *msg)
{
    switch(xgui_msg_get_type(msg)) {
    case XGUI_MSG_MOUSE_WHEEL_UP:
        enc_diff = -1;
        break;
    case XGUI_MSG_MOUSE_WHEEL_DOWN:
        enc_diff = 1;
        break;
    case XGUI_MSG_MOUSE_MBTN_DOWN:
        state = LV_INDEV_STATE_PR;
        break;
    case XGUI_MSG_MOUSE_MBTN_UP:
        state = LV_INDEV_STATE_REL;
        break;
    default:
        break;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
