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
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef MONITOR_GAPI_INCLUDE_PATH
#define MONITOR_GAPI_INCLUDE_PATH <gapi.h>
#endif

#include MONITOR_GAPI_INCLUDE_PATH
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
void keyboard_drv_init(void);

/**
 * Get the last pressed or released character from the PC's keyboard
 * @param indev_drv pointer to the related input device driver
 * @param data store the read data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool keyboard_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

void keyboard_handler(int keycode, int down);

/**********************
 *      MACROS
 **********************/

#endif /*USE_KEYBOARD*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*KEYBOARD_H*/
