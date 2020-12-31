/**
 * @file keyboard.h
 *
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif


/*********************
 *      INCLUDES
 *********************/
#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif
#endif

#if USE_KEYBOARD

#define LV_LVGL_H_INCLUDE_SIMPLE
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include <lvgl.h>
#else
#include "lvgl/lvgl.h"
#endif

#include <xgui_msg.h>
#include <sys/input.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * Initialize the keyboard
 */
void lv_keyboard_init(void);

/**
 * Get the last pressed or released character from the PC's keyboard
 * @param indev_drv pointer to the related input device driver
 * @param data store the read data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool lv_keyboard_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

void lv_keyboard_handler(xgui_msg_t *msg);

/**********************
 *      MACROS
 **********************/

#endif /*USE_KEYBOARD*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*KEYBOARD_H*/
