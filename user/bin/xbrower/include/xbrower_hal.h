#ifndef _XGUI_HAL_H
#define _XGUI_HAL_H

#include "xbrower_screen.h"
#include "xbrower_mouse.h"
#include "xbrower_keyboard.h"
#include "xbrower_section.h"

int xbrower_screen_open(xbrower_screen_t *screen);
int xbrower_screen_close(xbrower_screen_t *screen);

int xbrower_mouse_open(xbrower_mouse_t *mouse);
int xbrower_mouse_close(xbrower_mouse_t *mouse);
int xbrower_mouse_read(xbrower_mouse_t *mouse);

int xbrower_keyboard_open(xbrower_keyboard_t *keyboard);
int xbrower_keyboard_close(xbrower_keyboard_t *keyboard);

int xbrower_section_open(xbrower_section_t *section);
int xbrower_section_close(xbrower_section_t *section);

#endif /* _XGUI_HAL_H */
