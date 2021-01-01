/**
 * @file lvgl_window.h
 * Include all LittleV GL related headers
 */
 
#ifndef LVGL_WINDOW_H
#define LVGL_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

lv_indev_t *lv_window_get_kbd();
lv_indev_t *lv_window_get_mouse();
lv_indev_t *lv_window_get_mousewheel();
void lv_window_set_handler(void (*handler)(void));
int lv_window_init(uint32_t width, uint32_t height);
void lv_window_loop();
void lv_window_quit();
void lv_window_exit(); // exit imediately

#ifdef __cplusplus
}
#endif

#endif /*LVGL_WINDOW_H*/
