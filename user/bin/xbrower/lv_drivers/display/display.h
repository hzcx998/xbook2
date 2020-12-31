/**
 * @file display.h
 *
 */

#ifndef _GAPI_DISPLAY_H
#define _GAPI_DISPLAY_H

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

#ifdef USE_GAPI_DISPLAY

#define LV_LVGL_H_INCLUDE_SIMPLE

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include <lvgl.h>
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/
#define LV_DISPLAY_PAINT

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
// extern g_bitmap_t *lv_display_render;

int lv_display_init(uint32_t width, uint32_t height);
void lv_display_exit(void);
void lv_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);
void lv_display_get_sizes(uint32_t *width, uint32_t *height);

/**********************
 *      MACROS
 **********************/

#endif  /*USE_GAPI_DISPLAY*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*_GAPI_DISPLAY_H*/
