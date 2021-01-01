#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/lpc.h>
#include <sys/proc.h>
#include <xbrower_core.h>
#include <xbrower_view.h>
#include <xbrower_bitmap.h>
#include <xbrower_image.h>
#include <xbrower_screen.h>

#include "lvgl_window.h"
#include "lv_examples.h"

void *lvgl_window_thread()
{
    if (lv_window_init(xbrower_screen.width, xbrower_screen.height) < 0)
        return (void *) -1;
    /* 测试案例 */
    #if LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
    #endif
    #if LV_USE_DEMO_PRINTER
    lv_demo_printer();
    #endif
    #if LV_USE_DEMO_KEYPAD_AND_ENCODER
    lv_demo_keypad_encoder();
    #endif
    lv_window_loop();
    return NULL;
}

int main(int argc, char *argv[])
{
    if (xbrower_init() < 0) {
        return -1;
    }
    pthread_t th;
    if (pthread_create(&th, NULL, lvgl_window_thread, NULL) < 0) {
        xbrower_exit();
        return -1;
    }
    xbrower_loop();
    return 0;
}
