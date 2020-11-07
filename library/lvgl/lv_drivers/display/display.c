/**
 * @file display.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "display.h"

#if USE_GAPI_DISPLAY

extern int lv_winhdl;

#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>

/*********************
 *      DEFINES
 *********************/
g_bitmap_t *lv_display_render;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      STRUCTURES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int lv_display_init(uint32_t width, uint32_t height)
{
    lv_display_render = g_new_bitmap(width, height);
    if (lv_display_render == NULL)
        return -1;

    return 0;   
}

void lv_display_exit(void)
{
    g_del_bitmap(lv_display_render);
}

/**
 * Flush a buffer to the marked area
 * @param drv pointer to driver where this function belongs
 * @param area an area where to copy `color_p`
 * @param color_p an array of pixel to copy to the `area` part of the screen
 */
void lv_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    int32_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            g_putpixel(lv_display_render, x, y, *(g_color_t *) color_p);
            color_p++;
        }
    }
    /* 更新区域 */
    #ifdef LV_DISPLAY_PAINT
    g_invalid_window(lv_winhdl);
    g_update_window(lv_winhdl);
    #else
    g_paint_window(lv_winhdl, 0, 0, lv_display_render);
    #endif
    lv_disp_flush_ready(drv);         /* Indicate you are ready with the flushing*/
}

void lv_display_get_sizes(uint32_t *width, uint32_t *height) {
    if (width)
        *width = lv_display_render->width;

    if (height)
        *height = lv_display_render->height;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
