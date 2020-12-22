#ifndef _XGUI_HAL_H
#define _XGUI_HAL_H

#include "xgui_screen.h"
#include "xgui_mouse.h"
#include "xgui_keyboard.h"
#include "xgui_section.h"

int xgui_screen_open(xgui_screen_t *screen);
int xgui_screen_close(xgui_screen_t *screen);

int xgui_mouse_open(xgui_mouse_t *mouse);
int xgui_mouse_close(xgui_mouse_t *mouse);
int xgui_mouse_read(xgui_mouse_t *mouse);

int xgui_keyboard_open(xgui_keyboard_t *keyboard);
int xgui_keyboard_close(xgui_keyboard_t *keyboard);

int xgui_section_open(xgui_section_t *section);
int xgui_section_close(xgui_section_t *section);

#endif /* _XGUI_HAL_H */
