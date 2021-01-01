/**
 * @file display.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "display.h"
#include "xbrower_screen.h"
#include "xbrower_render.h"

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
xbrower_view_t *lv_display_view = NULL;

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
    lv_display_view = xbrower_view_create(0, 0, width, height);
    if (!lv_display_view)
        return -1;
    xbrower_view_set_z(lv_display_view, 0);
    return 0;   
}

void lv_display_exit(void)
{
    xbrower_view_set_z(lv_display_view, -1);
    xbrower_view_destroy(lv_display_view);
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
            xbrower_render_putpixel(lv_display_view, x, y, *(xbrower_color_t *) color_p);
            color_p++;
        }
    }
    view_self_refresh(lv_display_view);
    lv_disp_flush_ready(drv);         /* Indicate you are ready with the flushing*/
}

void lv_display_get_sizes(uint32_t *width, uint32_t *height) {
    if (width)
        *width = xbrower_screen.width;

    if (height)
        *height = xbrower_screen.height;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
